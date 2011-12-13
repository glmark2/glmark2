/*
 * Copyright © 2011 Linaro Limited
 *
 * This file is part of glcompbench.
 *
 * glcompbench is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * glcompbench is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with glcompbench.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *  Alexandros Frantzis <alexandros.frantzis@linaro.org>
 *  Jesse Barker <jesse.barker@linaro.org>
 */

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <getopt.h>
#include <sstream>

#include "options.h"
#include "util.h"
#include "log.h"

std::vector<std::string> Options::benchmarks;
std::vector<std::string> Options::benchmark_files;
bool Options::validate = false;
bool Options::swap_buffers = true;
std::pair<int,int> Options::size(800, 600);
bool Options::list_scenes = false;
bool Options::show_all_options = false;
bool Options::show_debug = false;
bool Options::show_fps = false;
bool Options::show_help = false;

static struct option long_options[] = {
    {"benchmark", 1, 0, 0},
    {"benchmark-file", 1, 0, 0},
    {"validate", 0, 0, 0},
    {"no-swap-buffers", 0, 0, 0},
    {"size", 1, 0, 0},
    {"list-scenes", 0, 0, 0},
    {"show-all-options", 0, 0, 0},
    {"show-fps", 0, 0, 0},
    {"debug", 0, 0, 0},
    {"help", 0, 0, 0},
    {0, 0, 0, 0}
};

/**
 * Parses a size string of the form WxH
 *
 * @param str the string to parse
 * @param size the parsed size (width, height)
 */
static void
parse_size(const std::string &str, std::pair<int,int> &size)
{
    std::vector<std::string> d;
    Util::split(str, 'x', d);

    size.first = Util::fromString<int>(d[0]);

    /*
     * Parse the second element (height). If there is none, use the value
     * of the first element for the second (width = height)
     */
    if (d.size() > 1)
        size.second = Util::fromString<int>(d[1]);
    else
        size.second = size.first;
}

void
Options::print_help()
{
    printf("A benchmark for Open GL (ES) 2.0\n"
           "\n"
           "Options:\n"
           "  -b, --benchmark BENCH  A benchmark to run: 'scene(:opt1=val1)*'\n"
           "                         (the option can be used multiple times)\n"
           "  -f, --benchmark-file F Load benchmarks to run from a file containing a\n"
           "                         list of benchmark descriptions (one per line)\n"
           "                         (the option can be used multiple times)\n"
           "      --validate         Run a quick output validation test instead of \n"
           "                         running the benchmarks\n"
           "      --no-swap-buffers  Don't update the canvas by swapping the front and\n"
           "                         back buffer, use glFinish() instead\n"
           "  -s, --size WxH         Size of the output window (default: 800x600)\n"
           "  -l, --list-scenes      Display information about the available scenes\n"
           "                         and their options\n"
           "      --show-all-options Show all scene option values used for benchmarks\n"
           "                         (only explicitly set options are shown by default)\n"
           "      --show-fps         Show live FPS count on screen (showing live FPS\n"
           "                         affects benchmarking results, use with care!)\n"
           "  -d, --debug            Display debug messages\n"
           "  -h, --help             Display help\n");
}

bool
Options::parse_args(int argc, char **argv)
{
    while (1) {
        int option_index = -1;
        int c;
        const char *optname = "";

        c = getopt_long(argc, argv, "b:f:s:ldh",
                        long_options, &option_index);
        if (c == -1)
            break;
        if (c == ':' || c == '?')
            return false;

        if (option_index != -1)
            optname = long_options[option_index].name;

        if (c == 'b' || !strcmp(optname, "benchmark"))
            Options::benchmarks.push_back(optarg);
        else if (c == 'f' || !strcmp(optname, "benchmark-file"))
            Options::benchmark_files.push_back(optarg);
        else if (!strcmp(optname, "validate"))
            Options::validate = true;
        else if (!strcmp(optname, "no-swap-buffers"))
            Options::swap_buffers = false;
        else if (c == 's' || !strcmp(optname, "size"))
            parse_size(optarg, Options::size);
        else if (c == 'l' || !strcmp(optname, "list-scenes"))
            Options::list_scenes = true;
        else if (!strcmp(optname, "show-all-options"))
            Options::show_all_options = true;
        else if (!strcmp(optname, "show-fps"))
            Options::show_fps = true;
        else if (c == 'd' || !strcmp(optname, "debug"))
            Options::show_debug = true;
        else if (c == 'h' || !strcmp(optname, "help"))
            Options::show_help = true;
    }

    return true;
}
