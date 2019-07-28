/*
 * Copyright © 2011 Linaro Limited
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
 *  Alexandros Frantzis (glmark2)
 *  Jesse Barker
 *  Ivan Efremov
 */
#include "canvas-android.h"
#include "log.h"
#include "options.h"
#include "gl-headers.h"
#include <EGL/egl.h>

#include <fstream>
#include <sstream>

/******************
 * Public methods *
 ******************/

bool
CanvasAndroid::init()
{
    EGLint attribs[] = {
        EGL_CONFIG_ID, 0,
        EGL_NONE
    };

    /* Init the GLAD entrypoints */
    if (!egl_lib_.open_from_alternatives({"libEGL.so", "libEGL.so.1" })) {
        Log::error("Error loading EGL library\n");
        return false;
    }

    if (gladLoadEGLUserPtr(EGL_NO_DISPLAY, load_proc, &egl_lib_) == 0) {
        Log::error("Loading EGL entry points failed\n");
        return false;
    }

    /* Get the current EGL config */
    EGLDisplay egl_display(eglGetCurrentDisplay());

    /* Reinitialize GLAD with a known display */
    if (gladLoadEGLUserPtr(egl_display, load_proc, &egl_lib_) == 0) {
        Log::error("Loading EGL entry points with display failed\n");
        return false;
    }

    EGLContext egl_context(eglGetCurrentContext());
    EGLConfig egl_config(0);
    EGLint num_configs;

    eglQueryContext(egl_display, egl_context, EGL_CONFIG_ID, &(attribs[1]));

    eglChooseConfig(egl_display, attribs, &egl_config, 1, &num_configs);

    /* Before calling GLES functions, init GLAD GLES */
    if (!gles_lib_.open("libGLESv2.so")) {
        Log::error("Error loading GLES library\n");
        return false;
    }

    if (!gladLoadGLES2UserPtr(load_proc, &gles_lib_)) {
        Log::error("Loading GLESv2 entry points failed.");
        return false;
    }

    resize(width_, height_);

    if (!eglSwapInterval(egl_display, 0))
        Log::info("** Failed to set swap interval. Results may be bounded above by refresh rate.\n");

    init_gl_extensions();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    clear();

    int buf, red, green, blue, alpha, depth, id, native_id;
    eglGetConfigAttrib(egl_display, egl_config, EGL_CONFIG_ID, &id);
    eglGetConfigAttrib(egl_display, egl_config, EGL_NATIVE_VISUAL_ID, &native_id);
    eglGetConfigAttrib(egl_display, egl_config, EGL_BUFFER_SIZE, &buf);
    eglGetConfigAttrib(egl_display, egl_config, EGL_RED_SIZE, &red);
    eglGetConfigAttrib(egl_display, egl_config, EGL_GREEN_SIZE, &green);
    eglGetConfigAttrib(egl_display, egl_config, EGL_BLUE_SIZE, &blue);
    eglGetConfigAttrib(egl_display, egl_config, EGL_ALPHA_SIZE, &alpha);
    eglGetConfigAttrib(egl_display, egl_config, EGL_DEPTH_SIZE, &depth);
    
    if (Options::show_debug) {
        Log::debug("EGL chosen config ID: 0x%x Native Visual ID: 0x%x\n"
                   "  Buffer: %d bits\n"
                   "     Red: %d bits\n"
                   "   Green: %d bits\n"
                   "    Blue: %d bits\n"
                   "   Alpha: %d bits\n"
                   "   Depth: %d bits\n",
                   id, native_id,
                   buf, red, green, blue, alpha, depth);
    }

    recognize_format(buf, red, green, blue, alpha, depth);
    if(Options::offscreen)
    {
        Log::info("Offscreen mode ON\n");
        Log::debug("Width = %d, height = %d\n", width_, height_);
        if(ensure_fbo())
            glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
        else
            Log::info("Problems occur while creating an FBO !!!\n");
    }

    return true;
}

void
CanvasAndroid::visible(bool visible)
{
    static_cast<void>(visible);
}

void
CanvasAndroid::clear()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void
CanvasAndroid::update()
{
}

void
CanvasAndroid::print_info()
{
    std::stringstream ss;

    ss << "    OpenGL Information" << std::endl;
    ss << "    GL_VENDOR:     " << glGetString(GL_VENDOR) << std::endl;
    ss << "    GL_RENDERER:   " << glGetString(GL_RENDERER) << std::endl;
    ss << "    GL_VERSION:    " << glGetString(GL_VERSION) << std::endl;

    Log::info("%s", ss.str().c_str());
}

Canvas::Pixel
CanvasAndroid::read_pixel(int x, int y)
{
    uint8_t pixel[4];

    glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

    return Canvas::Pixel(pixel[0], pixel[1], pixel[2], pixel[3]);
}

