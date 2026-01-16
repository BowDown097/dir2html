#include "commands.h"
#include <taglib/tdebuglistener.h>

extern "C" {
#include <libavutil/log.h>
}

struct NothingListener : TagLib::DebugListener
{
    void printMessage(const TagLib::String&) override {}
};

int main(int argc, char** argv)
{
    // libav and taglib absolutely spam the console with crud on default settings
    // sadly taglib has no concept of log levels, so we have to disable logging entirely
    av_log_set_level(AV_LOG_ERROR);
    std::unique_ptr<NothingListener> listener = std::make_unique<NothingListener>();
    TagLib::setDebugListener(listener.get());

    cxxopts::options options(argv[0], "Generate an HTML view of a directory.");
    options.allow_unrecognised_options().add_options()
        ("drop", "Drop a file from a given already generated HTML listing")
        ("entry-for", "Print the HTML entry for a given file", cxxopts::value<std::string>())
        ("external-thumbs", "Store thumbnails in a separate folder", cxxopts::value<bool>())
        ("help", "Print help")
        ("merge", "Merge an entry for a file into a given already generated HTML listing");

    cxxopts::parse_result result = options.parse(argc, argv);
    if (result.has("drop"))
        return Commands::drop(result);
    else if (result.has("entry-for"))
        Commands::entryFor(result);
    else if (result.has("help"))
        Commands::help(options);
    else if (result.has("merge"))
        return Commands::merge(result);
    else
        return Commands::runDefault(result);
}
