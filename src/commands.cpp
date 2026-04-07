#include "commands.h"
#include "dirdocument.h"
#include "alphanum.h"
#include <fstream>
#include <iostream>
#include <tbb/concurrent_map.h>
#include <tbb/parallel_for_each.h>
#include <tidy.h>
#include <tidybuffio.h>

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

using FileDocumentMap = std::map<stdfs::path, DirDocument,
    doj::alphanum_less<stdfs::path, doj::CASE_INSENSITIVE>>;
using FileEntryMap = tbb::concurrent_map<stdfs::path, FileProperties,
    doj::alphanum_less<stdfs::path, doj::CASE_INSENSITIVE>>;

// lexbor's tidying inserts random quotes and breaks the document for some reason.
// so, we're using tidy instead.
std::string beautifyHTML(const std::string& data)
{
    TidyDoc doc = tidyCreate();
    TidyBuffer output = {0};

    tidyBufInit(&output);

    tidyOptSetBool(doc, TidyHtmlOut, yes);
    tidyOptSetBool(doc, TidyIndentContent, yes);
    tidyOptSetBool(doc, TidyShowWarnings, no);
    tidyOptSetInt(doc, TidyIndentSpaces, 4);
    tidyOptSetInt(doc, TidyWrapLen, 0);
    tidyOptSetValue(doc, TidyCharEncoding, "utf8");

    tidyParseString(doc, data.c_str());
    tidyCleanAndRepair(doc);
    tidySaveBuffer(doc, &output);

    std::string result(reinterpret_cast<const char*>(output.bp), output.size ? output.size - 1 : 0);

    tidyBufFree(&output);
    tidyRelease(doc);

    return result;
}

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

std::optional<stdfs::path> getThumbsPath(const stdfs::path& outPath, bool externalThumbs)
{
    std::optional<stdfs::path> thumbsPath;
    if (externalThumbs)
        thumbsPath = outPath / "thumbs";
    stdfs::create_directories(thumbsPath.value_or(outPath));
    return thumbsPath;
}

namespace Commands
{
    int beautify(const cxxopts::parse_result& result)
    {
        if (result.unmatched().empty())
        {
            std::cerr << "Need HTML listing as argument" << std::endl;
            return EXIT_FAILURE;
        }

        stdfs::path listingPath(result.unmatched()[0]);
        if (!stdfs::is_regular_file(listingPath))
        {
            std::cerr << "Listing path '" << listingPath << "' must be regular file" << std::endl;
            return EXIT_FAILURE;
        }

        DirDocument doc(listingPath);

        std::ofstream docFile(listingPath);
        docFile << beautifyHTML(doc.serialize());

        return EXIT_SUCCESS;
    }

    int drop(const cxxopts::parse_result& result)
    {
        if (result.unmatched().size() < 2)
        {
            std::cerr << "Need HTML listing and file[name] as arguments" << std::endl;
            return EXIT_FAILURE;
        }

        stdfs::path listingPath(result.unmatched()[0]);
        if (!stdfs::is_regular_file(listingPath))
        {
            std::cerr << "Listing path '" << listingPath << "' must be regular file" << std::endl;
            return EXIT_FAILURE;
        }

        std::string filename;
        if (stdfs::path filePath(result.unmatched()[1]); stdfs::is_regular_file(filePath))
            filename = filePath.filename();
        else
            filename = result.unmatched()[1];

        DirDocument doc(listingPath);
        if (!doc.dropFile(filename))
        {
            std::cerr << "No file found in listing by the name '" << filename << '\'' << std::endl;
            return EXIT_FAILURE;
        }

        std::ofstream docFile(listingPath);
        docFile << beautifyHTML(doc.serialize());

        return EXIT_SUCCESS;
    }

    void entryFor(const cxxopts::parse_result& result)
    {
        stdfs::path filePath(result["entry-for"].as<std::string>());
        DirDocument doc;
        std::cout << doc.createFileElement(filePath).serialize() << std::endl;
    }

