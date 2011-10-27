/*
 * Copyright © 2008 Ben Smith
 * Copyright © 2010-2011 Linaro Limited
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
 *  Ben Smith (original glmark benchmark)
 *  Alexandros Frantzis (glmark2)
 *  Jesse Barker
 */
#ifndef GLMARK2_CANVAS_H_
#define GLMARK2_CANVAS_H_

#include "gl-headers.h"
#include "mat.h"

#include <sys/types.h>
#include <string>
#include <stdio.h>

class Canvas
{
public:
    virtual ~Canvas() {}

    struct Pixel {
        Pixel():
            r(0), g(0), b(0), a(0) {}
        Pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a):
            r(r), g(g), b(b), a(a) {}
        uint32_t to_le32()
        {
            return static_cast<uint32_t>(r) +
                   (static_cast<uint32_t>(g) << 8) +
                   (static_cast<uint32_t>(b) << 16) +
                   (static_cast<uint32_t>(a) << 24);

        }
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };

    virtual bool init() { return false; }
    virtual void visible(bool visible) { static_cast<void>(visible); }
    virtual void clear() {}
    virtual void update() {}
    virtual void print_info() {}
    virtual Pixel read_pixel(int x, int y)
    {
        static_cast<void>(x);
        static_cast<void>(y);
        return Pixel();
    }
    virtual void write_to_file(std::string &filename) { static_cast<void>(filename); }
    virtual bool should_quit() { return false; }
    virtual void resize(int width, int height) { static_cast<void>(width); static_cast<void>(height); }

    static Canvas &dummy()
    {
        static Canvas dummy_canvas(0, 0);
        return dummy_canvas;
    }

    int width() { return width_; }
    int height() { return height_; }
    const LibMatrix::mat4 &projection() { return projection_; }

protected:
    Canvas(int width, int height) : width_(width), height_(height) {}

    int width_;
    int height_;
    LibMatrix::mat4 projection_;
};

#endif
