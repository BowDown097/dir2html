#pragma once
#include <cxxopts.hpp>

namespace Commands
{
    void entryFor(const cxxopts::parse_result& result);
    void help(const cxxopts::options& options);
    //int merge(const cxxopts::parse_result& result);
    int runDefault(const cxxopts::parse_result& result);
}
