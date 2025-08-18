#include "dirdocument.h"
#include "include/alphanum.h"
#include <fstream>
#include <iostream>
#include <magic.h>
#include <map>
#include <taglib/tdebuglistener.h>
#include <tbb/parallel_for_each.h>

extern "C" {
#include <libavutil/log.h>
}

namespace stdfs = std::filesystem;

template<doj::StringComparison Comp>
struct doj::alphanum_less<stdfs::path, Comp>
{
    bool operator()(const stdfs::path& left, const stdfs::path& right) const
    {
        using path_string_view = std::basic_string_view<stdfs::path::value_type>;
        path_string_view leftView(left.native());
        path_string_view rightView(right.native());

        size_t leftDot = leftView.find_last_of('.');
        size_t rightDot = rightView.find_last_of('.');

        // first check the "bases" of the inputs
        path_string_view leftBase = leftView.substr(0, leftDot);
        path_string_view rightBase = rightView.substr(0, rightDot);
        const int res = doj::alphanum_comp(leftBase, rightBase, Comp);
        if (res != 0 || (leftBase.size() == leftView.size() && rightBase.size() == rightView.size()))
            return res < 0;

        // if those are equal, then check the extensions
        path_string_view leftExt = leftDot != path_string_view::npos
            ? leftView.substr(leftDot) : path_string_view();
        path_string_view rightExt = rightDot != path_string_view::npos
            ? rightView.substr(rightDot) : path_string_view();
        return doj::alphanum_comp(leftExt, rightExt, Comp) < 0;
    }
};

struct NothingListener : TagLib::DebugListener { void printMessage(const TagLib::String&) override {} };

using FileDocumentMap = std::map<stdfs::path, DirDocument, doj::alphanum_less<stdfs::path, doj::CASE_INSENSITIVE>>;
using FilePropertyMap = std::map<stdfs::path, FileProperties, doj::alphanum_less<stdfs::path, doj::CASE_INSENSITIVE>>;

FilePropertyMap getSortedFiles(const stdfs::path& dirPath)
{
    std::vector<stdfs::path> entries;
    for (const stdfs::directory_entry& entry : stdfs::recursive_directory_iterator(dirPath))
        if (entry.is_regular_file())
            entries.push_back(entry.path());

    FilePropertyMap result;

    tbb::parallel_for_each(std::make_move_iterator(entries.begin()), std::make_move_iterator(entries.end()), [&result](stdfs::path&& path) {
        // magic_set is not thread-safe
        static thread_local std::unique_ptr<magic_set, decltype(&magic_close)> magic(
            magic_open(MAGIC_MIME_TYPE), &magic_close);
        if (!magic || magic_load(magic.get(), nullptr) != 0)
        {
            std::cerr << "Failed to initialize magic library" << std::endl;
            exit(EXIT_FAILURE);
        }

        const std::string pathStr = path.string();
        const std::string mimeType = magic_file(magic.get(), pathStr.c_str());

        if (mimeType.rfind("audio/", 0) == 0)
        {
            if (std::optional<AudioProperties> props = openAsAudio(pathStr))
            {
                result.emplace(std::move(path), std::move(*props));
            }
            else
            {
                std::cout << "Could not handle " << pathStr << " as an audio file. "
                          << "Handling as a normal file." << std::endl;
                result.emplace(std::move(path), std::monostate{});
            }
        }
        else if (mimeType.rfind("image/", 0) == 0)
        {
            if (nonstd::expected<ImageProperties, std::string> props = openAsImage(pathStr))
            {
                result.emplace(std::move(path), std::move(*props));
            }
            else
            {
                std::cout << "Could not handle " << pathStr << " as an image file: "
                          << props.error() << ". Handling as normal file." << std::endl;
                result.emplace(std::move(path), std::monostate{});
            }
        }
        else if (mimeType.rfind("video/", 0) == 0)
        {
            if (auto [data, message] = openAsVideo(pathStr); data.has_value() && !data->thumbnailData.empty())
            {
                result.emplace(std::move(path), std::move(*data));
            }
            else if (data.has_value())
            {
                std::cout << "Could not generate thumbnail for video file " << pathStr << ": " << message << std::endl;
                result.emplace(std::move(path), std::move(*data));
            }
            else
            {
                std::cout << "Could not handle " << pathStr << " as a video file: "
                          << message << ". Handling as normal file." << std::endl;
                result.emplace(std::move(path), std::monostate{});
            }
        }
        else
        {
            result.emplace(std::move(path), std::monostate{});
        }
    });

    return result;
}

int main(int argc, char** argv)
{
    stdfs::path searchPath = argc >= 2 ? argv[1] : stdfs::path();
    if (!stdfs::is_directory(searchPath))
    {
        std::cerr << "Need directory as argument" << std::endl;
        return EXIT_FAILURE;
    }

    // libav and taglib absolutely spam the console with crud on default settings
    // sadly taglib has no concept of log levels, so we have to disable logging entirely
    static NothingListener* nl = new NothingListener;
    av_log_set_level(AV_LOG_ERROR);
    TagLib::setDebugListener(nl);

    FilePropertyMap fileMap = getSortedFiles(searchPath);
    if (fileMap.empty())
    {
        std::cout << "Directory is empty. No need to continue." << std::endl;
        return EXIT_SUCCESS;
    }

    // create out directories + blank documents for searchPath and its parent paths
    // even if some dirs may have no files of interest, they are still worth keeping for navigation purposes
    FileDocumentMap docs = { { searchPath, DirDocument() } };
    for (const stdfs::directory_entry& entry : stdfs::recursive_directory_iterator(searchPath))
        if (entry.is_directory())
            docs.emplace(entry.path(), DirDocument());

    // go through and add files to their respective docs
    for (auto& [parentPath, doc] : docs)
    {
        for (auto it = fileMap.begin(); it != fileMap.end();)
        {
            if (it->first.parent_path() == parentPath)
            {
                auto node = fileMap.extract(it++);
                doc.addFileEntry(node.key(), node.mapped());
            }
            else
            {
                ++it;
            }
        }
    }

    // relativize all of the paths in the docs map to out, create needed dirs
    {
        FileDocumentMap updatedDocs;
        for (auto& [parentPath, doc] : docs)
        {
            stdfs::path outPath = parentPath != searchPath ? "out" / stdfs::relative(parentPath, searchPath) : "out";
            stdfs::create_directories(outPath);
            updatedDocs.emplace(std::move(outPath), std::move(doc));
        }
        docs = std::move(updatedDocs);
    }

    // iterate through our now fully setup map, finalize the docs and write them
    for (auto& [outPath, doc] : docs)
    {
        std::vector<std::pair<std::string, std::string>> navLinks;

        // add link to go up a folder
        if (outPath != "out")
            if (stdfs::path parent = outPath.parent_path(); docs.find(parent) != docs.end())
                navLinks.emplace_back(stdfs::relative(parent / "index.html", outPath).string(), "⬆️ Up");

        // add links to subdirs
        for (const auto& [dir, _] : docs)
        {
            if (dir.parent_path().lexically_normal() == outPath.lexically_normal())
            {
                navLinks.emplace_back(
                    stdfs::relative(dir / "index.html", outPath).string(),
                    "📁 " + dir.filename().string());
            }
        }

        // finalize doc
        doc.addNavigationHeader(navLinks);
        doc.finalize();

        // write doc
        std::ofstream docFile(outPath / "index.html");
        docFile << doc.ToString(CTML::ToStringOptions(CTML::StringFormatting::MULTIPLE_LINES, true, 4, false));
    }
}
