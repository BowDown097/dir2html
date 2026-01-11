#pragma once
#include "filetypes/properties.h"
#include <ctml.hpp>
#include <filesystem>

namespace stdfs = std::filesystem;

class DirDocument : public CTML::Document
{
public:
    DirDocument();
    void addFileEntry(CTML::Node&& node);
    void addNavigationHeader(const std::vector<std::pair<std::string, std::string>>& links);
    static CTML::Node createFileNode(const stdfs::path& path);
    static CTML::Node createFileNode(
        const stdfs::path& path, const FileProperties& props,
        const std::optional<stdfs::path>& thumbsPath);
    void finalize();
private:
    CTML::Node fileGrid;

    static void appendMetadataItem(CTML::Node& parent, const std::string& key, const std::string& value);
    static std::string formatDuration(int duration);
    static void setThumbnail(
        CTML::Node& node, const std::vector<uint8_t>& data,
        const std::string& filename, const std::string& fallbackIcon,
        const std::optional<stdfs::path>& thumbsPath);
    static std::string sizeString(const stdfs::path& path);
};
