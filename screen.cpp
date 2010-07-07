#include "screen.h"

Screen::Screen()
{
    mWidth = 800;
    mHeight = 600;
    mBpp = 24;
    mFlags = SDL_OPENGL;
    mFullScreen = false;

    mInfo = SDL_GetVideoInfo();
}

Screen::~Screen()
{
    SDL_Quit();
}

Screen::Screen(int pWidth, int pHeight, int pBpp, int pFlags)
{
    mWidth = pWidth;
    mHeight = pHeight;
    mBpp = pBpp;
    mFlags = SDL_OPENGL | pFlags;

    mInfo = SDL_GetVideoInfo();
}

int Screen::init()
{
#ifdef _DEBUG
    printf("Initializing Screen...           ");
#endif
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "[ Fail ] - Video initialization failed: %s\n", SDL_GetError());
        return 0;
    }

    if(mFullScreen)
    mFlags = SDL_OPENGL | SDL_FULLSCREEN;
    
    mInfo = SDL_GetVideoInfo();

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if(SDL_SetVideoMode(mWidth, mHeight, mBpp, mFlags) == 0)
    {
        fprintf(stderr, "[ Fail ] - Video mode set failed: %s\n", SDL_GetError());
        return 0;
    }
    
    SDL_WM_SetCaption("GLMark 08", NULL);
    
    GLenum err = glewInit();
    if(GLEW_OK != err)
    {
        fprintf(stderr, "[ Fail ] - Error: %s\n", glewGetErrorString(err));
        return 0;
    }

    if(!GLEW_ARB_vertex_buffer_object)
    {
        fprintf(stderr, "[ Fail ] - VBO objects are not supported\n");
        return 0;
    }

    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glViewport(0, 0, mWidth, mHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, mWidth / (float)mHeight, 1.0, 1024.0);
    glMatrixMode(GL_MODELVIEW);
    
    clear();

#ifdef _DEBUG
    printf("[ Success ]\n");
#endif
    print_info();
    return 1;
}

void Screen::clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
}

void Screen::update()
{
    SDL_GL_SwapBuffers();
}

void Screen::print_info()
{
    printf("    OpenGL Information\n");
    printf("    GL_VENDOR:     %s\n", glGetString(GL_VENDOR));
    printf("    GL_RENDERER:   %s\n", glGetString(GL_RENDERER));
    printf("    GL_VERSION:    %s\n", glGetString(GL_VERSION));
//    printf("GL_EXTENSIONS:	%s\n", glGetString(GL_EXTENSIONS));
}

