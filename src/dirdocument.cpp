#include "dirdocument.h"
#include "documentconstants.h"
#include <base64.hpp>
#include <fstream>

DirDocument::DirDocument()
    : CTML::Document(), fileGrid("div")
{
    fileGrid.SetAttribute("class", "file-grid");

    AppendNodeToHead(CTML::Node("meta")
        .SetAttribute("name", "viewport")
        .SetAttribute("content", "width=device-width, initial-scale=1.0"));

    AppendNodeToHead(CTML::Node("style", DocumentConstants::StyleSheet));
}

void DirDocument::addFileEntry(CTML::Node&& node)
{
    fileGrid.AppendChild(std::move(node));
}

void DirDocument::addNavigationHeader(const std::vector<std::pair<std::string, std::string>>& links)
{
    CTML::Node nav("div");
    nav.SetAttribute("class", "folder-nav");

    for (const auto& [href, text] : links)
    {
        nav.AppendChild(CTML::Node("a").SetAttribute("href", href).AppendText(text));
        nav.AppendChild(CTML::Node("span", " | "));
    }

    // remove trailing separator
    if (!nav.GetChildren().empty())
        nav.RemoveChild(nav.GetChildren().size() - 1);

    AppendNodeToBody(nav);
}

void DirDocument::appendMetadataItem(CTML::Node& parent, const std::string& key, const std::string& value)
{
    if (!value.empty() && std::any_of(value.begin(), value.end(), std::not_fn(isspace)))
    {
        parent.AppendChild(
            CTML::Node("span")
                .AppendChild(CTML::Node("strong", key))
                .AppendText(' ' + value));
        parent.AppendChild(CTML::Node("br").UseClosingTag(false));
    }
}

CTML::Node DirDocument::createFileNode(const stdfs::path& path)
{
    return createFileNode(path, propertiesFor(path), std::nullopt);
}

CTML::Node DirDocument::createFileNode(
    const stdfs::path& path, const FileProperties& props,
    const std::optional<stdfs::path>& thumbsPath)
{
    const std::string filename = path.filename().string();

    CTML::Node fileCard("div");
    fileCard.SetAttribute("class", "file-card");

    CTML::Node fileImg("img");
    fileImg.SetAttribute("class", "file-img");
    fileImg.UseClosingTag(false);

    CTML::Node fileInfo("div");
    fileInfo.SetAttribute("class", "file-info");
    fileInfo.AppendChild(CTML::Node("div")
        .SetAttribute("class", "file-name")
        .AppendText(filename));

    CTML::Node metadataPanel("div");
    metadataPanel.SetAttribute("class", "metadata-panel");
    metadataPanel.SetAttribute("style", "height: 0px");
    appendMetadataItem(metadataPanel, "Size:", sizeString(path));

    if (const AudioProperties* audio = std::get_if<AudioProperties>(&props))
    {
        fileImg.SetAttribute("src", DocumentConstants::AudioIcon);
        appendMetadataItem(metadataPanel, "Title:", audio->title);
        appendMetadataItem(metadataPanel, "Duration:", formatDuration(audio->duration));
        appendMetadataItem(metadataPanel, "Album:", audio->album);
        appendMetadataItem(metadataPanel, "Artist:", audio->artist);
        appendMetadataItem(metadataPanel, "Comment:", audio->comment);
        appendMetadataItem(metadataPanel, "Genre:", audio->genre);
        if (audio->track)
            appendMetadataItem(metadataPanel, "Track #:", std::to_string(audio->track));
        if (audio->year)
            appendMetadataItem(metadataPanel, "Year:", std::to_string(audio->year));
    }
    else if (const ImageProperties* image = std::get_if<ImageProperties>(&props))
    {
        setThumbnail(fileImg, image->thumbnailData, filename, DocumentConstants::ImageIcon, thumbsPath);
        appendMetadataItem(metadataPanel, "Dimensions:",
            std::to_string(image->width) + "×" + std::to_string(image->height));
    }
    else if (const VideoProperties* video = std::get_if<VideoProperties>(&props))
    {
        setThumbnail(fileImg, video->thumbnailData, filename, DocumentConstants::VideoIcon, thumbsPath);

        std::string fps(16, '\0');
        fps.resize(snprintf(fps.data(), fps.size(), "%.2f", video->metadata.fps));

        appendMetadataItem(metadataPanel, "Duration:", formatDuration(video->metadata.duration));
        appendMetadataItem(metadataPanel, "Dimensions:",
            std::to_string(video->metadata.width) + "×" + std::to_string(video->metadata.height));
        appendMetadataItem(metadataPanel, "Frame Rate:", fps + " fps");
    }
    else
    {
        fileImg.SetAttribute("src", DocumentConstants::GenericIcon);
    }

    fileInfo.AppendChild(CTML::Node("span", "▼").SetAttribute("class", "metadata-toggle"));
    fileInfo.AppendChild(metadataPanel);
    fileCard.AppendChild(fileImg);
    fileCard.AppendChild(fileInfo);

    return fileCard;
}

void DirDocument::finalize()
{
    AppendNodeToBody(fileGrid);
    AppendNodeToBody(CTML::Node("script", DocumentConstants::ScriptBody));
}

std::string DirDocument::formatDuration(int duration)
{
    int hours = duration / 3600, minutes = (duration % 3600) / 60, seconds = duration % 60;

    std::ostringstream oss;
    if (hours > 0)
    {
        oss << hours << ':'
            << std::setw(2) << std::setfill('0') << minutes << ':'
            << std::setw(2) << std::setfill('0') << seconds;
    }
    else
    {
        oss << minutes << ':'
            << std::setw(2) << std::setfill('0') << seconds;
    }

    return oss.str();
}

void DirDocument::setThumbnail(
    CTML::Node& node, const std::vector<uint8_t>& data,
    const std::string& filename, const std::string& fallbackIcon,
    const std::optional<stdfs::path>& thumbsPath)
{
    if (data.empty())
    {
        node.SetAttribute("src", fallbackIcon);
    }
    else if (thumbsPath.has_value())
    {
        std::ofstream thumbStream(thumbsPath.value() / (filename + ".png"), std::ios::binary);
        thumbStream.write(reinterpret_cast<const char*>(data.data()), data.size());
        node.SetAttribute("src", "thumbs/" + filename + ".png");
    }
    else
    {
        node.SetAttribute("src", "data:image/png;base64," + code::base64_encode(data.data(), data.size()));
    }
}

std::string DirDocument::sizeString(const stdfs::path& path)
{
    double fileSize = stdfs::file_size(path);
    std::ostringstream stream = std::ostringstream() << std::fixed << std::setprecision(1);

    if (fileSize < 1024)
    {
        stream << fileSize << " B";
    }
    else if (fileSize < 1048576)
    {
        fileSize /= 1024;
        stream << fileSize << " KiB";
    }
    else if (fileSize < 1073741824)
    {
        fileSize /= 1048576;
        stream << fileSize << " MiB";
    }
    else
    {
        fileSize /= 1073741824;
        stream << fileSize << " GiB";
    }

    return stream.str();
}
