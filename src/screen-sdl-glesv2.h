#ifndef _SCREEN_SDL_GLESv2_H
#define _SCREEN_SDL_GLESv2_H

#include "screen-sdl.h"

class ScreenSDLGLESv2 : public ScreenSDL
{
public:
    ScreenSDLGLESv2(int pWidth, int pHeight, int pBpp, int pFullscreen, int pFlags = 0);
    ~ScreenSDLGLESv2();

    virtual void clear();
    virtual void update();
    virtual void print_info();
};

#endif
