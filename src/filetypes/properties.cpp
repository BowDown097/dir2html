#include "properties.h"
#include <iostream>
#include <magic.h>

void logHandlingError(std::string_view path, std::string_view type, std::string_view message = {})
{
    std::cout << "Could not handle " << path << " as a" << type << "file";
    if (!message.empty())
        std::cout << ": " << message;
    std::cout << ". Handling as a normal file." << std::endl;
}

FileProperties propertiesFor(const std::string& file)
{
    // magic_set is not thread-safe, so we need to make thread-local instances
    static thread_local std::unique_ptr<magic_set, decltype(&magic_close)> magic(
        magic_open(MAGIC_MIME_TYPE), &magic_close);
    if (!magic || magic_load(magic.get(), nullptr) != 0)
    {
        // not super smart technically, but whatever
        std::cerr << "Failed to initialize magic" << std::endl;
        exit(EXIT_FAILURE);
    }

    const std::string mimeType = magic_file(magic.get(), file.c_str());
    if (mimeType.rfind("audio/", 0) == 0)
    {
        if (std::optional<AudioProperties> props = openAsAudio(file))
            return *props;
        else
            logHandlingError(file, "n audio");
    }
    else if (mimeType.rfind("image/", 0) == 0)
    {
        if (nonstd::expected<ImageProperties, std::string> props = openAsImage(file))
            return *props;
        else
            logHandlingError(file, "n image", props.error());
    }
    else if (mimeType.rfind("video/", 0) == 0)
    {
        auto [props, message] = openAsVideo(file);
        if (props.has_value() && !props->thumbnailData.empty())
        {
            return *props;
        }
        else if (props.has_value())
        {
            std::cout << "Could not generate a thumbnail for " << file << ": " << message << std::endl;
            return *props;
        }
        else
        {
            logHandlingError(file, "video", message);
        }
    }

    return std::monostate{};
}
