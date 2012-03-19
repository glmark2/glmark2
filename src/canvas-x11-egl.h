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
#ifndef GLMARK2_CANVAS_X11_EGL_H_
#define GLMARK2_CANVAS_X11_EGL_H_

#include "canvas-x11.h"

#include <EGL/egl.h>

/**
 * Canvas for rendering to an X11 window using EGL.
 */
class CanvasX11EGL : public CanvasX11
{
public:
    CanvasX11EGL(int width, int height) :
        CanvasX11(width, height), egl_display_(EGL_NO_DISPLAY),
        egl_surface_(EGL_NO_SURFACE), egl_config_(0),
        egl_context_(EGL_NO_CONTEXT) {}
    ~CanvasX11EGL() {}

protected:
    XVisualInfo *get_xvisualinfo();
    bool make_current();
    bool reset_context();
    void swap_buffers() { eglSwapBuffers(egl_display_, egl_surface_); }
    void get_glvisualinfo(GLVisualInfo &gl_visinfo);

private:
    bool ensure_egl_display();
    bool ensure_egl_config();
    bool ensure_egl_context();
    bool ensure_egl_surface();
    void init_gl_extensions();

    EGLDisplay egl_display_;
    EGLSurface egl_surface_;
    EGLConfig egl_config_;
    EGLContext egl_context_;
};

#endif

