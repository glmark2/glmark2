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
 *  Jesse Barker
 */
#ifndef GLMARK2_CANVAS_X11_EGL_H_
#define GLMARK2_CANVAS_X11_EGL_H_

#include "canvas-x11.h"
#include "egl-state.h"

/**
 * Canvas for rendering to an X11 window using EGL.
 */
class CanvasX11EGL : public CanvasX11
{
public:
    CanvasX11EGL(int width, int height) :
        CanvasX11(width, height) {}
    ~CanvasX11EGL() {}

protected:
    XVisualInfo *get_xvisualinfo();
    bool make_current();
    bool reset_context();
    void swap_buffers();
    void get_glvisualconfig(GLVisualConfig &visual_config);
    bool init_gl_winsys();

private:
    void init_gl_extensions();
    EGLState egl_;
};

#endif
