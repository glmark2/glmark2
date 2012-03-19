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
#ifndef GLMARK2_CANVAS_X11_GLX_H_
#define GLMARK2_CANVAS_X11_GLX_H_

#include "canvas-x11.h"

#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#include <GL/glxext.h>

/**
 * Canvas for rendering to an X11 window using GLX.
 */
class CanvasX11GLX : public CanvasX11
{
public:
    CanvasX11GLX(int width, int height) :
        CanvasX11(width, height), glx_fbconfig_(0), glx_context_(0) {}
    ~CanvasX11GLX() {}

protected:
    XVisualInfo *get_xvisualinfo();
    bool make_current();
    bool reset_context();
    void swap_buffers() { glXSwapBuffers(xdpy_, xwin_); }
    void get_glvisualinfo(GLVisualInfo &gl_visinfo);

private:
    bool check_glx_version();
    void init_extensions();
    bool ensure_glx_fbconfig();
    bool ensure_glx_context();
    void init_gl_extensions();

    GLXFBConfig glx_fbconfig_;
    GLXContext glx_context_;

};

#endif

