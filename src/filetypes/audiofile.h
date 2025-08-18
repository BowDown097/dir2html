#pragma once
#include <optional>
#include <string>

struct AudioProperties
{
    std::string album;
    std::string artist;
    std::string comment;
    int duration;
    std::string genre;
    std::string title;
    unsigned int track;
    unsigned int year;
};

std::optional<AudioProperties> openAsAudio(std::string_view path);
