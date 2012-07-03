/*
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
 *  Alexandros Frantzis (glmark2)
 *  Jesse Barker
 */
#include "canvas-x11.h"
#include "log.h"
#include "options.h"
#include "util.h"

#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <fstream>
#include <sstream>

/******************
 * Public methods *
 ******************/
bool
CanvasX11::reset()
{
    release_fbo();

    if (!reset_context())
        return false;

    if (!do_make_current())
        return false;

    if (!supports_gl2()) {
        Log::error("Glmark2 needs OpenGL(ES) version >= 2.0 to run"
                   " (but version string is: '%s')!\n",
                   glGetString(GL_VERSION));
        return false;
    }

    glViewport(0, 0, width_, height_);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    clear();

    return true;
}

bool
CanvasX11::init()
{
    xdpy_ = XOpenDisplay(NULL);
    if (!xdpy_)
        return false;

    resize_no_viewport(width_, height_);

    if (!xwin_)
        return false;

    return reset();
}

void
CanvasX11::visible(bool visible)
{
    if (visible && !offscreen_)
        XMapWindow(xdpy_, xwin_);
}

void
CanvasX11::clear()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
#if USE_GL
    glClearDepth(1.0f);
#elif USE_GLESv2
    glClearDepthf(1.0f);
#endif
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void
CanvasX11::update()
{
    Options::FrameEnd m = Options::frame_end;

    if (m == Options::FrameEndDefault) {
        if (offscreen_)
            m = Options::FrameEndFinish;
        else
            m = Options::FrameEndSwap;
    }

    switch(m) {
        case Options::FrameEndSwap:
            swap_buffers();
            break;
        case Options::FrameEndFinish:
            glFinish();
            break;
        case Options::FrameEndReadPixels:
            read_pixel(width_ / 2, height_ / 2);
            break;
        case Options::FrameEndNone:
        default:
            break;
    }
}

void
CanvasX11::print_info()
{
    do_make_current();

    std::stringstream ss;

    ss << "    OpenGL Information" << std::endl;
    ss << "    GL_VENDOR:     " << glGetString(GL_VENDOR) << std::endl;
    ss << "    GL_RENDERER:   " << glGetString(GL_RENDERER) << std::endl;
    ss << "    GL_VERSION:    " << glGetString(GL_VERSION) << std::endl;

    Log::info("%s", ss.str().c_str());
}

Canvas::Pixel
CanvasX11::read_pixel(int x, int y)
{
    uint8_t pixel[4];

    glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

    return Canvas::Pixel(pixel[0], pixel[1], pixel[2], pixel[3]);
}

void
CanvasX11::write_to_file(std::string &filename)
{
    char *pixels = new char[width_ * height_ * 4];

    for (int i = 0; i < height_; i++) {
        glReadPixels(0, i, width_, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                     &pixels[(height_ - i - 1) * width_ * 4]);
    }

    std::ofstream output (filename.c_str(), std::ios::out | std::ios::binary);
    output.write(pixels, 4 * width_ * height_);

    delete [] pixels;
}

bool
CanvasX11::should_quit()
{
    XEvent event;

    if (!XPending(xdpy_))
        return false;

    XNextEvent(xdpy_, &event);

    if (event.type == KeyPress) {
        if (XLookupKeysym(&event.xkey, 0) == XK_Escape)
            return true;
    }
    else if (event.type == ClientMessage) {
        /* Window Delete event from window manager */
        return true;
    }

    return false;
}

void
CanvasX11::resize(int width, int height)
{
    resize_no_viewport(width, height);
    glViewport(0, 0, width_, height_);
}

unsigned int
CanvasX11::fbo()
{
    return fbo_;
}

