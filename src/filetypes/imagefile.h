#pragma once
#include "include/expected.hpp"
#include <cstdint>
#include <string>
#include <vector>

struct ImageProperties
{
    int width;
    int height;
    int channels;
    std::vector<uint8_t> thumbnailData;
};

nonstd::expected<ImageProperties, std::string> openAsImage(std::string_view path);
