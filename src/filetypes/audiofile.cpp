#include "audiofile.h"
#include <taglib/fileref.h>

std::optional<AudioProperties> openAsAudio(std::string_view path)
{
    TagLib::FileRef ref(path.data());
    if (ref.isNull())
        return std::nullopt;

    return AudioProperties {
        .album = ref.tag()->album().to8Bit(true),
        .artist = ref.tag()->artist().to8Bit(true),
        .comment = ref.tag()->comment().to8Bit(true),
        .duration = ref.audioProperties()->lengthInSeconds(),
        .genre = ref.tag()->genre().to8Bit(true),
        .title = ref.tag()->title().to8Bit(true),
        .track = ref.tag()->track(),
        .year = ref.tag()->year()
    };
}
