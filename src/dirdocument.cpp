#include "dirdocument.h"
#include "alphanum.h"
#include "documentconstants.h"
#include <base64.hpp>
#include <fstream>
#include <lexbor-cpp/collection.h>

std::string getFileContent(const stdfs::path& path)
{
    std::ifstream in(path);
    size_t size = stdfs::file_size(path);
    std::string content(size, '\0');
    in.read(&content[0], size);
    return content;
}

std::optional<lexbor::element> getFileGrid(lexbor::document& doc)
{
    if (lxb_dom_node_t* node = doc.query_selector(".file-grid"))
        return lexbor::element(lxb_dom_interface_element(node));
    else
        return std::nullopt;
}

DirDocument::DirDocument()
    : lexbor::document("<!DOCTYPE html><html><head></head><body></body></html>"),
      m_fileGrid(create_element("div"))
{
    m_fileGrid->set_attribute("class", "file-grid");

    lexbor::element meta = create_element("meta");
    meta.set_attribute("name", "viewport");
    meta.set_attribute("content", "width=device-width, initial-scale=1.0");

    lexbor::element style = create_element("style");
    style.inner_html_set(DocumentConstants::StyleSheet);

    head_element().insert_child(meta);
    head_element().insert_child(style);
}

DirDocument::DirDocument(const stdfs::path& path)
    : lexbor::document(lexbor::string_view(getFileContent(path))),
      m_fileGrid(getFileGrid(*this))
{
    if (!m_fileGrid)
        throw std::runtime_error("File grid not found");
}

void DirDocument::addFileEntry(lexbor::node node)
{
    m_fileGrid->insert_child(node);
}

void DirDocument::addNavigationHeader(const std::vector<std::pair<std::string, std::string>>& links)
{
    lexbor::element nav = create_element("div");
    nav.set_attribute("class", "folder-nav");

    for (const auto& [href, text] : links)
    {
        lexbor::element anchor = create_element("a");
        anchor.set_attribute("href", lexbor::string_view(href));
        anchor.inner_html_set(lexbor::string_view(text));

        lexbor::element span = create_element("span");
        span.inner_html_set(" | ");

        nav.insert_child(anchor);
        nav.insert_child(span);
    }

    // remove trailing separator
    lexbor::collection children(*this);
    nav.children(children);
    if (children.length() > 0)
        nav.remove_child(children.at(children.length() - 1));

    body_element().insert_child(nav);
}

void DirDocument::appendMetadataItem(lexbor::element& parent, const std::string& key, const std::string& value)
{
    if (!value.empty() && std::any_of(value.begin(), value.end(), std::not_fn(isspace)))
    {
        lexbor::element strong = create_element("strong");
        strong.inner_html_set(lexbor::string_view(key));

        lexbor::element span = create_element("span");
        span.insert_child(strong);
        span.insert_child(create_text_node(lexbor::string_view(' ' + value)));

        parent.insert_child(span);
        parent.insert_child(create_element("br"));
    }
}

lexbor::element DirDocument::createFileElement(const stdfs::path& path)
{
    return createFileElement(path, propertiesFor(path), std::nullopt);
}

