#pragma once
#include <cstdint>
#include <nonstd/expected.hpp>

class VideoThumbnailer
{
public:
    struct VideoFrame
    {
        int width;
        int height;
        int lineSize;
        std::vector<uint8_t> frameData;
    };

    // filePath must exist for the duration of this running and formatCtx must be managed elsewhere!
    VideoThumbnailer(struct AVFormatContext* formatCtx, const struct AVStream* stream)
        : m_formatCtx(formatCtx), m_stream(stream) {}

    ~VideoThumbnailer();

    nonstd::expected<std::vector<uint8_t>, std::string> run(int seekSeconds);
private:
    const struct AVCodec* m_codec{};
    struct AVCodecContext* m_codecCtx{};
    struct AVFormatContext* m_formatCtx{};
    struct AVFrame* m_frame{};
    struct AVPacket* m_packet{};
    const struct AVStream* m_stream{};

    struct FilterGraph
    {
        struct AVFilterGraph* graph{};
        struct AVFilterContext* source{};
        struct AVFilterContext* sink{};
    };

    void checkRc(int ret, const std::string& message);
    std::string codecName();
    FilterGraph createFilterGraph(struct AVRational timeBase, int height);
    void decodeFrame();
    bool decodePacket();
    bool getPacket();
    std::vector<uint8_t> getPNGData(const VideoFrame& frame);
    VideoFrame getScaledFrame(int height);
    int getRotationValue();
    void initializeCodec();
    void seek(int seconds);
};
