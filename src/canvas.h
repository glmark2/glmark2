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
 */
#ifndef GLMARK2_CANVAS_H_
#define GLMARK2_CANVAS_H_

#include "oglsdl.h"
#include "mat.h"

#include <string>
#include <stdio.h>

class Canvas
{
public:
    ~Canvas() {}

    struct Pixel {
        Pixel():
            r(0), g(0), b(0), a(0) {}
        Pixel(Uint8 r, Uint8 g, Uint8 b, Uint8 a):
            r(r), g(g), b(b), a(a) {}
        Uint32 to_le32()
        {
            return static_cast<Uint32>(r) +
                   (static_cast<Uint32>(g) << 8) +
                   (static_cast<Uint32>(b) << 16) +
                   (static_cast<Uint32>(a) << 24);

        }
        Uint8 r;
        Uint8 g;
        Uint8 b;
        Uint8 a;
    };

    int mWidth;
    int mHeight;
    int mBpp;
    int mFullScreen;
    LibMatrix::mat4 mProjection;
    int mInitSuccess;

    virtual bool init() { return false; }
    virtual void visible(bool visible) { (void)visible; }
    virtual void clear() {}
    virtual void update() {}
    virtual void print_info() {}
    virtual Pixel read_pixel(int x, int y)
    {
        (void)x;
        (void)y;
        return Pixel();
    }
    virtual void write_to_file(std::string &filename) { (void)filename; }

    static Canvas &dummy()
    {
        static Canvas dummy_canvas(0, 0);
        return dummy_canvas;
    }

protected:
    Canvas(int width, int height) : mWidth(width), mHeight(height) {}
};

#endif