void
CanvasAndroid::write_to_file(std::string &filename)
{
    if(Options::write_file)
    {
        Log::debug("Writing to file %s...\n", filename.c_str());
        int bpp = 24; //* Bits per pixel

        char *pixels = new char[width_ * height_ * 4];

        for (int i = 0; i < height_; i++) {
            glReadPixels(0, i, width_, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                        &pixels[(height_ - i - 1) * width_ * 4]);
        }
        
        unsigned char *dataBuffer = new unsigned char[width_ * height_ * 3];
        int x, y;

 	    for(y = 0; y < height_; ++y)
        {
		    for(x = 0; x < width_; ++x)
            {
                dataBuffer[(y * width_ + x) * 3 + 0] = pixels[((height_-y) * width_ + x) * 4 + 2];
                dataBuffer[(y * width_ + x) * 3 + 1] = pixels[((height_-y) * width_ + x) * 4 + 1];
                dataBuffer[(y * width_ + x) * 3 + 2] = pixels[((height_-y) * width_ + x) * 4 + 0];
            }
        }

        char *outdata = new char[width_ * height_ * (bpp / 8) + 18];
        char *data = outdata;

        // Build TGA header
        char c;
        short s;
        c = 0;      memcpy( data, &c, 1 ); data += 1;  // idLength
        c = 0;      memcpy( data, &c, 1 ); data += 1;  // colmapType
        c = 2;      memcpy( data, &c, 1 ); data += 1;  // imageType
        s = 0;      memcpy( data, &s, 2 ); data += 2;  // colmapStart
        s = 0;      memcpy( data, &s, 2 ); data += 2;  // colmapLength
        c = 0;      memcpy( data, &c, 1 ); data += 1;  // colmapBits
        s = 0;      memcpy( data, &s, 2 ); data += 2;  // x
        s = 0;      memcpy( data, &s, 2 ); data += 2;  // y
        s = width_;  memcpy( data, &s, 2 ); data += 2;  // width
        s = height_; memcpy( data, &s, 2 ); data += 2;  // height
        c = bpp;    memcpy( data, &c, 1 ); data += 1;  // bpp
        c = 0;      memcpy( data, &c, 1 ); data += 1;  // imageDesc

        if(dataBuffer)
            memcpy(data, dataBuffer, width_ * height_ * (bpp / 8));

        FILE* file = fopen(filename.c_str(), "w+b");
        
        if (file != NULL) {         // Checking the existing of file
            fwrite(outdata, 3, width_ * height_ * (bpp / 8) + 18, file);
            fclose(file); 
        } else
            Log::debug("File doesn't exist!\n");

        delete [] dataBuffer;
        delete [] pixels;
        delete [] outdata;

        Log::info("Writing to file %s successfully!\n", filename.c_str());
    }
}

bool
CanvasAndroid::should_quit()
{
    return false;
}

void
CanvasAndroid::resize(int width, int height)
{
    width_ = width;
    height_ = height;

    glViewport(0, 0, width_, height_);
    if(Options::offscreen)
    {
        Log::debug("CanvasAndroid::resize with offscreen\n");
        if (color_renderbuffer_) {
            glBindRenderbuffer(GL_RENDERBUFFER, color_renderbuffer_);
            glRenderbufferStorage(GL_RENDERBUFFER, gl_color_format_,
                                width_, height_);
        }

        if (depth_renderbuffer_) {
            glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer_);
            glRenderbufferStorage(GL_RENDERBUFFER, gl_depth_format_,
                                width_, height_);
        }
    }
    projection_ = LibMatrix::Mat4::perspective(60.0, width_ / static_cast<float>(height_),
                                               1.0, 1024.0);
}


unsigned int
CanvasAndroid::fbo()
{
    return fbo_;
}


/*******************
 * Private methods *
 *******************/

GLADapiproc
CanvasAndroid::load_proc(const char *name, void *userdata)
{
    if (eglGetProcAddress) {
        GLADapiproc sym = reinterpret_cast<GLADapiproc>(eglGetProcAddress(name));
        if (sym) {
            return sym;
        }
    }

    SharedLibrary* lib = reinterpret_cast<SharedLibrary*>(userdata);
    return reinterpret_cast<GLADapiproc>(lib->load(name));
}

bool
CanvasAndroid::ensure_gl_formats()
{
    if (gl_color_format_ && gl_depth_format_)
        return true;
    return false;
}

