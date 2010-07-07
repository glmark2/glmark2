#ifndef _SCREEN_H
#define _SCREEN_H

#include "oglsdl.h"

#include <stdio.h>

class Screen
{
public:
    const SDL_VideoInfo *mInfo;
    
    int mWidth;
    int mHeight;
    int mBpp;
    int mFlags;
    int mFullScreen;
//    SDL_Surface *mScreen;

    Screen();
    ~Screen();
    Screen(int pWidth, int pHeight, int pBpp, int pFlags);
    int init();
    void clear();
    void update();
    void print_info();
};

#endif
