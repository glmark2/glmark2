/*
 * Copyright © 2011-2012 Linaro Limited
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
#include <cstdio>
#include <getopt.h>

#include "options.h"
#include "util.h"

std::vector<std::string> Options::benchmarks;
std::vector<std::string> Options::benchmark_files;
bool Options::validate = false;
std::string Options::data_path = std::string(GLMARK_DATA_PATH);
Options::FrameEnd Options::frame_end = Options::FrameEndDefault;
Options::SwapMode Options::swap_mode = Options::SwapModeDefault;
std::pair<int,int> Options::size(800, 600);
bool Options::list_scenes = false;
bool Options::show_all_options = false;
bool Options::show_debug = false;
bool Options::show_version = false;
bool Options::show_help = false;
bool Options::reuse_context = false;
bool Options::run_forever = false;
bool Options::annotate = false;
bool Options::offscreen = false;
GLVisualConfig Options::visual_config;
bool Options::good_config = false;
Options::Results Options::results = Options::ResultsFps;
std::string Options::results_file;
std::vector<Options::WindowSystemOption> Options::winsys_options;
std::string Options::winsys_options_help;

static struct option long_options[] = {
    {"annotate", 0, 0, 0},
    {"benchmark", 1, 0, 0},
    {"benchmark-file", 1, 0, 0},
    {"validate", 0, 0, 0},
    {"data-path", 1, 0, 0},
    {"frame-end", 1, 0, 0},
    {"swap-mode", 1, 0, 0},
    {"off-screen", 0, 0, 0},
    {"visual-config", 1, 0, 0},
    {"good-config", 0, 0, 0},
    {"reuse-context", 0, 0, 0},
    {"run-forever", 0, 0, 0},
    {"size", 1, 0, 0},
    {"fullscreen", 0, 0, 0},
    {"results", 1, 0, 0},
    {"results-file", 1, 0, 0},
    {"winsys-options", 1, 0, 0},
    {"list-scenes", 0, 0, 0},
    {"show-all-options", 0, 0, 0},
    {"debug", 0, 0, 0},
    {"version", 0, 0, 0},
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
    Util::split(str, 'x', d, Util::SplitModeNormal);

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

/**
 * Parses a frame-end method string
 *
 * @param str the string to parse
 *
 * @return the parsed frame end method
 */
static Options::FrameEnd 
frame_end_from_str(const std::string &str)
{
    Options::FrameEnd m = Options::FrameEndDefault;

    if (str == "swap")
        m = Options::FrameEndSwap;
    else if (str == "finish")
        m = Options::FrameEndFinish;
    else if (str == "readpixels")
        m = Options::FrameEndReadPixels;
    else if (str == "none")
        m = Options::FrameEndNone;

    return m;
}

/**
 * Parses a swap mode string
 *
 * @param str the string to parse
 *
 * @return the parsed swap mode
 */
static Options::SwapMode
swap_mode_from_str(const std::string &str)
{
    Options::SwapMode m = Options::SwapModeDefault;

    if (str == "immediate")
        m = Options::SwapModeImmediate;
    else if (str == "mailbox")
        m = Options::SwapModeMailbox;
    else if (str == "fifo")
        m = Options::SwapModeFIFO;

    return m;
}

Options::Results
results_from_str(std::string const& str)
{
    std::vector<std::string> results_vec;
    Options::Results results = static_cast<Options::Results>(0);

    Util::split(str, ':', results_vec, Util::SplitModeNormal);

    for (auto const& res : results_vec)
    {
        if (res == "fps")
            results = static_cast<Options::Results>(results | Options::ResultsFps);
        else if (res == "cpu")
            results = static_cast<Options::Results>(results | Options::ResultsCpu);
        else if (res == "shader")
            results = static_cast<Options::Results>(results | Options::ResultsShader);
        else
            throw std::runtime_error{"Invalid result type '" + res + "'"};
    }

    return results;
}

std::vector<Options::WindowSystemOption>
winsys_options_from_str(std::string const& str)
{
    std::vector<Options::WindowSystemOption> ret;
    std::vector<std::string> opts;

    Util::split(str, ':', opts, Util::SplitModeNormal);

    for (auto const& opt : opts)
    {
        std::vector<std::string> kv;
        Util::split(opt, '=', kv, Util::SplitModeNormal);
        if (kv.size() == 2)
            ret.push_back({kv[0], kv[1]});
        else
            throw std::runtime_error{"Invalid window system option '" + opt + "'"};
    }

    return ret;
}

