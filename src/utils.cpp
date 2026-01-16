#include "utils.h"
#include <algorithm>
#include <fstream>

std::string getFileContent(const std::filesystem::path& path)
{
    std::ifstream in(path);
    size_t size = std::filesystem::file_size(path);
    std::string content(size, '\0');
    in.read(&content[0], size);
    return content;
}

std::string_view ltrim(std::string_view s)
{
    auto it = std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isspace(c); });
    s.remove_prefix(it - s.begin());
    return s;
}

std::string_view rtrim(std::string_view s)
{
    auto it = std::find_if(s.rbegin(), s.rend(), [](unsigned char c) { return !std::isspace(c); });
    s.remove_suffix(it - s.rbegin());
    return s;
}
