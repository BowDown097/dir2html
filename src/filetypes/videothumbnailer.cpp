#include "videothumbnailer.h"
#include <memory>
#include <sstream>
#include <stdexcept>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavutil/display.h>

// implementation of both of these provided elsewhere
#include <stb_image.h>
#include <stb_image_write.h>
}

VideoThumbnailer::~VideoThumbnailer()
{
    if (m_codecCtx)
        avcodec_free_context(&m_codecCtx);
    if (m_packet)
        av_packet_free(&m_packet);
    if (m_frame)
        av_frame_free(&m_frame);
}

nonstd::expected<std::vector<uint8_t>, std::string> VideoThumbnailer::run(int seekSeconds)
{
    try
    {
        initializeCodec();
        m_frame = av_frame_alloc();
        decodeFrame();
        seek(seekSeconds);
        return getPNGData(getScaledFrame(64));
    }
    catch (const std::runtime_error& e)
    {
        return nonstd::unexpected<std::string>(e.what());
    }
}

void VideoThumbnailer::checkRc(int ret, const std::string& message)
{
    if (ret < 0)
    {
        char buf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, buf, sizeof(buf));
        throw std::runtime_error(message + ": " + buf);
    }
}

std::string VideoThumbnailer::codecName()
{
    return avcodec_get_name(m_stream->codecpar->codec_id);
}

VideoThumbnailer::FilterGraph VideoThumbnailer::createFilterGraph(AVRational timeBase, int height)
{
    FilterGraph result;

    result.graph = avfilter_graph_alloc();
    if (!result.graph)
        throw std::runtime_error("Could not create filter graph");

    std::stringstream ss;
    ss << "video_size=" << m_codecCtx->width << 'x' << m_codecCtx->height
       << ":pix_fmt=" << m_codecCtx->pix_fmt
       << ":time_base=" << timeBase.num << '/' << timeBase.den
       << ":pixel_aspect=" << m_codecCtx->sample_aspect_ratio.num << '/' << std::max(m_codecCtx->sample_aspect_ratio.den, 1);

    checkRc(avfilter_graph_create_filter(&result.source, avfilter_get_by_name("buffer"), "thumb_buffer", ss.str().c_str(), nullptr, result.graph),
            "Could not create filter source");
    checkRc(avfilter_graph_create_filter(&result.sink, avfilter_get_by_name("buffersink"), "thumb_buffersink", nullptr, nullptr, result.graph),
            "Could not create filter sink");

    AVFilterContext* yadifFilter{};
    if (m_frame->flags & AV_FRAME_FLAG_INTERLACED)
    {
        checkRc(avfilter_graph_create_filter(&yadifFilter, avfilter_get_by_name("yadif"), "thumb_deint", "deint=1", nullptr, result.graph),
                "Could not create deinterlace filter");
    }

    AVFilterContext* scaleFilter{};
    std::string scaleString = "w=-1:h=" + std::to_string(height);
    checkRc(avfilter_graph_create_filter(&scaleFilter, avfilter_get_by_name("scale"), "thumb_scale", scaleString.c_str(), nullptr, result.graph),
            "Could not create scale filter");

    AVFilterContext* formatFilter{};
    checkRc(avfilter_graph_create_filter(&formatFilter, avfilter_get_by_name("format"), "thumb_format", "pix_fmts=rgb24", nullptr, result.graph),
            "Could not create format filter");

    AVFilterContext* rotateFilter{};
    if (int rotation = getRotationValue(); rotation == 3)
    {
        checkRc(avfilter_graph_create_filter(&rotateFilter, avfilter_get_by_name("rotate"), "thumb_rotate", "PI", nullptr, result.graph),
                "Could not create rotate filter");
    }
    else if (rotation != -1)
    {
        checkRc(avfilter_graph_create_filter(&rotateFilter, avfilter_get_by_name("transpose"), "thumb_transpose", std::to_string(rotation).c_str(), nullptr, result.graph),
                "Could not create rotate filter");
    }

    checkRc(avfilter_link(rotateFilter ? rotateFilter : formatFilter, 0, result.sink, 0), "Could not link final filter");

    if (rotateFilter)
        checkRc(avfilter_link(formatFilter, 0, rotateFilter, 0), "Could not link format filter");

    checkRc(avfilter_link(scaleFilter, 0, formatFilter, 0), "Could not link scale filter");

    if (yadifFilter)
        checkRc(avfilter_link(yadifFilter, 0, scaleFilter, 0), "Could not link deinterlace filter");

    checkRc(avfilter_link(result.source, 0, yadifFilter ? yadifFilter : scaleFilter, 0), "Could not link source filter");
    checkRc(avfilter_graph_config(result.graph, nullptr), "Could not configure filter graph");

    return result;
}

void VideoThumbnailer::decodeFrame()
{
    bool frameFinished = false;
    while (!frameFinished && getPacket())
        frameFinished = decodePacket();

    if (!frameFinished)
        throw std::runtime_error("Could not decode frame: frame not finished");
}

