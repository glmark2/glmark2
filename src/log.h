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

#ifndef LOG_H_
#define LOG_H_

/**
 * A prefix that informs the logging infrastructure that the log
 * message is a continuation of a previous log message to be put
 * on the same line.
 */
#define LOG_CONTINUE "\x10"

class Log
{
public:
    static void info(const char *fmt, ...);
    static void debug(const char *fmt, ...);
    static void error(const char *fmt, ...);
    static void flush();
};

#endif /* LOG_H_ */
