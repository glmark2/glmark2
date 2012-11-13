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
#include "canvas-x11-egl.h"
#include "log.h"
#include "options.h"

#include <fstream>
#include <sstream>
#include <climits>

/*********************
 * Protected methods *
 *********************/

bool
CanvasX11EGL::init_gl_winsys()
{
    egl_.init_display(reinterpret_cast<EGLNativeDisplayType>(xdpy_),
                      visual_config_);
    return true;
}

XVisualInfo *
CanvasX11EGL::get_xvisualinfo()
{
    XVisualInfo vis_tmpl;
    XVisualInfo *vis_info;
    int num_visuals;
    EGLint vid;

    if (!egl_.gotNativeConfig(vid))
    {
        Log::error("Failed to get a native-renderable EGLConfig\n");
        return 0;
    }

    /* The X window visual must match the EGL config */
    vis_tmpl.visualid = vid;
    vis_info = XGetVisualInfo(xdpy_, VisualIDMask, &vis_tmpl,
                             &num_visuals);
    if (!vis_info) {
        Log::error("couldn't get X visual\n");
        return 0;
    }

    return vis_info;
}

bool
CanvasX11EGL::make_current()
{
    egl_.init_surface(reinterpret_cast<EGLNativeWindowType>(xwin_));
    if (!egl_.valid()) {
        Log::error("CanvasX11EGL: Invalid EGL state\n");
        return false;
    }

    init_gl_extensions();

    return true;
}

void
CanvasX11EGL::swap_buffers()
{
    egl_.swap();
}

void
CanvasX11EGL::get_glvisualconfig(GLVisualConfig &visual_config)
{
    egl_.getVisualConfig(visual_config);
}

/*******************
 * Private methods *
 *******************/

bool
CanvasX11EGL::reset_context()
{
    return egl_.reset();
}

void
CanvasX11EGL::init_gl_extensions()
{
#if USE_GLESv2
    if (GLExtensions::support("GL_OES_mapbuffer")) {
        GLExtensions::MapBuffer =
            reinterpret_cast<PFNGLMAPBUFFEROESPROC>(eglGetProcAddress("glMapBufferOES"));
        GLExtensions::UnmapBuffer =
            reinterpret_cast<PFNGLUNMAPBUFFEROESPROC>(eglGetProcAddress("glUnmapBufferOES"));
    }
#elif USE_GL
    GLExtensions::MapBuffer = glMapBuffer;
    GLExtensions::UnmapBuffer = glUnmapBuffer;
#endif
}
