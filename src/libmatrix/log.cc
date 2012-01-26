//
// Copyright (c) 2010-2012 Linaro Limited
//
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the MIT License which accompanies
// this distribution, and is available at
// http://www.opensource.org/licenses/mit-license.php
//
// Contributors:
//     Alexandros Frantzis <alexandros.frantzis@linaro.org>
//     Jesse Barker <jesse.barker@linaro.org>
//
#include <cstdio>
#include <cstdarg>
#include <string>
#include <sstream>
#include <iostream>
#include "log.h"

#ifdef ANDROID
#include <android/log.h>
#endif

using std::string;

const string Log::continuation_prefix("\x10");
string Log::appname;
bool Log::do_debug_(false);

#ifndef ANDROID

static const string terminal_color_normal("\033[0m");
static const string terminal_color_red("\033[1;31m");
static const string terminal_color_cyan("\033[36m");
static const string terminal_color_yellow("\033[33m");
static const string empty;

static void
print_prefixed_message(std::ostream& stream, const string& color, const string& prefix,
                       const string& fmt, va_list ap)
{
    va_list aq;

    /* Estimate message size */
    va_copy(aq, ap);
    int msg_size = vsnprintf(NULL, 0, fmt.c_str(), aq);
    va_end(aq);

    /* Create the buffer to hold the message */
    char *buf = new char[msg_size + 1];

    /* Store the message in the buffer */
    va_copy(aq, ap);
    vsnprintf(buf, msg_size + 1, fmt.c_str(), aq);
    va_end(aq);

    /*
     * Print the message lines prefixed with the supplied prefix.
     * If the target stream is a terminal make the prefix colored.
     */
    string start_color;
    string end_color;
    static const string colon(": ");
    if (!color.empty())
    {
        start_color = color;
        if (color[0] != 0)
        {
            end_color = terminal_color_normal;
        }
    }

    std::string line;
    std::stringstream ss(buf);

    while(std::getline(ss, line)) {
        /*
         * If this line is a continuation of a previous log message
         * just print the line plainly.
         */
        if (line[0] == Log::continuation_prefix[0]) {
            stream << line.c_str() + 1;
        }
        else {
            /* Normal line, emit the prefix. */
            stream << start_color << prefix << end_color << colon << line;
        }

        /* Only emit a newline if the original message has it. */
        if (!(ss.rdstate() & std::stringstream::eofbit))
            stream << std::endl;
    }

    delete[] buf;
}

void
Log::info(const char *fmt, ...)
{
    static const string infoprefix("Info");
    static const string& infocolor(isatty(fileno(stdout)) ? terminal_color_cyan : empty);
    va_list ap;
    va_start(ap, fmt);
    if (do_debug_)
        print_prefixed_message(std::cout, infocolor, infoprefix, fmt, ap);
    else
        print_prefixed_message(std::cout, empty, empty, fmt, ap);
    va_end(ap);
}

void
Log::debug(const char *fmt, ...)
{
    static const string dbgprefix("Debug");
    static const string& dbgcolor(isatty(fileno(stdout)) ? terminal_color_yellow : empty);
    if (!do_debug_)
        return;
    va_list ap;
    va_start(ap, fmt);
    print_prefixed_message(std::cout, dbgcolor, dbgprefix, fmt, ap);
    va_end(ap);
}

void
Log::error(const char *fmt, ...)
{
    static const string errprefix("Error");
    static const string& errcolor(isatty(fileno(stderr)) ? terminal_color_red : empty);
    va_list ap;
    va_start(ap, fmt);
    print_prefixed_message(std::cerr, errcolor, errprefix, fmt, ap);
    va_end(ap);
}

void
Log::flush()
{
    std::cout.flush();
    std::cerr.flush();
}
#else
void
Log::info(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    __android_log_vprint(ANDROID_LOG_INFO, appname.c_str(), fmt, ap);
    va_end(ap);
}q

void
Log::debug(const char *fmt, ...)
{
    if (!Options::show_debug)
        return;
    va_list ap;
    va_start(ap, fmt);
    __android_log_vprint(ANDROID_LOG_DEBUG, appname.c_str(), fmt, ap);
    va_end(ap);
}

void
Log::error(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    __android_log_vprint(ANDROID_LOG_ERROR, appname.c_str(), fmt, ap);
    va_end(ap);
}

void
Log::flush()
{
}

#endif
