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

XVisualInfo *
CanvasX11EGL::get_xvisualinfo()
{
    XVisualInfo vis_tmpl;
    XVisualInfo *vis_info;
    int num_visuals;
    EGLint vid;

    if (!ensure_egl_config())
        return 0;

    if (!eglGetConfigAttrib(egl_display_, egl_config_,
                            EGL_NATIVE_VISUAL_ID, &vid))
    {
        Log::error("eglGetConfigAttrib() failed with error: %d\n",
                   eglGetError());
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
    if (!ensure_egl_surface())
        return false;

    if (!ensure_egl_context())
        return false;

    if (egl_context_ == eglGetCurrentContext())
        return true;

    if (!eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_context_)) {
        Log::error("Error: eglMakeCurrent failed with error %d\n", eglGetError());
        return false;
    }

    if (!eglSwapInterval(egl_display_, 0))
        Log::info("** Failed to set swap interval. Results may be bounded above by refresh rate.\n");

    init_gl_extensions();

    return true;
}

void
CanvasX11EGL::get_glvisualconfig(GLVisualConfig &visual_config)
{
    if (!ensure_egl_config())
        return;

    get_glvisualconfig_egl(egl_config_, visual_config);
}

/*******************
 * Private methods *
 *******************/

bool
CanvasX11EGL::ensure_egl_display()
{
    if (egl_display_)
        return true;

    egl_display_ = eglGetDisplay((EGLNativeDisplayType) xdpy_);
    if (!egl_display_) {
        Log::error("eglGetDisplay() failed with error: %d\n",
                   eglGetError());
        return false;
    }
    if (!eglInitialize(egl_display_, NULL, NULL)) {
        Log::error("eglInitialize() failed with error: %d\n",
                   eglGetError());
        return false;
        egl_display_ = 0;
    }

    return true;
}

bool
CanvasX11EGL::ensure_egl_config()
{
    const EGLint attribs[] = {
        EGL_RED_SIZE, visual_config_.red,
        EGL_GREEN_SIZE, visual_config_.green,
        EGL_BLUE_SIZE, visual_config_.blue,
        EGL_ALPHA_SIZE, visual_config_.alpha,
        EGL_DEPTH_SIZE, visual_config_.depth,
        EGL_BUFFER_SIZE, visual_config_.buffer,
#ifdef USE_GLESv2
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
#elif USE_GL
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
#endif
        EGL_NONE
    };
    EGLint num_configs;
    EGLint vid;

    if (egl_config_)
        return true;

    if (!ensure_egl_display())
        return false;

    /* Find out how many configs match the attributes */
    if (!eglChooseConfig(egl_display_, attribs, 0, 0, &num_configs)) {
        Log::error("eglChooseConfig() (explore) failed with error: %d\n",
                   eglGetError());
        return false;
    }

    if (num_configs == 0) {
        Log::error("eglChooseConfig() didn't return any configs\n");
        return false;
    }

    /* Get all the matching configs */
    std::vector<EGLConfig> configs(num_configs);

    if (!eglChooseConfig(egl_display_, attribs, &(configs[0]),
                         num_configs, &num_configs))
    {
        Log::error("eglChooseConfig() failed with error: %d\n",
                     eglGetError());
        return false;
    }

    /* Select the best matching config */
    egl_config_ = select_best_config(configs);

    if (!eglGetConfigAttrib(egl_display_, egl_config_,
                            EGL_NATIVE_VISUAL_ID, &vid))
    {
        Log::error("eglGetConfigAttrib() failed with error: %d\n",
                   eglGetError());
        return false;
    }

    if (Options::show_debug) {
        int buf, red, green, blue, alpha, depth, id, native_id;
        eglGetConfigAttrib(egl_display_, egl_config_, EGL_CONFIG_ID, &id);
        eglGetConfigAttrib(egl_display_, egl_config_, EGL_NATIVE_VISUAL_ID, &native_id);
        eglGetConfigAttrib(egl_display_, egl_config_, EGL_BUFFER_SIZE, &buf);
        eglGetConfigAttrib(egl_display_, egl_config_, EGL_RED_SIZE, &red);
        eglGetConfigAttrib(egl_display_, egl_config_, EGL_GREEN_SIZE, &green);
        eglGetConfigAttrib(egl_display_, egl_config_, EGL_BLUE_SIZE, &blue);
        eglGetConfigAttrib(egl_display_, egl_config_, EGL_ALPHA_SIZE, &alpha);
        eglGetConfigAttrib(egl_display_, egl_config_, EGL_DEPTH_SIZE, &depth);
        Log::debug("EGL chosen config ID: 0x%x Native Visual ID: 0x%x\n"
                   "  Buffer: %d bits\n"
                   "     Red: %d bits\n"
                   "   Green: %d bits\n"
                   "    Blue: %d bits\n"
                   "   Alpha: %d bits\n"
                   "   Depth: %d bits\n",
                   id, native_id,
                   buf, red, green, blue, alpha, depth);
    }

    return true;
}

