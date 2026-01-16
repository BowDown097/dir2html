#pragma once
#include <filesystem>

std::string getFileContent(const std::filesystem::path& path);
std::string_view ltrim(std::string_view s);
std::string_view rtrim(std::string_view s);
