#pragma once
#include "audiofile.h"
#include "imagefile.h"
#include "videofile.h"
#include <variant>

using FileProperties = std::variant<std::monostate, AudioProperties, ImageProperties, VideoProperties>;
FileProperties propertiesFor(const std::string& file);
