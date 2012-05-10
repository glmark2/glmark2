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
#ifndef GLMARK2_GL_VISUAL_CONFIG_H_
#define GLMARK2_GL_VISUAL_CONFIG_H_

#include <string>

/**
 * Configuration parameters for a GL visual
 */
class GLVisualConfig
{
public:
    GLVisualConfig():
        red(1), green(1), blue(1), alpha(1), depth(1), buffer(1) {}
    GLVisualConfig(int r, int g, int b, int a, int d, int buf):
        red(r), green(g), blue(b), alpha(a), depth(d), buffer(buf) {}
    GLVisualConfig(const std::string &s);

    int red;
    int green;
    int blue;
    int alpha;
    int depth;
    int buffer;
};

#endif