bool VideoThumbnailer::decodePacket()
{
    if (m_packet->stream_index != m_stream->index)
        return false;

    int rc = avcodec_send_packet(m_codecCtx, m_packet);
    if (rc == AVERROR(EAGAIN))
        rc = 0;

    if (rc == AVERROR_EOF)
        return false;
    else if (rc < 0)
        throw std::runtime_error("Could not decode frame: avcodec_send_packet() < 0");

    rc = avcodec_receive_frame(m_codecCtx, m_frame);
    switch (rc)
    {
    case 0:
        return true;
    case AVERROR(EAGAIN):
        return false;
    default:
        throw std::runtime_error("Could not decode frame: avcodec_receive_frame() < 0");
    }
}

bool VideoThumbnailer::getPacket()
{
    if (!m_packet)
        m_packet = av_packet_alloc();
    else
        av_packet_unref(m_packet);

    bool framesAvailable = true, frameDecoded = false;
    while (framesAvailable && !frameDecoded)
    {
        framesAvailable = av_read_frame(m_formatCtx, m_packet) >= 0;
        if (framesAvailable)
        {
            frameDecoded = m_packet->stream_index == m_stream->index;
            if (!frameDecoded)
                av_packet_unref(m_packet);
        }
    }

    return frameDecoded;
}

std::vector<uint8_t> VideoThumbnailer::getPNGData(const VideoFrame& frame)
{
    std::vector<uint8_t> result;

    int res = stbi_write_png_to_func(
        [](void* context, void* data, int size) {
            auto* buffer = reinterpret_cast<std::vector<uint8_t>*>(context);
            auto* u8data = reinterpret_cast<uint8_t*>(data);
            buffer->insert(buffer->end(), u8data, u8data + size);
        },
        &result,
        frame.width, frame.height, 3,
        frame.frameData.data(), frame.lineSize);

    if (res == 0)
        throw std::runtime_error(std::string("Could not write PNG: ") + stbi_failure_reason());

    return result;
}

int VideoThumbnailer::getRotationValue()
{
    const uint8_t* matrix{};
    for (int i = 0; i < m_stream->codecpar->nb_coded_side_data; ++i)
    {
        if (const AVPacketSideData& sd = m_stream->codecpar->coded_side_data[i]; sd.type == AV_PKT_DATA_DISPLAYMATRIX)
        {
            matrix = sd.data;
            break;
        }
    }

    if (matrix)
    {
        long angle = lround(av_display_rotation_get(reinterpret_cast<const int32_t*>(matrix)));
        if (angle < -135)
            return 3;
        else if (angle > 45 && angle < 135)
            return 2;
        else if (angle < -45 && angle > -135)
            return 1;
    }

    return -1;
}

VideoThumbnailer::VideoFrame VideoThumbnailer::getScaledFrame(int height)
{
    FilterGraph filterGraph = createFilterGraph(m_stream->time_base, height);

    auto del = [](AVFrame* f) { av_frame_free(&f); };
    std::unique_ptr<AVFrame, decltype(del)> res(av_frame_alloc(), del);

    checkRc(av_buffersrc_write_frame(filterGraph.source, m_frame), "Could not write frame to filter graph");

    int attempts = 0;
    int rc = av_buffersink_get_frame(filterGraph.sink, res.get());
    while (rc == AVERROR(EAGAIN) && attempts++ < 10)
    {
        decodeFrame();
        checkRc(av_buffersrc_write_frame(filterGraph.source, m_frame), "Could not write frame to filter graph");
        rc = av_buffersink_get_frame(filterGraph.sink, res.get());
    }

    checkRc(rc, "Could not get buffer from filter");

    VideoThumbnailer::VideoFrame result = {
        .width = res->width,
        .height = res->height,
        .lineSize = res->linesize[0]
    };

    result.frameData.resize(result.lineSize * result.height);
    memcpy(result.frameData.data(), res->data[0], result.frameData.size());

    if (filterGraph.graph)
        avfilter_graph_free(&filterGraph.graph);

    return result;
}

void VideoThumbnailer::initializeCodec()
{
    m_codec = avcodec_find_decoder(m_stream->codecpar->codec_id);
    if (!m_codec)
        throw std::runtime_error("Could not find decoder for codec " + codecName());

    m_codecCtx = avcodec_alloc_context3(m_codec);
    if (!m_codecCtx)
        throw std::runtime_error("Could not allocate context for codec " + codecName());

    if (avcodec_parameters_to_context(m_codecCtx, m_stream->codecpar) < 0)
        throw std::runtime_error("Could not configure context for codec " + codecName());

    m_codecCtx->workaround_bugs = true;
    if (avcodec_open2(m_codecCtx, m_codec, nullptr) < 0)
        throw std::runtime_error("Could not open codec " + codecName());
}

void VideoThumbnailer::seek(int seconds)
{
    int64_t timestamp = AV_TIME_BASE * seconds;
    if (timestamp < 0)
        timestamp = 0;

    checkRc(av_seek_frame(m_formatCtx, -1, timestamp, 0), "Could not seek video");

    avcodec_flush_buffers(m_codecCtx);

    int keyFrameAttempts = 0;
    bool gotFrame;

    do
    {
        int count = 0;
        gotFrame = false;

        while (!gotFrame && count < 20)
        {
            getPacket();
            try { gotFrame = decodePacket(); } catch (const std::runtime_error&) {}
            ++count;
        }

        ++keyFrameAttempts;
    } while ((!gotFrame || !(m_frame->flags & AV_FRAME_FLAG_KEY)) && keyFrameAttempts < 200);

    if (!gotFrame)
        throw std::runtime_error("Could not seek video");
}
