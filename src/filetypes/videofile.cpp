#include "videofile.h"
#include "videothumbnailer.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/rational.h>
}

using MetadataResult = nonstd::expected<std::pair<const AVStream*, VideoMetadata>, std::string>;
using ThumbnailResult = nonstd::expected<std::vector<uint8_t>, std::string>;

ThumbnailResult generateThumbnail(AVFormatContext* formatCtx, const AVStream* stream, double videoDuration)
{
    VideoThumbnailer thumbnailer(formatCtx, stream);
    return thumbnailer.run(videoDuration * 0.1);
}

MetadataResult probeMetadata(AVFormatContext* formatCtx)
{
    if (avformat_find_stream_info(formatCtx, nullptr) < 0)
        return nonstd::unexpected<std::string>("Could not get stream info");

    const AVStream* stream{};
    for (unsigned int i = 0; i < formatCtx->nb_streams; ++i)
    {
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            stream = formatCtx->streams[i];
            break;
        }
    }

    if (!stream)
        return nonstd::unexpected<std::string>("Could not find video stream");

    AVRational avgFrameRate = stream->avg_frame_rate;
    return std::make_pair(stream, VideoMetadata {
        .duration = int(formatCtx->duration / AV_TIME_BASE),
        .fps = (avgFrameRate.den != 0) ? (double)avgFrameRate.num / avgFrameRate.den : 0.0,
        .width = stream->codecpar->width,
        .height = stream->codecpar->height
    });
}

std::pair<std::optional<VideoProperties>, std::string> openAsVideo(std::string_view path)
{
    AVFormatContext* formatCtxRaw{};
    if (avformat_open_input(&formatCtxRaw, path.data(), nullptr, nullptr) < 0)
        return std::make_pair(std::nullopt, std::string("Could not open file"));

    auto del = [](AVFormatContext* p) { avformat_close_input(&p); };
    std::unique_ptr<AVFormatContext, decltype(del)> formatCtx(formatCtxRaw, del);

    if (MetadataResult meta = probeMetadata(formatCtx.get()))
    {
        if (ThumbnailResult thumb = generateThumbnail(formatCtx.get(), meta->first, meta->second.duration))
            return std::make_pair(VideoProperties { std::move(meta->second), std::move(*thumb) }, std::string());
        else
            return std::make_pair(VideoProperties{std::move(meta->second)}, thumb.error());
    }
    else
    {
        return std::make_pair(std::nullopt, meta.error());
    }
}
