#include "texture.h"

int load_texture(const char pFilename[], GLuint *pTexture)
{
    SDL_Surface *surface;
    GLenum texture_format = GL_RGB;
    GLint  nOfColors;

    if ((surface = SDL_LoadBMP(pFilename))) {
        if ((surface->w & (surface->w - 1)) != 0)
            printf("warning: image.bmp's width is not a power of 2\n");

        if ((surface->h & (surface->h - 1)) != 0)
            printf("warning: image.bmp's height is not a power of 2\n");

        nOfColors = surface->format->BytesPerPixel;
        if (nOfColors == 4) {
            if (surface->format->Rmask == 0x000000ff)
                texture_format = GL_RGBA;
            else
                texture_format = GL_BGRA;
        }
        else {
            if (nOfColors == 3) {
                if (surface->format->Rmask == 0x000000ff)
                    texture_format = GL_RGB;
                else
                    texture_format = GL_BGR;
            }
            else {
                printf("warning: the image is not truecolor..  this will probably break\n");
            }
        }

        glGenTextures(3, pTexture);

        // Create Nearest Filtered Texture
        glBindTexture(GL_TEXTURE_2D, pTexture[0]);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, nOfColors, surface->w, surface->h, 0,
                     texture_format, GL_UNSIGNED_BYTE, surface->pixels);

        // Create Linear Filtered Texture
        glBindTexture(GL_TEXTURE_2D, pTexture[1]);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, nOfColors, surface->w, surface->h, 0,
                     texture_format, GL_UNSIGNED_BYTE, surface->pixels);

        // Create trilinear filtered mipmapped texture
        glBindTexture(GL_TEXTURE_2D, pTexture[2]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, nOfColors, surface->w, surface->h, 0,
                     texture_format, GL_UNSIGNED_BYTE, surface->pixels);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        fprintf(stderr, "SDL could not load image.bmp: %s\n", SDL_GetError());
        return 0;
    }

    if (surface)
        SDL_FreeSurface(surface);

    return 1;
}
