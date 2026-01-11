#include "dirdocument.h"
#include "alphanum.h"
#include <cxxopts.hpp>
#include <fstream>
#include <iostream>
#include <taglib/tdebuglistener.h>
#include <tbb/concurrent_map.h>
#include <tbb/parallel_for_each.h>

extern "C" {
#include <libavutil/log.h>
}

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

using FileDocumentMap = std::map<stdfs::path, DirDocument,
    doj::alphanum_less<stdfs::path, doj::CASE_INSENSITIVE>>;
using FileEntryMap = tbb::concurrent_map<stdfs::path, FileProperties,
    doj::alphanum_less<stdfs::path, doj::CASE_INSENSITIVE>>;

FileEntryMap getSortedFiles(const stdfs::path& dirPath)
{
    std::vector<stdfs::path> entries;
    for (const stdfs::directory_entry& entry : stdfs::recursive_directory_iterator(dirPath))
        if (entry.is_regular_file())
            entries.push_back(entry.path());

    FileEntryMap result;
    tbb::parallel_for_each(entries, [&](const stdfs::path& path) {
        result.emplace(path, propertiesFor(path));
    });
    return result;
}

static const CTML::ToStringOptions stropts(CTML::StringFormatting::MULTIPLE_LINES, true, 4, false);

int main(int argc, char** argv)
{
    // libav and taglib absolutely spam the console with crud on default settings
    // sadly taglib has no concept of log levels, so we have to disable logging entirely
    static NothingListener* nl = new NothingListener;
    av_log_set_level(AV_LOG_ERROR);
    TagLib::setDebugListener(nl);

    cxxopts::options options(argv[0], "Generate an HTML view of a directory.");
    options.allow_unrecognised_options().add_options()
        ("entry-for", "Print the HTML entry for a given file", cxxopts::value<std::string>())
        ("external-thumbs", "Store thumbnails in a separate folder", cxxopts::value<bool>())
        ("help", "Print help");

    cxxopts::parse_result result = options.parse(argc, argv);
    if (result.has("entry-for"))
    {
        stdfs::path filePath(result["entry-for"].as<std::string>());
        std::cout << DirDocument::createFileNode(filePath).ToString(stropts) << std::endl;
    }
    else if (result.has("help"))
    {
        std::cout << options.help() << std::endl;
    }
    else if (result.unmatched().empty() || !stdfs::is_directory(result.unmatched().front()))
    {
        std::cerr << "Need directory as argument" << std::endl;
        return EXIT_FAILURE;
    }
    else
    {
        bool externalThumbs = result["external-thumbs"].as<bool>();
        stdfs::path searchPath(result.unmatched().front());
        FileEntryMap fileMap = getSortedFiles(searchPath);

        if (fileMap.empty())
        {
            std::cout << "Directory is empty. No need to continue." << std::endl;
            return EXIT_SUCCESS;
        }

        // create out directories + blank documents for searchPath and its parent paths
        // even if some dirs may have no files of interest, they are still worth keeping for navigation purposes
        FileDocumentMap docs;
        docs.try_emplace(searchPath);
        for (const stdfs::directory_entry& entry : stdfs::recursive_directory_iterator(searchPath))
            if (entry.is_directory())
                docs.try_emplace(entry.path());

        // go through and add files to their respective docs,
        // relativizing paths to the out folder and updating the docs map with them along the way
        {
            FileDocumentMap updatedDocs;
            for (auto& [parentPath, doc] : docs)
            {
                stdfs::path outPath = "out";
                if (parentPath != searchPath)
                    outPath /= stdfs::relative(parentPath, searchPath);

                std::optional<stdfs::path> thumbsPath;
                if (externalThumbs)
                    thumbsPath = outPath / "thumbs";
                stdfs::create_directories(thumbsPath.value_or(outPath));

                for (auto it2 = fileMap.begin(); it2 != fileMap.end();)
                {
                    if (it2->first.parent_path() == parentPath)
                    {
                        doc.addFileEntry(doc.createFileNode(it2->first, it2->second, thumbsPath));
                        it2 = fileMap.unsafe_erase(it2);
                    }
                    else
                    {
                        ++it2;
                    }
                }

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
                if (dir.parent_path() == outPath)
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
            docFile << doc.ToString(stropts);
        }
    }
}