lexbor::element DirDocument::createFileElement(
    const stdfs::path& path, const FileProperties& props,
    const std::optional<stdfs::path>& thumbsPath)
{
    const std::string filenameStr = path.filename().string();

    lexbor::element fileCard = create_element("div");
    fileCard.set_attribute("class", "file-card");

    lexbor::element fileImg = create_element("img");
    fileImg.set_attribute("class", "file-img");

    lexbor::element fileName = create_element("div");
    fileName.set_attribute("class", "file-name");
    fileName.inner_html_set(lexbor::string_view(filenameStr));

    lexbor::element fileInfo = create_element("div");
    fileInfo.set_attribute("class", "file-info");
    fileInfo.insert_child(fileName);

    lexbor::element metadataPanel = create_element("div");
    metadataPanel.set_attribute("class", "metadata-panel");
    metadataPanel.set_attribute("style", "height: 0px");
    appendMetadataItem(metadataPanel, "Size:", sizeString(path));

    if (const AudioProperties* audio = std::get_if<AudioProperties>(&props))
    {
        fileImg.set_attribute("src", DocumentConstants::AudioIcon);
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
        setThumbnail(fileImg, image->thumbnailData, filenameStr, DocumentConstants::ImageIcon, thumbsPath);
        appendMetadataItem(metadataPanel, "Dimensions:",
            std::to_string(image->width) + "×" + std::to_string(image->height));
    }
    else if (const VideoProperties* video = std::get_if<VideoProperties>(&props))
    {
        setThumbnail(fileImg, video->thumbnailData, filenameStr, DocumentConstants::VideoIcon, thumbsPath);

        std::string fps(16, '\0');
        fps.resize(snprintf(fps.data(), fps.size(), "%.2f", video->metadata.fps));

        appendMetadataItem(metadataPanel, "Duration:", formatDuration(video->metadata.duration));
        appendMetadataItem(metadataPanel, "Dimensions:",
            std::to_string(video->metadata.width) + "×" + std::to_string(video->metadata.height));
        appendMetadataItem(metadataPanel, "Frame Rate:", fps + " fps");
    }
    else
    {
        fileImg.set_attribute("src", DocumentConstants::GenericIcon);
    }

    lexbor::element metadataToggle = create_element("span");
    metadataToggle.inner_html_set("▼");
    metadataToggle.set_attribute("class", "metadata-toggle");

    fileInfo.insert_child(metadataToggle);
    fileInfo.insert_child(metadataPanel);
    fileCard.insert_child(fileImg);
    fileCard.insert_child(fileInfo);

    return fileCard;
}

void DirDocument::finalize()
{
    lexbor::element script = create_element("script");
    script.inner_html_set(DocumentConstants::ScriptBody);

    body_element().insert_child(*m_fileGrid);
    body_element().insert_child(script);
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

bool DirDocument::mergeFileEntry(lexbor::element element, const std::string& filename)
{
    lexbor::collection cards(*this);
    m_fileGrid->elements_by_class_name(cards, "file-card");

    if (cards.length() == 0)
    {
        m_fileGrid->insert_child(element);
        return true;
    }

    auto it = std::find_if(cards.begin(), cards.end(), [&filename](const lexbor::element& card) {
        if (lxb_dom_node_t* node = card.query_selector(".file-name"))
        {
            size_t len = 0;
            if (lxb_char_t* text = lxb_dom_node_text_content(node, &len))
            {
                std::string_view sv(reinterpret_cast<const char*>(text), len);
                return doj::alphanum_comp(sv, filename, doj::CASE_INSENSITIVE) >= 0;
            }
        }

        return false;
    });

    if (it != cards.end())
    {
        lxb_dom_exception_code_t code = lxb_dom_node_insert_before_spec(
            lxb_dom_interface_node(m_fileGrid->get_ptr()),
            lxb_dom_interface_node(element.get_ptr()),
            lxb_dom_interface_node((*it).get_ptr()));
        return code == LXB_DOM_EXCEPTION_OK;
    }

    return false;
}

void DirDocument::setThumbnail(
    lexbor::element& element, const std::vector<uint8_t>& data,
    const std::string& filename, std::string_view fallbackIcon,
    const std::optional<stdfs::path>& thumbsPath)
{
    if (data.empty())
    {
        element.set_attribute("src", fallbackIcon);
    }
    else if (thumbsPath.has_value())
    {
        std::ofstream thumbStream(thumbsPath.value() / (filename + ".png"), std::ios::binary);
        thumbStream.write(reinterpret_cast<const char*>(data.data()), data.size());
        element.set_attribute("src", lexbor::string_view("thumbs/" + filename + ".png"));
    }
    else
    {
        element.set_attribute("src", lexbor::string_view(
            "data:image/png;base64," + code::base64_encode(data.data(), data.size())));
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
