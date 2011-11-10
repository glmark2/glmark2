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

#include <cstdio>
#include <cstdarg>
#include <string>
#include <sstream>

#include "options.h"
#include "log.h"

#ifdef ANDROID
#include <android/log.h>
#endif

#ifndef ANDROID

static const char *terminal_color_normal("\033[0m");
static const char *terminal_color_red("\033[1;31m");
static const char *terminal_color_cyan("\033[36m");
static const char *terminal_color_yellow("\033[33m");

static void
print_prefixed_message(FILE *stream, const char *color, const char *prefix,
                       const char *fmt, va_list ap)
{
    va_list aq;

    /* Estimate message size */
    va_copy(aq, ap);
    int msg_size = vsnprintf(NULL, 0, fmt, aq);
    va_end(aq);

    /* Create the buffer to hold the message */
    char *buf = new char[msg_size + 1];

    /* Store the message in the buffer */
    va_copy(aq, ap);
    vsnprintf(buf, msg_size + 1, fmt, aq);
    va_end(aq);

    /*
     * Print the message lines prefixed with the supplied prefix.
     * If the target stream is a terminal make the prefix colored.
     */
    bool use_color = isatty(fileno(stream));
    const char *start_color(use_color ? color : "");
    const char *end_color(use_color && *color ? terminal_color_normal : "");

    std::string line;
    std::stringstream ss(buf);

    while(std::getline(ss, line)) {
        /*
         * If this line is a continuation of a previous log message
         * just print the line plainly.
         */
        if (line[0] == LOG_CONTINUE[0]) {
            fprintf(stream, "%s", line.c_str() + 1);
        }
        else {
            /* Normal line, emit the prefix. */
            fprintf(stream, "%s%s%s: %s", start_color, prefix, end_color,
                    line.c_str());
        }

        /* Only emit a newline if the original message has it. */
        if (!(ss.rdstate() & std::stringstream::eofbit))
            fputs("\n", stream);
    }

    delete[] buf;
}

void
Log::info(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (Options::show_debug)
        print_prefixed_message(stdout, terminal_color_cyan, "Info", fmt, ap);
    else
        vfprintf(stdout, fmt, ap);
    va_end(ap);
}

void
Log::debug(const char *fmt, ...)
{
    if (!Options::show_debug)
        return;
    va_list ap;
    va_start(ap, fmt);
    print_prefixed_message(stdout, terminal_color_yellow, "Debug", fmt, ap);
    va_end(ap);
}

void
Log::error(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    print_prefixed_message(stderr, terminal_color_red, "Error", fmt, ap);
    va_end(ap);
}

void
Log::flush()
{
    fflush(stdout);
    fflush(stderr);
}
#else
void
Log::info(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    __android_log_vprint(ANDROID_LOG_INFO, "glmark2", fmt, ap);
    va_end(ap);
}

void
Log::debug(const char *fmt, ...)
{
    if (!Options::show_debug)
        return;
    va_list ap;
    va_start(ap, fmt);
    __android_log_vprint(ANDROID_LOG_DEBUG, "glmark2", fmt, ap);
    va_end(ap);
}

void
Log::error(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    __android_log_vprint(ANDROID_LOG_ERROR, "glmark2", fmt, ap);
    va_end(ap);
}

void
Log::flush()
{
}

#endif
