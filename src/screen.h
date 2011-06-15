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
#ifndef GLMARK2_SCREEN_H_
#define GLMARK2_SCREEN_H_

#include "oglsdl.h"
#include "matrix.h"

#include <stdio.h>

class Screen
{
public:
    ~Screen() {}

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
    Matrix4f mProjection;
    int mInitSuccess;

    virtual void clear() {}
    virtual void update() {}
    virtual void print_info() {}
    virtual Pixel read_pixel(int x, int y)
    {
        (void)x;
        (void)y;
        return Pixel();
    }

    static Screen &dummy()
    {
        static Screen dummy_screen;
        return dummy_screen;
    }

protected:
    Screen() {}
};

#endif
