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
