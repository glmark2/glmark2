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
 */
#include "canvas-android.h"
#include "log.h"
#include "options.h"
#include <EGL/egl.h>

#include <fstream>
#include <sstream>

bool
CanvasAndroid::init()
{
    resize(mWidth, mHeight);

    if (!eglSwapInterval(eglGetCurrentDisplay(), 0))
        Log::info("** Failed to set swap interval. Results may be bounded above by refresh rate.\n");

    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glClearDepthf(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    clear();

    return true;
}

void
CanvasAndroid::visible(bool visible)
{
    (void)visible;
}

void
CanvasAndroid::clear()
{
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
    char *pixels = new char[mWidth * mHeight * 4];

    for (int i = 0; i < mHeight; i++) {
        glReadPixels(0, i, mWidth, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                     &pixels[(mHeight - i - 1) * mWidth * 4]);
    }

    std::ofstream output (filename.c_str(), std::ios::out | std::ios::binary);
    output.write(pixels, 4 * mWidth * mHeight);

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
    mWidth = width;
    mHeight = height;

    glViewport(0, 0, mWidth, mHeight);
    mProjection = LibMatrix::Mat4::perspective(60.0, mWidth / (float)mHeight,
                                               1.0, 1024.0);
}

