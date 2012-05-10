/*
 * Copyright © 2012 Linaro Limited
 *
 * This file is part of the glmark2 OpenGL (ES) 2.0 benchmark.
 *
 * glmark2 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * glmark2 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * glmark2.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *  Alexandros Frantzis <alexandros.frantzis@linaro.org>
 */
#include "gl-visual-config.h"
#include "util.h"
#include "log.h"

#include <vector>

GLVisualConfig::GLVisualConfig(const std::string &s) :
    red(1), green(1), blue(1), alpha(1), depth(1), buffer(1)
{
    std::vector<std::string> elems;

    Util::split(s, ':', elems);

    for (std::vector<std::string>::const_iterator iter = elems.begin();
         iter != elems.end();
         iter++)
    {
        std::vector<std::string> opt;

        Util::split(*iter, '=', opt);
        if (opt.size() == 2) {
            if (opt[0] == "r" || opt[0] == "red")
                red = Util::fromString<int>(opt[1]);
            else if (opt[0] == "g" || opt[0] == "green")
                green = Util::fromString<int>(opt[1]);
            else if (opt[0] == "b" || opt[0] == "blue")
                blue = Util::fromString<int>(opt[1]);
            else if (opt[0] == "a" || opt[0] == "alpha")
                alpha = Util::fromString<int>(opt[1]);
            else if (opt[0] == "d" || opt[0] == "depth")
                depth = Util::fromString<int>(opt[1]);
            else if (opt[0] == "buf" || opt[0] == "buffer")
                buffer = Util::fromString<int>(opt[1]);
        }
        else
            Log::info("Warning: ignoring invalid option string '%s' "
                      "in GLVisualConfig description\n",
                      iter->c_str());
    }
}