    void help(const cxxopts::options& options)
    {
        std::cout << options.help() << std::endl;
    }

    int merge(const cxxopts::parse_result& result)
    {
        if (result.unmatched().size() < 2)
        {
            std::cerr << "Need HTML listing and file as arguments" << std::endl;
            return EXIT_FAILURE;
        }

        stdfs::path listingPath(result.unmatched()[0]);
        stdfs::path filePath(result.unmatched()[1]);

        if (!stdfs::is_regular_file(listingPath) || !stdfs::is_regular_file(filePath))
        {
            std::cerr << "Arguments "
                "('" << listingPath << "' '" << filePath << "') "
                "must be regular files" << std::endl;
            return EXIT_FAILURE;
        }

        bool externalThumbs = result["external-thumbs"].as<bool>();
        std::optional<stdfs::path> thumbsPath = getThumbsPath(listingPath.parent_path(), externalThumbs);

        DirDocument doc(listingPath);
        lexbor::element element = doc.createFileElement(filePath, propertiesFor(filePath), thumbsPath);

        if (!doc.mergeFileEntry(std::move(element), filePath.filename()))
        {
            std::cerr << "Failed to merge entry for file '" << filePath << '\'' << std::endl;
            return EXIT_FAILURE;
        }

        std::ofstream docFile(listingPath);
        docFile << beautifyHTML(doc.serialize());

        return EXIT_SUCCESS;
    }

    int move(const cxxopts::parse_result& result)
    {
        if (result.unmatched().size() < 3)
        {
            std::cerr << "Need two HTML listings and filename as arguments" << std::endl;
            return EXIT_FAILURE;
        }

        stdfs::path fromListingPath(result.unmatched()[0]);
        stdfs::path toListingPath(result.unmatched()[1]);
        const std::string& filename = result.unmatched()[2];

        if (!stdfs::is_regular_file(fromListingPath) || !stdfs::is_regular_file(toListingPath))
        {
            std::cerr << "First two arguments "
                "('" << fromListingPath << "' '" << toListingPath << "') "
                "must be regular files" << std::endl;
            return EXIT_FAILURE;
        }

        DirDocument fromDoc(fromListingPath);
        DirDocument toDoc(toListingPath);

        std::optional<lexbor::element> elem = fromDoc.dropFile(filename);
        if (!elem.has_value())
        {
            std::cerr << "No file found in '" << fromListingPath << "' by the name '" << filename << '\'' << std::endl;
            return EXIT_FAILURE;
        }

        if (!toDoc.mergeFileEntry(std::move(elem.value()), filename))
        {
            std::cerr << "Failed to merge entry for file into '" << toListingPath << '\'' << std::endl;
            return EXIT_FAILURE;
        }

        std::ofstream fromDocFile(fromListingPath);
        fromDocFile << beautifyHTML(fromDoc.serialize());

        std::ofstream toDocFile(toListingPath);
        toDocFile << beautifyHTML(toDoc.serialize());

        return EXIT_SUCCESS;
    }

    int runDefault(const cxxopts::parse_result& result)
    {
        if (result.unmatched().empty() || !stdfs::is_directory(result.unmatched()[0]))
        {
            std::cerr << "Need directory as argument" << std::endl;
            return EXIT_FAILURE;
        }

        bool externalThumbs = result["external-thumbs"].as<bool>();
        stdfs::path searchPath(result.unmatched()[0]);
        FileEntryMap fileMap = getSortedFiles(searchPath);

        if (fileMap.empty())
        {
            std::cout << "Directory '" << searchPath << "' is empty. No need to continue." << std::endl;
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

                std::optional<stdfs::path> thumbsPath = getThumbsPath(outPath, externalThumbs);
                for (auto it2 = fileMap.begin(); it2 != fileMap.end();)
                {
                    if (it2->first.parent_path() == parentPath)
                    {
                        doc.addFileEntry(doc.createFileElement(it2->first, it2->second, thumbsPath));
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
            docFile << beautifyHTML(doc.serialize());
        }

        return EXIT_SUCCESS;
    }
}
