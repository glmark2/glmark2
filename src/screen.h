#ifndef _SCREEN_H
#define _SCREEN_H

#include "oglsdl.h"
#include "matrix.h"

#include <stdio.h>

class Screen
{
public:
    int mWidth;
    int mHeight;
    int mBpp;
    int mFullScreen;
    Matrix4f mProjection;
    int mInitSuccess;

    virtual void clear() = 0;
    virtual void update() = 0;
    virtual void print_info() = 0;
};

#endif
