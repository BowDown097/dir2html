#pragma once
#include "filetypes/properties.h"
#include <filesystem>
#include <lexbor-cpp/document.h>

namespace stdfs = std::filesystem;

class DirDocument : public lexbor::document
{
public:
    DirDocument();
    explicit DirDocument(const stdfs::path& path);

    void addFileEntry(lexbor::node node);
    void addNavigationHeader(const std::vector<std::pair<std::string, std::string>>& links);
    bool mergeFileEntry(lexbor::element element, const std::string& filename);

    lexbor::element createFileElement(const stdfs::path& path);
    lexbor::element createFileElement(
        const stdfs::path& path, const FileProperties& props,
        const std::optional<stdfs::path>& thumbsPath);

    void finalize();
private:
    std::optional<lexbor::element> m_fileGrid;

    void appendMetadataItem(lexbor::element& parent, const std::string& key, const std::string& value);
    static std::string formatDuration(int duration);
    static void setThumbnail(
        lexbor::element& element, const std::vector<uint8_t>& data,
        const std::string& filename, std::string_view fallbackIcon,
        const std::optional<stdfs::path>& thumbsPath);
    static std::string sizeString(const stdfs::path& path);
};
