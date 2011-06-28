/*
 * Copyright © 2008 Ben Smith
 * Copyright © 2010-2011 Linaro Limited
 *
 * This file is part of the glmark2 OpenGL (ES) 2.0 benchmark.
 *
 * glmark2 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * glmark2 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * glmark2.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *  Ben Smith (original glmark benchmark)
 *  Alexandros Frantzis (glmark2)
 */
#include "texture.h"
#include "log.h"

#include <png.h>

class ImageData {
public:
    ImageData() : pixels(0), width(0), height(0), bpp(0) {}
    ~ImageData() { delete [] pixels; }
    bool load_png(const std::string &filename);
    void resize(int w, int h, int b)
    {
        width = w;
        height = h;
        bpp = b;
        delete [] pixels;
        pixels = new unsigned char[bpp * w * h];
    }

    unsigned char *pixels;
    int width;
    int height;
    int bpp;
};

bool
ImageData::load_png(const std::string &filename)
{
    bool ret = false;
    png_structp png_ptr = 0;
    png_infop info_ptr = 0;
    png_bytepp row_pointers = 0;
    static const int png_transforms = PNG_TRANSFORM_STRIP_16 |
                                      PNG_TRANSFORM_GRAY_TO_RGB |
                                      PNG_TRANSFORM_PACKING |
                                      PNG_TRANSFORM_EXPAND;

    Log::debug("Reading PNG file %s\n", filename.c_str());

    FILE *fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        Log::error("Cannot open file %s!\n", filename.c_str());
        goto out;
    }

    /* Set up all the libpng structs we need */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if (!png_ptr) {
        Log::error("Couldn't create libpng read struct\n");
        goto out;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        Log::error("Couldn't create libpng info struct\n");
        goto out;
    }

    /* Set up libpng error handling */
    if (setjmp(png_jmpbuf(png_ptr))) {
        Log::error("libpng error while reading file %s\n", filename.c_str());
        goto out;
    }

    /* Read the image information and data */
    png_init_io(png_ptr, fp);

    png_read_png(png_ptr, info_ptr, png_transforms, 0);

    row_pointers = png_get_rows(png_ptr, info_ptr);

    resize(png_get_image_width(png_ptr, info_ptr),
           png_get_image_height(png_ptr, info_ptr),
           png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB ? 3 : 4);


    Log::debug("    Height: %d Width: %d Bpp: %d\n", width, height, bpp);

    /*
     * Copy the image data to a contiguous memory area suitable for texture
     * upload.
     */
    for (int i = 0; i < height; i++) {
        memcpy(&pixels[bpp * width * i],
               row_pointers[height - i - 1],
               width * bpp);
    }

    ret = true;

out:
    if (fp)
       fclose(fp);

    if (png_ptr)
        png_destroy_read_struct(&png_ptr,
                                info_ptr != 0 ? &info_ptr : 0,
                                0);

    return ret;
}

static void
setup_texture(GLuint *tex, ImageData &image, GLint min_filter, GLint mag_filter)
{
    GLenum format = image.bpp == 3 ? GL_RGB : GL_RGBA;

    glGenTextures(1, tex);
    glBindTexture(GL_TEXTURE_2D, *tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexImage2D(GL_TEXTURE_2D, 0, format, image.width, image.height, 0,
                 format, GL_UNSIGNED_BYTE, image.pixels);

    if ((min_filter != GL_NEAREST && min_filter != GL_LINEAR) ||
        (mag_filter != GL_NEAREST && mag_filter != GL_LINEAR))
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
}

int
Texture::load(const std::string &filename, GLuint *pTexture, ...)
{
    ImageData image;

    if (!image.load_png(filename))
        return 0;

    va_list ap;
    va_start(ap, pTexture);
    GLint arg;

    while ((arg = va_arg(ap, GLint)) != 0) {
        GLint arg2 = va_arg(ap, GLint);
        setup_texture(pTexture, image, arg, arg2);
        pTexture++;
    }

    va_end(ap);

    return 1;
}
