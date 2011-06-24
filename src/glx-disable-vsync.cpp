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

#if USE_GL

#include "glx-disable-vsync.h"
#include <SDL_syswm.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xlib.h>

#include <string>
#include "log.h"

PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT_ = 0;
PFNGLXSWAPINTERVALMESAPROC glXSwapIntervalMESA_ = 0;

void
glx_disable_vsync()
{
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);

    if (SDL_GetWMInfo(&info) != 1) {
        Log::error("Couldn't get windowing system info from SDL.\n");
        return;
    }

    Display *display = info.info.x11.gfxdisplay;
    Window window = info.info.x11.window;

    std::string extString;
    const char* exts = glXQueryExtensionsString(display, 0);
    if (exts) {
        extString = exts;
    }

    /*
     * GLX_EXT_swap_control or GL_MESA_swap_control. Note that
     * GLX_SGI_swap_control is not enough because it doesn't allow 0 as a valid
     * value (i.e. we can't turn off VSync).
     */
    if (extString.find("GLX_EXT_swap_control") != std::string::npos) {
        glXSwapIntervalEXT_ =
            reinterpret_cast<PFNGLXSWAPINTERVALEXTPROC>(
                glXGetProcAddress((const GLubyte *)"glXSwapIntervalEXT"));
    }
    else if (extString.find("GLX_MESA_swap_control") != std::string::npos) {
        glXSwapIntervalMESA_ =
            reinterpret_cast<PFNGLXSWAPINTERVALMESAPROC>(
                glXGetProcAddress((const GLubyte *)"glXSwapIntervalMESA"));
    }


    if (!glXSwapIntervalEXT_ && !glXSwapIntervalMESA_) {
        Log::info("** GLX does not support GLX_EXT_swap_control or GLX_MESA_swap_control!\n");
    }

    /* Turn off VSync */
    if ((!glXSwapIntervalEXT_ || glXSwapIntervalEXT_(display, window, 0)) &&
        (!glXSwapIntervalMESA_ || glXSwapIntervalMESA_(0)))
    {
        Log::info("** Failed to set swap interval. Results may be bounded above by refresh rate.\n");
    }
}

#endif
