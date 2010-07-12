#include "screen-sdl-gl.h"

ScreenSDLGL::ScreenSDLGL(int pWidth, int pHeight, int pBpp, int pFullScreen, int pFlags)
    : ScreenSDL(pWidth, pHeight, pBpp, pFullScreen, pFlags | SDL_OPENGL)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glViewport(0, 0, mWidth, mHeight);

    clear();
}

ScreenSDLGL::~ScreenSDLGL()
{
}


void ScreenSDLGL::clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ScreenSDLGL::update()
{
    SDL_GL_SwapBuffers();
}

void ScreenSDLGL::print_info()
{
    printf("    OpenGL Information\n");
    printf("    GL_VENDOR:     %s\n", glGetString(GL_VENDOR));
    printf("    GL_RENDERER:   %s\n", glGetString(GL_RENDERER));
    printf("    GL_VERSION:    %s\n", glGetString(GL_VERSION));
}

