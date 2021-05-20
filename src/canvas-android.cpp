/*
 * Copyright © 2011 Linaro Limited
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
#include "canvas-android.h"
#include "log.h"
#include "options.h"
#include "gl-headers.h"

#include <fstream>
#include <sstream>

/******************
 * Public methods *
 ******************/

bool
CanvasAndroid::init()
{
    EGLint attribs[] = {
        EGL_CONFIG_ID, 0,
        EGL_NONE
    };

    /* Init the GLAD entrypoints */
    if (!egl_lib_.open_from_alternatives({"libEGL.so", "libEGL.so.1" })) {
        Log::error("Error loading EGL library\n");
        return false;
    }

    if (gladLoadEGLUserPtr(EGL_NO_DISPLAY, load_proc, &egl_lib_) == 0) {
        Log::error("Loading EGL entry points failed\n");
        return false;
    }

    /* Get the current EGL config */
    EGLDisplay egl_display(eglGetCurrentDisplay());

    /* Reinitialize GLAD with a known display */
    if (gladLoadEGLUserPtr(egl_display, load_proc, &egl_lib_) == 0) {
        Log::error("Loading EGL entry points with display failed\n");
        return false;
    }

    EGLContext egl_context(eglGetCurrentContext());
    EGLConfig egl_config(0);
    EGLint num_configs;

    eglQueryContext(egl_display, egl_context, EGL_CONFIG_ID, &(attribs[1]));

    eglChooseConfig(egl_display, attribs, &egl_config, 1, &num_configs);

    /* Before calling GLES functions, init GLAD GLES */
    if (!gles_lib_.open("libGLESv2.so")) {
        Log::error("Error loading GLES library\n");
        return false;
    }

    if (!gladLoadGLES2UserPtr(load_proc, &gles_lib_)) {
        Log::error("Loading GLESv2 entry points failed.");
        return false;
    }

    resize(width_, height_);

    if (!eglSwapInterval(egl_display, 0))
        Log::info("** Failed to set swap interval. Results may be bounded above by refresh rate.\n");

    init_gl_extensions();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    clear();

    if (Options::show_debug) {
        int buf, red, green, blue, alpha, depth, id, native_id;
        eglGetConfigAttrib(egl_display, egl_config, EGL_CONFIG_ID, &id);
        eglGetConfigAttrib(egl_display, egl_config, EGL_NATIVE_VISUAL_ID, &native_id);
        eglGetConfigAttrib(egl_display, egl_config, EGL_BUFFER_SIZE, &buf);
        eglGetConfigAttrib(egl_display, egl_config, EGL_RED_SIZE, &red);
        eglGetConfigAttrib(egl_display, egl_config, EGL_GREEN_SIZE, &green);
        eglGetConfigAttrib(egl_display, egl_config, EGL_BLUE_SIZE, &blue);
        eglGetConfigAttrib(egl_display, egl_config, EGL_ALPHA_SIZE, &alpha);
        eglGetConfigAttrib(egl_display, egl_config, EGL_DEPTH_SIZE, &depth);
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

void
CanvasAndroid::visible(bool visible)
{
    static_cast<void>(visible);
}

void
CanvasAndroid::clear()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void
CanvasAndroid::update()
{
}

void
CanvasAndroid::print_info()
{
    std::stringstream ss;

    ss << "    OpenGL Information" << std::endl;
    ss << "    GL_VENDOR:     " << glGetString(GL_VENDOR) << std::endl;
    ss << "    GL_RENDERER:   " << glGetString(GL_RENDERER) << std::endl;
    ss << "    GL_VERSION:    " << glGetString(GL_VERSION) << std::endl;

    Log::info("%s", ss.str().c_str());
}

Canvas::Pixel
CanvasAndroid::read_pixel(int x, int y)
{
    uint8_t pixel[4];

    glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

    return Canvas::Pixel(pixel[0], pixel[1], pixel[2], pixel[3]);
}

void
CanvasAndroid::write_to_file(std::string &filename)
{
    char *pixels = new char[width_ * height_ * 4];

    for (int i = 0; i < height_; i++) {
        glReadPixels(0, i, width_, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                     &pixels[(height_ - i - 1) * width_ * 4]);
    }

    std::ofstream output (filename.c_str(), std::ios::out | std::ios::binary);
    output.write(pixels, 4 * width_ * height_);

    delete [] pixels;
}

bool
CanvasAndroid::should_quit()
{
    return false;
}

void
CanvasAndroid::resize(int width, int height)
{
    width_ = width;
    height_ = height;

    glViewport(0, 0, width_, height_);
    projection_ = LibMatrix::Mat4::perspective(60.0, width_ / static_cast<float>(height_),
                                               1.0, 1024.0);
}

/*******************
 * Private methods *
 *******************/

GLADapiproc
CanvasAndroid::load_proc(void *userdata, const char *name)
{
    if (eglGetProcAddress) {
        GLADapiproc sym = reinterpret_cast<GLADapiproc>(eglGetProcAddress(name));
        if (sym) {
            return sym;
        }
    }

    SharedLibrary* lib = reinterpret_cast<SharedLibrary*>(userdata);
    return reinterpret_cast<GLADapiproc>(lib->load(name));
}

void
CanvasAndroid::init_gl_extensions()
{
    GLExtensions::MapBuffer = glMapBufferOES;
    GLExtensions::UnmapBuffer = glUnmapBufferOES;

    GLExtensions::GenFramebuffers = glGenFramebuffers;
    GLExtensions::DeleteFramebuffers = glDeleteFramebuffers;
    GLExtensions::BindFramebuffer = glBindFramebuffer;
    GLExtensions::FramebufferTexture2D = glFramebufferTexture2D;
    GLExtensions::FramebufferRenderbuffer = glFramebufferRenderbuffer;
    GLExtensions::CheckFramebufferStatus = glCheckFramebufferStatus;

    GLExtensions::GenRenderbuffers = glGenRenderbuffers;
    GLExtensions::DeleteRenderbuffers = glDeleteRenderbuffers;
    GLExtensions::BindRenderbuffer = glBindRenderbuffer;
    GLExtensions::RenderbufferStorage = glRenderbufferStorage;

    GLExtensions::GenerateMipmap = glGenerateMipmap;
}
