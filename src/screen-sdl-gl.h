#ifndef _SCREEN_SDL_GL_H
#define _SCREEN_SDL_GL_H

#include "screen-sdl.h"

class ScreenSDLGL : public ScreenSDL
{
public:
    ScreenSDLGL(int pWidth, int pHeight, int pBpp, int pFullscreen, int pFlags = 0);
    ~ScreenSDLGL();

    virtual void clear();
    virtual void update();
    virtual void print_info();
};

#endif