bool
CanvasAndroid::recognize_format(int buf, int red, int green, int blue,
                         int alpha, int depth)
{
    gl_color_format_ = 0;
    gl_depth_format_ = 0;

    bool supports_rgba8(false);
    bool supports_rgb8(false);
    bool supports_depth24(true);
    bool supports_depth32(false);

#if USE_GLESv2
    if (GLExtensions::support("GL_ARM_rgba8"))
        supports_rgba8 = true;

    if (GLExtensions::support("GL_OES_rgb8_rgba8")) {
        supports_rgba8 = true;
        supports_rgb8 = true;
    }

    if (GLExtensions::support("GL_OES_depth24"))
        supports_depth24 = true;

    if (GLExtensions::support("GL_OES_depth32"))
        supports_depth32 = true;
#elif USE_GL
    supports_rgba8 = true;
    supports_rgb8 = true;
    supports_depth24 = true;
    supports_depth32 = true;
#endif

    if (buf == 32) {
        if (supports_rgba8)
            gl_color_format_ = GL_RGBA8;
        else
            gl_color_format_ = GL_RGBA4;
    }
    else if (buf == 24) {
        if (supports_rgb8)
            gl_color_format_ = GL_RGB8;
        else
            gl_color_format_ = GL_RGB565;
    }
    else if (buf == 16) {
        if (red == 4 && green == 4 &&
            blue == 4 && alpha == 4)
        {
            gl_color_format_ = GL_RGBA4;
        }
        else if (red == 5 && green == 5 &&
                 blue == 5 && alpha == 1)
        {
            gl_color_format_ = GL_RGB5_A1;
        }
        else if (red == 5 && green == 6 &&
                 blue == 5 && alpha == 0)
        {
            gl_color_format_ = GL_RGB565;
        }
    }

    if (depth == 32 && supports_depth32)
        gl_depth_format_ = GL_DEPTH_COMPONENT32;
    else if (depth >= 24 && supports_depth24)
        gl_depth_format_ = GL_DEPTH_COMPONENT24;
    else if (depth == 16)
        gl_depth_format_ = GL_DEPTH_COMPONENT16;

    Log::info("Selected Renderbuffer ColorFormat: %s DepthFormat: %s\n",
               get_gl_format_str(gl_color_format_),
               get_gl_format_str(gl_depth_format_));

    return (gl_color_format_ && gl_depth_format_);
}

bool
CanvasAndroid::ensure_fbo()
{
    if (!fbo_) {
        if(!ensure_gl_formats())
            return false;

        Log::debug("Ensure FBO func begin. width = %d, height = %d\n", width_, height_);

        /* Create a texture for the color attachment  */
        glGenRenderbuffers(1, &color_renderbuffer_);
        glBindRenderbuffer(GL_RENDERBUFFER, color_renderbuffer_);
        glRenderbufferStorage(GL_RENDERBUFFER, gl_color_format_,
                              width_, height_);
        Log::debug("gl_color_format_ is %x\n", gl_color_format_);

        /* Create a renderbuffer for the depth attachment */
        glGenRenderbuffers(1, &depth_renderbuffer_);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer_);
        glRenderbufferStorage(GL_RENDERBUFFER, gl_depth_format_,
                              width_, height_);
        Log::debug("gl_depth_format_ is %x\n", gl_depth_format_);

        /* Create a FBO and set it up */
        glGenFramebuffers(1, &fbo_);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_RENDERBUFFER, color_renderbuffer_);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                  GL_RENDERBUFFER, depth_renderbuffer_);
        Log::debug("Ensure FBO func end.\n");
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if(status != GL_FRAMEBUFFER_COMPLETE) {
        Log::debug("Problem with OpenGL framebuffer after specifying color render buffer: n%xn", status);
    } else {
        Log::debug("FBO has created successfully");
    }
    return true;
}

void
CanvasAndroid::release_fbo()
{
    glDeleteFramebuffers(1, &fbo_);
    glDeleteRenderbuffers(1, &color_renderbuffer_);
    glDeleteRenderbuffers(1, &depth_renderbuffer_);
    fbo_ = 0;
    color_renderbuffer_ = 0;
    depth_renderbuffer_ = 0;

    gl_color_format_ = 0;
    gl_depth_format_ = 0;
}

const char *
CanvasAndroid::get_gl_format_str(GLenum f)
{
    const char *str;

    switch(f) {
        case GL_RGBA8: str = "GL_RGBA8"; break;
        case GL_RGB8: str = "GL_RGB8"; break;
        case GL_RGBA4: str = "GL_RGBA4"; break;
        case GL_RGB5_A1: str = "GL_RGB5_A1"; break;
        case GL_RGB565: str = "GL_RGB565"; break;
        case GL_DEPTH_COMPONENT16: str = "GL_DEPTH_COMPONENT16"; break;
        case GL_DEPTH_COMPONENT24: str = "GL_DEPTH_COMPONENT24"; break;
        case GL_DEPTH_COMPONENT32: str = "GL_DEPTH_COMPONENT32"; break;
        case GL_NONE: str = "GL_NONE"; break;
        default: str = "Unknown"; break;
    }

    return str;
}

void
CanvasAndroid::init_gl_extensions()
{
    /*
     * Parse the extensions we care about from the extension string.
     * Don't even bother to get function pointers until we know the
     * extension is present.
     */
    std::string extString;
    const char* exts = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
    if (exts) {
        extString = exts;
    }

    if (extString.find("GL_OES_mapbuffer") != std::string::npos) {
        GLExtensions::MapBuffer =
            reinterpret_cast<PFNGLMAPBUFFEROESPROC>(eglGetProcAddress("glMapBufferOES"));
        GLExtensions::UnmapBuffer =
            reinterpret_cast<PFNGLUNMAPBUFFEROESPROC>(eglGetProcAddress("glUnmapBufferOES"));
    }
}
