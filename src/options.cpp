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

#include "options.h"

bool Options::swap_buffers = true;
bool Options::list_scenes = false;
bool Options::show_debug = false;
bool Options::show_help = false;

static struct option long_options[] = {
    {"no-swap-buffers", 0, 0, 0},
    {"list-scenes", 0, 0, 0},
    {"debug", 0, 0, 0},
    {"help", 0, 0, 0},
    {0, 0, 0, 0}
};

void
Options::print_help()
{
    printf("A benchmark for Open GL (ES) 2.0\n"
           "\n"
           "Options:\n"
           "  --no-swap-buffers  Don't update the screen by swapping the front and\n"
           "                     back buffer, use glFinish() instead\n"
           "  --list-scenes      Display information about the available scenes\n"
           "                     and their options\n"
           "  --debug            Display debug messages\n"
           "  --help             Display help\n");
}

bool
Options::parse_args(int argc, char **argv)
{
    while (1) {
        int option_index = 0;
        int c;
        const char *optname;

        c = getopt_long(argc, argv, "",
                        long_options, &option_index);
        if (c == -1)
            break;
        if (c == ':' || c == '?')
            return false;

       optname = long_options[option_index].name;

       if (!strcmp(optname, "no-swap-buffers"))
           Options::swap_buffers = false;
       else if (!strcmp(optname, "list-scenes"))
           Options::list_scenes = true;
       else if (!strcmp(optname, "debug"))
           Options::show_debug = true;
       else if (!strcmp(optname, "help"))
           Options::show_help = true;
    }

    return true;
}