bool
CanvasX11EGL::reset_context()
{
    if (!ensure_egl_display())
        return false;

    if (!egl_context_)
        return true;

    if (eglDestroyContext(egl_display_, egl_context_) == EGL_FALSE) {
        Log::debug("eglDestroyContext() failed with error: 0x%x\n",
                   eglGetError());
    }

    egl_context_ = 0;
    return true;
}

bool
CanvasX11EGL::ensure_egl_context()
{
    if (egl_context_)
        return true;

    if (!ensure_egl_display())
        return false;

    if (!ensure_egl_config())
        return false;

    static const EGLint ctx_attribs[] = {
#ifdef USE_GLESv2
        EGL_CONTEXT_CLIENT_VERSION, 2,
#endif
        EGL_NONE
    };

    egl_context_ = eglCreateContext(egl_display_, egl_config_,
                                    EGL_NO_CONTEXT, ctx_attribs);
    if (!egl_context_) {
        Log::error("eglCreateContext() failed with error: 0x%x\n",
                     eglGetError());
        return false;
    }

    return true;
}

bool
CanvasX11EGL::ensure_egl_surface()
{
    if (egl_surface_)
        return true;

    if (!ensure_egl_display())
        return false;

#ifdef USE_GLESv2
    eglBindAPI(EGL_OPENGL_ES_API);
#elif USE_GL
    eglBindAPI(EGL_OPENGL_API);
#endif

    egl_surface_ = eglCreateWindowSurface(egl_display_, egl_config_,
                                          (EGLNativeWindowType) xwin_,
                                          NULL);
    if (!egl_surface_) {
        Log::error("eglCreateWindowSurface failed with error: %d\n",
                     eglGetError());
        return false;
    }

    return true;
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

void
CanvasX11EGL::get_glvisualconfig_egl(EGLConfig config, GLVisualConfig &visual_config)
{
    eglGetConfigAttrib(egl_display_, config, EGL_BUFFER_SIZE, &visual_config.buffer);
    eglGetConfigAttrib(egl_display_, config, EGL_RED_SIZE, &visual_config.red);
    eglGetConfigAttrib(egl_display_, config, EGL_GREEN_SIZE, &visual_config.green);
    eglGetConfigAttrib(egl_display_, config, EGL_BLUE_SIZE, &visual_config.blue);
    eglGetConfigAttrib(egl_display_, config, EGL_ALPHA_SIZE, &visual_config.alpha);
    eglGetConfigAttrib(egl_display_, config, EGL_DEPTH_SIZE, &visual_config.depth);
}

EGLConfig
CanvasX11EGL::select_best_config(std::vector<EGLConfig> configs)
{
    int best_score(INT_MIN);
    EGLConfig best_config(0);

    /*
     * Go through all the configs and choose the one with the best score,
     * i.e., the one better matching the requested config.
     */
    for (std::vector<EGLConfig>::const_iterator iter = configs.begin();
         iter != configs.end();
         iter++)
    {
        const EGLConfig config(*iter);
        GLVisualConfig vc;
        int score;

        get_glvisualconfig_egl(config, vc);

        score = vc.match_score(visual_config_);

        if (score > best_score) {
            best_score = score;
            best_config = config;
        }
    }

    return best_config;
}
