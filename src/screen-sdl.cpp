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
#include "screen-sdl.h"

ScreenSDL::ScreenSDL(int pWidth, int pHeight, int pBpp, int pFullScreen, int pFlags)
{
    mWidth = pWidth;
    mHeight = pHeight;
    mFullScreen = pFullScreen;
    mBpp = pBpp;

    if (mFullScreen)
        pFlags |= SDL_FULLSCREEN;

#ifdef _DEBUG
    printf("Initializing Screen...           ");
#endif
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "[ Fail ] - Video initialization failed: %s\n", SDL_GetError());
        return;
    }

    mInfo = SDL_GetVideoInfo();

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if(SDL_SetVideoMode(mWidth, mHeight, mBpp, pFlags) == 0)
    {
        fprintf(stderr, "[ Fail ] - Video mode set failed: %s\n", SDL_GetError());
        return;
    }

    SDL_WM_SetCaption("glmark2 " GLMARK_VERSION, NULL);

    mProjection.perspective(60.0, mWidth / (float)mHeight, 1.0, 1024.0);

#ifdef _DEBUG
    mProjection.display("Projection");
#endif

    mInitSuccess = 1;
}

ScreenSDL::~ScreenSDL()
{
    SDL_Quit();
}


void ScreenSDL::clear()
{
}

void ScreenSDL::update()
{
}

void ScreenSDL::print_info()
{
}

