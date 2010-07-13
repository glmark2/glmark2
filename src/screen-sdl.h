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
#ifndef _SCREEN_SDL_H
#define _SCREEN_SDL_H

#include "screen.h"

class ScreenSDL : public Screen
{
public:
    ScreenSDL(int pWidth, int pHeight, int pBpp, int pFullscreen, int pFlags = 0);
    ~ScreenSDL();

    virtual void clear();
    virtual void update();
    virtual void print_info();

protected:
    const SDL_VideoInfo *mInfo;
};

#endif
