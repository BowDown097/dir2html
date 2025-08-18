#pragma once
#include "documentconstants.h"
#include "filetypes/audiofile.h"
#include "filetypes/imagefile.h"
#include "filetypes/videofile.h"
#include "include/base64.hpp"
#include "include/ctml.hpp"
#include <filesystem>
#include <variant>

using FileProperties = std::variant<std::monostate, AudioProperties, ImageProperties, VideoProperties>;

class DirDocument : public CTML::Document
{
public:
    DirDocument()
        : CTML::Document(), fileGrid("div")
    {
        fileGrid.SetAttribute("class", "file-grid");

        AppendNodeToHead(CTML::Node("meta")
            .SetAttribute("name", "viewport")
            .SetAttribute("content", "width=device-width, initial-scale=1.0"));

        AppendNodeToHead(CTML::Node("style", std::string(DocumentConstants::StyleSheet)));
    }

    void addFileEntry(const std::filesystem::path& filePath, const FileProperties& fileProperties)
    {
        CTML::Node fileCard("div");
        fileCard.SetAttribute("class", "file-card");

        CTML::Node fileImg("img");
        fileImg.SetAttribute("class", "file-img");
        fileImg.UseClosingTag(false);

        CTML::Node fileInfo("div");
        fileInfo.SetAttribute("class", "file-info");
        fileInfo.AppendChild(CTML::Node("div")
            .SetAttribute("class", "file-name")
            .AppendText(filePath.filename().string()));

        CTML::Node metadataPanel("div");
        metadataPanel.SetAttribute("class", "metadata-panel");
        metadataPanel.SetAttribute("style", "height: 0px");
        appendMetadataItem(metadataPanel, "Size:", sizeString(filePath));

        if (const AudioProperties* ap = std::get_if<AudioProperties>(&fileProperties))
        {
            fileImg.SetAttribute("src", std::string(DocumentConstants::AudioIcon));
            appendMetadataItem(metadataPanel, "Title:", ap->title);
            appendMetadataItem(metadataPanel, "Duration:", formatDuration(ap->duration));
            appendMetadataItem(metadataPanel, "Album:", ap->album);
            appendMetadataItem(metadataPanel, "Artist:", ap->artist);
            appendMetadataItem(metadataPanel, "Comment:", ap->comment);
            appendMetadataItem(metadataPanel, "Genre:", ap->genre);
            if (ap->track)
                appendMetadataItem(metadataPanel, "Track #:", std::to_string(ap->track));
            if (ap->year)
                appendMetadataItem(metadataPanel, "Year:", std::to_string(ap->year));
        }
        else if (const ImageProperties* ip = std::get_if<ImageProperties>(&fileProperties))
        {
            if (!ip->thumbnailData.empty())
            {
                fileImg.SetAttribute("src",
                    "data:image/png;base64," +
                    code::base64_encode(ip->thumbnailData.data(), ip->thumbnailData.size()));
            }
            else
            {
                fileImg.SetAttribute("src", std::string(DocumentConstants::ImageIcon));
            }

            appendMetadataItem(metadataPanel, "Dimensions:",
                std::to_string(ip->width) + "×" + std::to_string(ip->height));
        }
        else if (const VideoProperties* vp = std::get_if<VideoProperties>(&fileProperties))
        {
            if (!vp->thumbnailData.empty())
            {
                fileImg.SetAttribute("src",
                    "data:image/png;base64," +
                    code::base64_encode(vp->thumbnailData.data(), vp->thumbnailData.size()));
            }
            else
            {
                fileImg.SetAttribute("src", std::string(DocumentConstants::VideoIcon));
            }

            std::string fps(16, '\0');
            fps.resize(snprintf(fps.data(), fps.size(), "%.2f", vp->metadata.fps));

            appendMetadataItem(metadataPanel, "Duration:", formatDuration(vp->metadata.duration));
            appendMetadataItem(metadataPanel, "Dimensions:",
                std::to_string(vp->metadata.width) + "×" + std::to_string(vp->metadata.height));
            appendMetadataItem(metadataPanel, "Frame Rate:", fps + " fps");
        }
        else
        {
            fileImg.SetAttribute("src", std::string(DocumentConstants::GenericIcon));
        }

        fileInfo.AppendChild(CTML::Node("span", "▼").SetAttribute("class", "metadata-toggle"));

        fileInfo.AppendChild(metadataPanel);
        fileCard.AppendChild(fileImg);
        fileCard.AppendChild(fileInfo);
        fileGrid.AppendChild(fileCard);
    }

    void addNavigationHeader(const std::vector<std::pair<std::string, std::string>>& links)
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

    void finalize()
    {
        AppendNodeToBody(fileGrid);
        AppendNodeToBody(CTML::Node("script", std::string(DocumentConstants::ScriptBody)));
    }
private:
    CTML::Node fileGrid;

    static void appendMetadataItem(CTML::Node& parent, const std::string& key, const std::string& value)
    {
        if (!value.empty())
        {
            parent.AppendChild(
                CTML::Node("span")
                    .AppendChild(CTML::Node("strong", key))
                    .AppendText(' ' + value));
            parent.AppendChild(CTML::Node("br").UseClosingTag(false));
        }
    }

    static std::string formatDuration(int duration)
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

    static std::string sizeString(const std::filesystem::path& path)
    {
        double fileSize = std::filesystem::file_size(path);
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
};
