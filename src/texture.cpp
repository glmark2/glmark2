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
#include "util.h"

#include <cstdarg>
#include <png.h>
#include <memory>

class PNGState
{
public:
    PNGState() :
        png_(0),
        info_(0),
        rows_(0) {}
    ~PNGState()
    {
        if (png_)
        {
            png_destroy_read_struct(&png_, &info_, 0);
        }
    }
    bool gotData(const std::string& filename)
    {
        static const int png_transforms = PNG_TRANSFORM_STRIP_16 |
                                          PNG_TRANSFORM_GRAY_TO_RGB |
                                          PNG_TRANSFORM_PACKING |
                                          PNG_TRANSFORM_EXPAND;

        Log::debug("Reading PNG file %s\n", filename.c_str());

        const std::auto_ptr<std::istream> is_ptr(Util::get_resource(filename));
        if (!(*is_ptr)) {
            Log::error("Cannot open file %s!\n", filename.c_str());
            return false;
        }

        /* Set up all the libpng structs we need */
        png_ = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
        if (!png_) {
            Log::error("Couldn't create libpng read struct\n");
            return false;
        }

        info_ = png_create_info_struct(png_);
        if (!info_) {
            Log::error("Couldn't create libpng info struct\n");
            return false;
        }

        /* Set up libpng error handling */
        if (setjmp(png_jmpbuf(png_))) {
            Log::error("libpng error while reading file %s\n", filename.c_str());
            return false;
        }

        /* Read the image information and data */
        png_set_read_fn(png_, reinterpret_cast<voidp>(is_ptr.get()), png_read_fn);

        png_read_png(png_, info_, png_transforms, 0);

        rows_ = png_get_rows(png_, info_);

        return true;
    }
    unsigned int width() const { return png_get_image_width(png_, info_); }
    unsigned int height() const { return png_get_image_height(png_, info_); }
    unsigned int pixelBytes() const
    {
        if (png_get_color_type(png_, info_) == PNG_COLOR_TYPE_RGB)
        {
            return 3;
        }
        return 4;
    }
    const unsigned char* row(unsigned int idx) const { return rows_[idx]; }
private:
    static void png_read_fn(png_structp png_ptr, png_bytep data, png_size_t length)
    {
        std::istream *is = reinterpret_cast<std::istream*>(png_get_io_ptr(png_ptr));
        is->read(reinterpret_cast<char *>(data), length);
    }
    png_structp png_;
    png_infop info_;
    png_bytepp rows_;
};

class ImageData {
    void resize(unsigned int w, unsigned int h, unsigned int b)
    {
        width = w;
        height = h;
        bpp = b;
        delete [] pixels;
        pixels = new unsigned char[bpp * width * height];
    }

public:
    ImageData() : pixels(0), width(0), height(0), bpp(0) {}
    ~ImageData() { delete [] pixels; }
    bool load_png(const std::string &filename);

    unsigned char *pixels;
    unsigned int width;
    unsigned int height;
    unsigned int bpp;
};

bool
ImageData::load_png(const std::string &filename)
{
    PNGState png;
    bool ret = png.gotData(filename);
    if (!ret)
    {
        return ret;
    }

    resize(png.width(), png.height(), png.pixelBytes());

    Log::debug("    Height: %d Width: %d Bpp: %d\n", width, height, bpp);

    /*
     * Copy the image data to a contiguous memory area suitable for texture
     * upload.
     */
    for (unsigned int i = 0; i < height; i++) {
        memcpy(&pixels[bpp * width * i],
               png.row(height - i - 1),
               width * bpp);
    }

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
