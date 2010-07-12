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

    SDL_WM_SetCaption("GLMark 08", NULL);

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