bool
CanvasX11::supports_gl2()
{
    std::string gl_version_str(reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    int gl_major(0);

    size_t point_pos(gl_version_str.find('.'));

    if (point_pos != std::string::npos) {
        point_pos--;

        size_t start_pos(gl_version_str.rfind(' ', point_pos));
        if (start_pos == std::string::npos)
            start_pos = 0;
        else
            start_pos++;

        gl_major = Util::fromString<int>(
                gl_version_str.substr(start_pos, point_pos - start_pos + 1)
                );
    }

    return gl_major >= 2;
}

/*******************
 * Private methods *
 *******************/

bool
CanvasX11::ensure_x_window()
{
    static const char *win_name("glmark2 "GLMARK_VERSION);

    if (xwin_)
        return true;

    if (!xdpy_) {
        Log::error("Error: X11 Display has not been initialized!\n");
        return false;
    }

    XVisualInfo *vis_info = get_xvisualinfo();
    if (!vis_info) {
        Log::error("Error: Could not get a valid XVisualInfo!\n");
        return false;
    }

    Log::debug("Creating XWindow W: %d H: %d VisualID: 0x%x\n",
               width_, height_, vis_info->visualid);

    /* window attributes */
    XSetWindowAttributes attr;
    unsigned long mask;
    Window root = RootWindow(xdpy_, DefaultScreen(xdpy_));

    attr.background_pixel = 0;
    attr.border_pixel = 0;
    attr.colormap = XCreateColormap(xdpy_, root, vis_info->visual, AllocNone);
    attr.event_mask = KeyPressMask;
    mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

    xwin_ = XCreateWindow(xdpy_, root, 0, 0, width_, height_,
                          0, vis_info->depth, InputOutput,
                          vis_info->visual, mask, &attr);

    XFree(vis_info);

    if (!xwin_) {
        Log::error("Error: XCreateWindow() failed!\n");
        return false;
    }

    /* set hints and properties */
    if (fullscreen_) {
        Atom atom = XInternAtom(xdpy_, "_NET_WM_STATE_FULLSCREEN", True);
        XChangeProperty(xdpy_, xwin_,
                        XInternAtom(xdpy_, "_NET_WM_STATE", True),
                        XA_ATOM, 32, PropModeReplace,
                        reinterpret_cast<unsigned char*>(&atom),  1);
    }
    else {
        XSizeHints sizehints;
        sizehints.min_width  = width_;
        sizehints.min_height = height_;
        sizehints.max_width  = width_;
        sizehints.max_height = height_;
        sizehints.flags = PMaxSize | PMinSize;

        XSetWMProperties(xdpy_, xwin_, NULL, NULL,
                         NULL, 0, &sizehints, NULL, NULL);
    }

    /* Set the window name */
    XStoreName(xdpy_ , xwin_,  win_name);

    /* Gracefully handle Window Delete event from window manager */
    Atom wmDelete = XInternAtom(xdpy_, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(xdpy_, xwin_, &wmDelete, 1);

    return true;
}

void
CanvasX11::resize_no_viewport(int width, int height)
{
    bool request_fullscreen = (width == -1 || height == -1);

    /* Recreate an existing window only if it has actually been resized */
    if (xwin_) {
        if (width_ != width || height_ != height ||
            fullscreen_ != request_fullscreen)
        {
            XDestroyWindow(xdpy_, xwin_);
            xwin_ = 0;
        }
        else
        {
            return;
        }
    }

    fullscreen_ = request_fullscreen;

    if (fullscreen_) {
        /* Get the screen (root window) size */
        XWindowAttributes window_attr;
        XGetWindowAttributes(xdpy_, RootWindow(xdpy_, DefaultScreen(xdpy_)), 
                             &window_attr);
        width_ = window_attr.width;
        height_ = window_attr.height;
    }
    else {
        width_ = width;
        height_ = height;
    }

    if (!ensure_x_window())
        Log::error("Error: Couldn't create X Window!\n");

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

    projection_ = LibMatrix::Mat4::perspective(60.0, width_ / static_cast<float>(height_),
                                               1.0, 1024.0);
}

bool
CanvasX11::do_make_current()
{
    if (!make_current())
        return false;

    if (offscreen_) {
        if (!ensure_fbo())
            return false;

        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    }

    return true;
}

bool
CanvasX11::ensure_gl_formats()
{
    if (gl_color_format_ && gl_depth_format_)
        return true;

    GLVisualConfig vc;
    get_glvisualconfig(vc);

    gl_color_format_ = 0;
    gl_depth_format_ = 0;

    bool supports_rgba8(false);
    bool supports_rgb8(false);
    bool supports_depth24(false);
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

    if (vc.buffer == 32) {
        if (supports_rgba8)
            gl_color_format_ = GL_RGBA8;
        else
            gl_color_format_ = GL_RGBA4;
    }
    else if (vc.buffer == 24) {
        if (supports_rgb8)
            gl_color_format_ = GL_RGB8;
        else
            gl_color_format_ = GL_RGB565;
    }
    else if (vc.buffer == 16) {
        if (vc.red == 4 && vc.green == 4 &&
            vc.blue == 4 && vc.alpha == 4)
        {
            gl_color_format_ = GL_RGBA4;
        }
        else if (vc.red == 5 && vc.green == 5 &&
                 vc.blue == 5 && vc.alpha == 1)
        {
            gl_color_format_ = GL_RGB5_A1;
        }
        else if (vc.red == 5 && vc.green == 6 &&
                 vc.blue == 5 && vc.alpha == 0)
        {
            gl_color_format_ = GL_RGB565;
        }
    }

    if (vc.depth == 32 && supports_depth32)
        gl_depth_format_ = GL_DEPTH_COMPONENT32;
    else if (vc.depth >= 24 && supports_depth24)
        gl_depth_format_ = GL_DEPTH_COMPONENT24;
    else if (vc.depth == 16)
        gl_depth_format_ = GL_DEPTH_COMPONENT16;

    Log::debug("Selected Renderbuffer ColorFormat: %s DepthFormat: %s\n",
               get_gl_format_str(gl_color_format_),
               get_gl_format_str(gl_depth_format_));

    return (gl_color_format_ && gl_depth_format_);
}

bool
CanvasX11::ensure_fbo()
{
    if (!fbo_) {
        if (!ensure_gl_formats())
            return false;

        /* Create a texture for the color attachment  */
        glGenRenderbuffers(1, &color_renderbuffer_);
        glBindRenderbuffer(GL_RENDERBUFFER, color_renderbuffer_);
        glRenderbufferStorage(GL_RENDERBUFFER, gl_color_format_,
                              width_, height_);

        /* Create a renderbuffer for the depth attachment */
        glGenRenderbuffers(1, &depth_renderbuffer_);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer_);
        glRenderbufferStorage(GL_RENDERBUFFER, gl_depth_format_,
                              width_, height_);

        /* Create a FBO and set it up */
        glGenFramebuffers(1, &fbo_);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_RENDERBUFFER, color_renderbuffer_);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                  GL_RENDERBUFFER, depth_renderbuffer_);
    }

    return true;
}

void
CanvasX11::release_fbo()
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
CanvasX11::get_gl_format_str(GLenum f)
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

