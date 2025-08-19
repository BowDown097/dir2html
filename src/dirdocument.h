#pragma once
#include "filetypes/audiofile.h"
#include "filetypes/imagefile.h"
#include "filetypes/videofile.h"
#include <ctml.hpp>
#include <filesystem>
#include <variant>

using FileProperties = std::variant<std::monostate, AudioProperties, ImageProperties, VideoProperties>;

class DirDocument : public CTML::Document
{
public:
    DirDocument();
    void addFileEntry(const std::filesystem::path& filePath, const FileProperties& fileProperties);
    void addNavigationHeader(const std::vector<std::pair<std::string, std::string>>& links);
    void finalize();
private:
    CTML::Node fileGrid;

    static void appendMetadataItem(CTML::Node& parent, const std::string& key, const std::string& value);
    static std::string formatDuration(int duration);
    static std::string sizeString(const std::filesystem::path& path);
};
