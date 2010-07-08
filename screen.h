#ifndef _SCREEN_H
#define _SCREEN_H

#include "oglsdl.h"
#include "matrix.h"

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
    Matrix4f mProjection;

    Screen();
    ~Screen();
    Screen(int pWidth, int pHeight, int pBpp, int pFlags);
    int init();
    void clear();
    void update();
    void print_info();
};

#endif
