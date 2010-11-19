/*
 * Copyright © 2008 Ben Smith
 * Copyright © 2010 Linaro
 *
 * This file is part of the glmark2 OpenGL (ES) 2.0 benchmark.
 *
 * glmark2 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 3, as published by the Free
 * Software Foundation.
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
#include "screen-sdl-glesv2.h"
#include "sdlgles/SDL_gles.h"

ScreenSDLGLESv2::ScreenSDLGLESv2(int pWidth, int pHeight, int pBpp, int pFullScreen, int pFlags)
    : ScreenSDL(pWidth, pHeight, pBpp, pFullScreen, pFlags | SDL_OPENGL)
{
    mInitSuccess = 0;

    if (SDL_GLES_Init(SDL_GLES_VERSION_2_0) < 0) {
	fprintf(stderr, "[ Fail ] - GLES initialization failed: %s\n", SDL_GetError());
    }

    SDL_GLES_Context *context;
    context = SDL_GLES_CreateContext();
    if (context == NULL) {
	fprintf(stderr, "[ Fail ] - GLES create context: %s\n", SDL_GetError());
	return;
    }

    if (SDL_GLES_MakeCurrent(context) != 0) {
	fprintf(stderr, "[ Fail ] - GLES make context current: %s\n", SDL_GetError());
	return;
    }

#ifdef _DEBUG
    {
    int buf, red, green, blue, alpha, depth;
    SDL_GLES_GetAttribute(SDL_GLES_BUFFER_SIZE, &buf);
    SDL_GLES_GetAttribute(SDL_GLES_RED_SIZE, &red);
    SDL_GLES_GetAttribute(SDL_GLES_GREEN_SIZE, &green);
    SDL_GLES_GetAttribute(SDL_GLES_BLUE_SIZE, &blue);
    SDL_GLES_GetAttribute(SDL_GLES_ALPHA_SIZE, &alpha);
    SDL_GLES_GetAttribute(SDL_GLES_DEPTH_SIZE, &depth);
    printf("EGL chosen config:\n"
           "  Buffer: %d bits\n"
           "     Red: %d bits\n"
           "   Green: %d bits\n"
           "    Blue: %d bits\n"
           "   Alpha: %d bits\n"
           "   Depth: %d bits\n",
           buf, red, green, blue, alpha, depth);
    }
#endif

    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glViewport(0, 0, mWidth, mHeight);

    clear();

    mInitSuccess = 1;
}

ScreenSDLGLESv2::~ScreenSDLGLESv2()
{
}


void ScreenSDLGLESv2::clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ScreenSDLGLESv2::update()
{
	SDL_GLES_SwapBuffers();
}

void ScreenSDLGLESv2::print_info()
{
    printf("    OpenGL Information\n");
    printf("    GL_VENDOR:     %s\n", glGetString(GL_VENDOR));
    printf("    GL_RENDERER:   %s\n", glGetString(GL_RENDERER));
    printf("    GL_VERSION:    %s\n", glGetString(GL_VERSION));
}

