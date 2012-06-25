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
#include <cstdio>
#include <jpeglib.h>
#include <memory>
#include <vector>

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

struct JPEGErrorMgr
{
    struct jpeg_error_mgr pub;
    jmp_buf jmp_buffer;

    JPEGErrorMgr()
    {
        jpeg_std_error(&pub);
        pub.error_exit = error_exit;
    }

    static void error_exit(j_common_ptr cinfo)
    {
        JPEGErrorMgr *err =
            reinterpret_cast<JPEGErrorMgr *>(cinfo->err);

        char buffer[JMSG_LENGTH_MAX];

        /* Create the message */
        (*cinfo->err->format_message)(cinfo, buffer);
        std::string msg(std::string(buffer) + "\n");
        Log::error(msg.c_str());

        longjmp(err->jmp_buffer, 1);
    }
};

struct JPEGIStreamSourceMgr
{
    static const int BUFFER_SIZE = 4096;
    struct jpeg_source_mgr pub;
    std::istream *is;
    JOCTET buffer[BUFFER_SIZE];

    JPEGIStreamSourceMgr(const std::string& filename) : is(0)
    {
        is = Util::get_resource(filename);

        /* Fill in jpeg_source_mgr pub struct */
        pub.init_source = init_source;
        pub.fill_input_buffer = fill_input_buffer;
        pub.skip_input_data = skip_input_data;
        pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
        pub.term_source = term_source;
        pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
        pub.next_input_byte = NULL; /* until buffer loaded */
    }

    ~JPEGIStreamSourceMgr()
    {
        delete is;
    }

    bool error()
    {
        return !is || (is->fail() && !is->eof());
    }

    static void init_source(j_decompress_ptr cinfo)
    {
        static_cast<void>(cinfo);
    }

    static boolean fill_input_buffer(j_decompress_ptr cinfo)
    {
        JPEGIStreamSourceMgr *src =
            reinterpret_cast<JPEGIStreamSourceMgr *>(cinfo->src);

        src->is->read(reinterpret_cast<char *>(src->buffer), BUFFER_SIZE);

        src->pub.next_input_byte = src->buffer;
        src->pub.bytes_in_buffer = src->is->gcount();

        /* 
         * If the decoder needs more data, but we have no more bytes left to
         * read mark the end of input.
         */
        if (src->pub.bytes_in_buffer == 0) {
            src->pub.bytes_in_buffer = 2;
            src->buffer[0] = 0xFF;
            src->buffer[0] = JPEG_EOI;
        }

        return TRUE;
    }

    static void skip_input_data(j_decompress_ptr cinfo, long num_bytes)
    {
        JPEGIStreamSourceMgr *src =
            reinterpret_cast<JPEGIStreamSourceMgr *>(cinfo->src);

        if (num_bytes > 0) {
            while (num_bytes > (long) src->pub.bytes_in_buffer) {
                num_bytes -= (long) src->pub.bytes_in_buffer;
                (void) (*src->fill_input_buffer) (cinfo);
            }
            src->pub.next_input_byte += (size_t) num_bytes;
            src->pub.bytes_in_buffer -= (size_t) num_bytes;
        }
    }

    static void term_source(j_decompress_ptr cinfo)
    {
        static_cast<void>(cinfo);
    }
};

class JPEGReader
{
public:
    JPEGReader(const std::string& filename) :
        source_mgr_(filename), jpeg_error_(false)
    {
        init(filename);
    }

    ~JPEGReader()
    {
        finish();
    }

    bool error()
    {
        return jpeg_error_ || source_mgr_.error();
    }

    bool nextRow(unsigned char *dst)
    {
        bool ret = true;
        unsigned char *buffer[1];
        buffer[0] = dst;

        /* Set up error handling */
        if (setjmp(error_mgr_.jmp_buffer)) {
            jpeg_error_ = true;
            return false;
        }

        /* While there are lines left, read next line */
        if (cinfo_.output_scanline < cinfo_.output_height) {
            jpeg_read_scanlines(&cinfo_, buffer, 1);
        }
        else {
            jpeg_finish_decompress(&cinfo_);
            ret = false;
        }

        return ret;
    }

    unsigned int width() const { return cinfo_.output_width; }
    unsigned int height() const { return cinfo_.output_height; }
    unsigned int pixelBytes() const { return cinfo_.output_components; }

private:
    void init(const std::string& filename)
    {
        Log::debug("Reading JPEG file %s\n", filename.c_str());

        /* Initialize error manager */
        cinfo_.err = reinterpret_cast<jpeg_error_mgr *>(&error_mgr_);

        if (setjmp(error_mgr_.jmp_buffer)) {
            jpeg_error_ = true;
            return;
        }

        jpeg_create_decompress(&cinfo_);
        cinfo_.src = reinterpret_cast<jpeg_source_mgr*>(&source_mgr_);

        /* Read header */
        jpeg_read_header(&cinfo_, TRUE);

        jpeg_start_decompress(&cinfo_);
    }

