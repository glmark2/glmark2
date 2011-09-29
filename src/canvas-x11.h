/*
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
 *  Alexandros Frantzis (glmark2)
 */
#ifndef GLMARK2_CANVAS_X11_H_
#define GLMARK2_CANVAS_X11_H_

#include "canvas.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>

class CanvasX11 : public Canvas
{
public:
    ~CanvasX11() {}

    virtual bool init();
    virtual void visible(bool visible);
    virtual void clear();
    virtual void update();
    virtual void print_info();
    virtual Pixel read_pixel(int x, int y);
    virtual void write_to_file(std::string &filename);
    virtual bool should_quit();
    virtual void resize(int width, int height);

protected:
    CanvasX11(int width, int height) :
        Canvas(width, height), xwin_(0), xdpy_(0) {}

    virtual XVisualInfo *get_xvisualinfo() = 0;
    virtual bool make_current() = 0;
    virtual void swap_buffers() = 0;
    bool supports_gl2();

    Window xwin_;
    Display *xdpy_;
};

#endif

