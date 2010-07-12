#include "screen-sdl-glesv2.h"
#include "sdlgles/SDL_gles.h"

ScreenSDLGLESv2::ScreenSDLGLESv2(int pWidth, int pHeight, int pBpp, int pFullScreen, int pFlags)
    : ScreenSDL(pWidth, pHeight, pBpp, pFullScreen, pFlags | SDL_OPENGL)
{
    mInitSuccess = 0;

    if (SDL_GLES_Init(SDL_GLES_VERSION_2_0) < 0) {
	fprintf(stderr, "[ Fail ] - GLES initialization failed: %s\n", SDL_GetError());
    }

    SDL_GLES_Context *context;
    context = SDL_GLES_CreateContext();
    if (context == NULL) {
	fprintf(stderr, "[ Fail ] - GLES create context: %s\n", SDL_GetError());
	return;
    }

    if (SDL_GLES_MakeCurrent(context) != 0) {
	fprintf(stderr, "[ Fail ] - GLES make context current: %s\n", SDL_GetError());
	return;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glViewport(0, 0, mWidth, mHeight);

    clear();

    mInitSuccess = 1;
}

ScreenSDLGLESv2::~ScreenSDLGLESv2()
{
}


void ScreenSDLGLESv2::clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ScreenSDLGLESv2::update()
{
	SDL_GLES_SwapBuffers();
}

void ScreenSDLGLESv2::print_info()
{
    printf("    OpenGL Information\n");
    printf("    GL_VENDOR:     %s\n", glGetString(GL_VENDOR));
    printf("    GL_RENDERER:   %s\n", glGetString(GL_RENDERER));
    printf("    GL_VERSION:    %s\n", glGetString(GL_VERSION));
}