void
Options::print_help()
{
    printf("A benchmark for Open GL (ES) 2.0\n"
           "\n"
           "Options:\n"
           "  -b, --benchmark BENCH  A benchmark or options to run: '(scene)?(:opt1=val1)*'\n"
           "                         (the option can be used multiple times)\n"
           "  -f, --benchmark-file F Load benchmarks to run from a file containing a\n"
           "                         list of benchmark descriptions (one per line)\n"
           "                         (the option can be used multiple times)\n"
           "      --validate         Run a quick output validation test instead of \n"
           "                         running the benchmarks\n"
           "      --data-path PATH   Path to glmark2 models, shaders and textures\n"
           "                         Default: " GLMARK_DATA_PATH "\n"
           "      --frame-end METHOD How to end a frame [default,none,swap,finish,readpixels]\n"
           "      --swap-mode MODE   How to swap a frame, all modes supported only in the DRM\n"
           "                         flavor, 'fifo' available in all flavors to force vsync\n"
           "                         [default,immediate,mailbox,fifo]\n"
           "      --off-screen       Render to an off-screen surface\n"
           "      --visual-config C  The visual configuration to use for the rendering\n"
           "                         target: 'id=ID:red=R:green=G:blue=B:alpha=A:buffer=BUF:\n"
           "                         stencil=STENCIL:samples=SAMPLES'. The parameters may be\n"
           "                         defined in any order, and any omitted parameters assume a\n"
           "                         default value of '0' (id, samples), '-1' (stencil) or\n"
           "                         '1' (red, green, blue, alpha, buffer). If 'id' is set to\n"
           "                         a non-zero value, all other parameters are ignored\n"
           "      --good-config      Require a config that meets all the requested component\n"
           "                         requirements (see --visual-config)\n"
           "      --reuse-context    Use a single context for all scenes\n"
           "                         (by default, each scene gets its own context)\n"
           "  -s, --size WxH         Size of the output window (default: 800x600)\n"
           "      --fullscreen       Run in fullscreen mode (equivalent to --size -1x-1)\n"
           "      --results RESULTS  The types of results to report for each benchmark,\n"
           "                         as a ':' separated list [fps,cpu,shader]\n"
           "      --results-file F   The file to save the results to, in the format determined\n"
           "                         by the file extension [csv,xml]\n"
           "      --winsys-options O A list of 'opt=value' pairs for window system specific\n"
           "                         options, separated by ':'\n"
           "  -l, --list-scenes      Display information about the available scenes\n"
           "                         and their options\n"
           "      --show-all-options Show all scene option values used for benchmarks\n"
           "                         (only explicitly set options are shown by default)\n"
           "      --run-forever      Run indefinitely, looping from the last benchmark\n"
           "                         back to the first\n"
           "      --annotate         Annotate the benchmarks with on-screen information\n"
           "                         (same as -b :show-fps=true:title=#info#)\n"
           "  -d, --debug            Display debug messages\n"
           "      --version          Display program version\n"
           "  -h, --help             Display help\n");

    if (Options::winsys_options_help.empty())
        return;

    printf("\nWindow System options (pass in --winsys-options):\n%s",
           Options::winsys_options_help.c_str());
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

        if (!strcmp(optname, "annotate"))
            Options::annotate = true;
        if (c == 'b' || !strcmp(optname, "benchmark"))
            Options::benchmarks.push_back(optarg);
        else if (c == 'f' || !strcmp(optname, "benchmark-file"))
            Options::benchmark_files.push_back(optarg);
        else if (!strcmp(optname, "validate"))
            Options::validate = true;
        else if (!strcmp(optname, "data-path"))
            Options::data_path = std::string(optarg);
        else if (!strcmp(optname, "frame-end"))
            Options::frame_end = frame_end_from_str(optarg);
        else if (!strcmp(optname, "swap-mode"))
            Options::swap_mode = swap_mode_from_str(optarg);
        else if (!strcmp(optname, "off-screen"))
            Options::offscreen = true;
        else if (!strcmp(optname, "visual-config"))
            Options::visual_config = GLVisualConfig(optarg);
        else if (!strcmp(optname, "good-config"))
            Options::good_config = true;
        else if (!strcmp(optname, "reuse-context"))
            Options::reuse_context = true;
        else if (c == 's' || !strcmp(optname, "size"))
            parse_size(optarg, Options::size);
        else if (!strcmp(optname, "fullscreen"))
            Options::size = std::pair<int,int>(-1, -1);
        else if (!strcmp(optname, "results"))
            Options::results = results_from_str(optarg);
        else if (!strcmp(optname, "results-file"))
            Options::results_file = optarg;
        else if (!strcmp(optname, "winsys-options"))
            Options::winsys_options = winsys_options_from_str(optarg);
        else if (c == 'l' || !strcmp(optname, "list-scenes"))
            Options::list_scenes = true;
        else if (!strcmp(optname, "show-all-options"))
            Options::show_all_options = true;
        else if (!strcmp(optname, "run-forever"))
            Options::run_forever = true;
        else if (c == 'd' || !strcmp(optname, "debug"))
            Options::show_debug = true;
        else if (!strcmp(optname, "version"))
            Options::show_version = true;
        else if (c == 'h' || !strcmp(optname, "help"))
            Options::show_help = true;
    }

    return true;
}
