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

#include <sstream>

#include "util.h"

/** 
 * Splits a string using a delimiter
 * 
 * @param s the string to split
 * @param delim the delimitir to use
 * @param elems the string vector to populate
 */
void
Util::split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);

    std::string item;
    while(std::getline(ss, item, delim))
        elems.push_back(item);
}

