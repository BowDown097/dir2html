#pragma once
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

struct VideoMetadata
{
    int duration;
    double fps;
    int width;
    int height;
};

struct VideoProperties
{
    VideoMetadata metadata;
    std::vector<uint8_t> thumbnailData;
};

// not using expected due to thumbnail generation failure not being an error
std::pair<std::optional<VideoProperties>, std::string> openAsVideo(std::string_view path);
