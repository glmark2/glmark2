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
#include "canvas-sdl-gl.h"
#include "options.h"
#include "glx-disable-vsync.h"
#include <fstream>

CanvasSDLGL::CanvasSDLGL(int pWidth, int pHeight, int pBpp, int pFullScreen, int pFlags)
    : CanvasSDL(pWidth, pHeight, pBpp, pFullScreen, pFlags | SDL_OPENGL)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glViewport(0, 0, mWidth, mHeight);

    /*
     * There is a bug in SDL that prevents us from setting the swap
     * interval using the SDL_GL_SWAP_CONTROL attribute. We take care
     * of this manually for now.
     */
    glx_disable_vsync();
    
    clear();
}

CanvasSDLGL::~CanvasSDLGL()
{
}


void CanvasSDLGL::clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void CanvasSDLGL::update()
{
    if (Options::swap_buffers)
        SDL_GL_SwapBuffers();
    else
        glFinish();
}

void CanvasSDLGL::print_info()
{
    printf("    OpenGL Information\n");
    printf("    GL_VENDOR:     %s\n", glGetString(GL_VENDOR));
    printf("    GL_RENDERER:   %s\n", glGetString(GL_RENDERER));
    printf("    GL_VERSION:    %s\n", glGetString(GL_VERSION));
}

Canvas::Pixel
CanvasSDLGL::read_pixel(int x, int y)
{
    Uint8 pixel[4];

    glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

    return Canvas::Pixel(pixel[0], pixel[1], pixel[2], pixel[3]);
}

void
CanvasSDLGL::write_to_file(std::string &filename)
{
    char *pixels = new char[mWidth * mHeight * 4];

    for (int i = 0; i < mHeight; i++) {
        glReadPixels(0, i, mWidth, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                     &pixels[(mHeight - i - 1) * mWidth * 4]);
    }

    std::ofstream output(filename.c_str(), std::ios::out | std::ios::binary);
    output.write(pixels, 4 * mWidth * mHeight);

    delete [] pixels;
}