    void finish()
    {
        jpeg_destroy_decompress(&cinfo_);
    }

    struct jpeg_decompress_struct cinfo_;
    JPEGErrorMgr error_mgr_;
    JPEGIStreamSourceMgr source_mgr_;
    bool jpeg_error_;
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
    bool load_jpeg(const std::string &filename);

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

bool
ImageData::load_jpeg(const std::string &filename)
{
    JPEGReader reader(filename);
    if (reader.error())
        return false;

    resize(reader.width(), reader.height(), reader.pixelBytes());

    Log::debug("    Height: %d Width: %d Bpp: %d\n", width, height, bpp);

    /* 
     * Copy the row data to the image buffer in reverse Y order, suitable
     * for texture upload.
     */
    unsigned char *ptr = &pixels[bpp * width * (height - 1)];

    while (reader.nextRow(ptr))
        ptr -= bpp * width;

    return !reader.error();
}

static void
setup_texture(GLuint *tex, ImageData &image, GLint min_filter, GLint mag_filter)
{
    GLenum format = image.bpp == 3 ? GL_RGB : GL_RGBA;

    glGenTextures(1, tex);
    glBindTexture(GL_TEXTURE_2D, *tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, format, image.width, image.height, 0,
                 format, GL_UNSIGNED_BYTE, image.pixels);

    if ((min_filter != GL_NEAREST && min_filter != GL_LINEAR) ||
        (mag_filter != GL_NEAREST && mag_filter != GL_LINEAR))
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
}

namespace TexturePrivate
{
TextureMap textureMap;
}

bool
Texture::load(const std::string &textureName, GLuint *pTexture, ...)
{
    // Make sure the named texture is in the map.
    TextureMap::const_iterator textureIt = TexturePrivate::textureMap.find(textureName);
    if (textureIt == TexturePrivate::textureMap.end())
    {
        return false;
    }

    // Pull the pathname out of the descriptor and use it for the PNG load.
    TextureDescriptor* desc = textureIt->second;
    const std::string& filename = desc->pathname();
    ImageData image;

    if (desc->filetype() == TextureDescriptor::FileTypePNG) {
        if (!image.load_png(filename))
            return false;
    }
    else if (desc->filetype() == TextureDescriptor::FileTypeJPEG) {
        if (!image.load_jpeg(filename))
            return false;
    }

    va_list ap;
    va_start(ap, pTexture);
    GLint arg;

    while ((arg = va_arg(ap, GLint)) != 0) {
        GLint arg2 = va_arg(ap, GLint);
        setup_texture(pTexture, image, arg, arg2);
        pTexture++;
    }

    va_end(ap);

    return true;
}

const TextureMap&
Texture::find_textures()
{
    using std::vector;
    using std::string;
    if (!TexturePrivate::textureMap.empty())
    {
        return TexturePrivate::textureMap;
    }
    vector<string> pathVec;
    string dataDir(GLMARK_DATA_PATH"/textures");
    Util::list_files(dataDir, pathVec);
    // Now that we have a list of all of the model files available to us,
    // let's go through and pull out the names and what format they're in
    // so the scene can decide which ones to use.
    for(vector<string>::const_iterator pathIt = pathVec.begin();
        pathIt != pathVec.end();
        pathIt++)
    {
        const string& curPath = *pathIt;
        string::size_type namePos(0);
        string::size_type slashPos = curPath.rfind("/");
        if (slashPos != string::npos)
        {
            // Advance to the first character after the last slash
            namePos = slashPos + 1;
        }

        // Find the position of the extension
        string::size_type pngExtPos = curPath.rfind(".png");
        string::size_type jpgExtPos = curPath.rfind(".jpg");
        string::size_type extPos(string::npos);

        // Select the extension that's closer to the end of the file name
        if (pngExtPos == string::npos)
        {
            extPos = jpgExtPos;
        }
        else if (jpgExtPos == string::npos)
        {
            extPos = pngExtPos;
        }
        else
        {
            extPos = std::max(pngExtPos, jpgExtPos);
        }

        if (extPos == string::npos)
        {
            // We can't trivially determine it's an image file so skip it...
            continue;
        }

        // Set the file type based on the extension
        TextureDescriptor::FileType type(TextureDescriptor::FileTypeUnknown);
        if (extPos == pngExtPos)
        {
            type = TextureDescriptor::FileTypePNG;
        }
        else if (extPos == jpgExtPos)
        {
            type = TextureDescriptor::FileTypeJPEG;
        }

        string name(curPath, namePos, extPos - namePos);
        TextureDescriptor* desc = new TextureDescriptor(name, curPath, type);
        TexturePrivate::textureMap.insert(std::make_pair(name, desc));
    }

    return TexturePrivate::textureMap;
}
