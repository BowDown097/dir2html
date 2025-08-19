#include "imagefile.h"

extern "C" {
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>

// implementation provided in src/stb/stb_image_write.cpp
// #define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
}

nonstd::expected<ImageProperties, std::string> openAsImage(std::string_view path)
{
    ImageProperties props;
    if (uint8_t* data = stbi_load(path.data(), &props.width, &props.height, &props.channels, 0))
    {
        int scaledHeight = 64, scaledWidth = (props.width / (float)props.height) * scaledHeight;

        uint8_t* rawThumbnailData = new uint8_t[scaledWidth * scaledHeight * props.channels];
        stbir_resize_uint8_srgb(
            data,
            props.width, props.height, 0,
            rawThumbnailData,
            scaledWidth, scaledHeight, 0,
            (stbir_pixel_layout)props.channels);

        stbi_write_png_to_func(
            [](void* context, void* data, int size) {
                auto* buffer = reinterpret_cast<std::vector<uint8_t>*>(context);
                auto* u8data = reinterpret_cast<uint8_t*>(data);
                buffer->insert(buffer->end(), u8data, u8data + size);
            },
            &props.thumbnailData,
            scaledWidth, scaledHeight, props.channels,
            rawThumbnailData, 0);

        stbi_image_free(data);
        delete[] rawThumbnailData;

        return props;
    }
    else
    {
        return nonstd::unexpected<std::string>(stbi_failure_reason());
    }
}
