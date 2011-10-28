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
#include <fstream>
#include <sstream>

/******************
 * Public methods *
 ******************/

bool
CanvasX11::init()
{
    xdpy_ = XOpenDisplay(NULL);
    if (!xdpy_)
        return false;

    resize(width_, height_);

    if (!xwin_)
        return false;

    if (!make_current())
        return false;

    if (!supports_gl2()) {
        Log::error("Glmark2 needs OpenGL(ES) version >= 2.0 to run"
                   " (but version string is: '%s')!\n",
                   glGetString(GL_VERSION));
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    clear();

    return true;
}

void
CanvasX11::visible(bool visible)
{
    if (visible)
        XMapWindow(xdpy_, xwin_);
}

void
CanvasX11::clear()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
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
    if (Options::swap_buffers)
        swap_buffers();
    else
        glFinish();
}

void
CanvasX11::print_info()
{
    make_current();

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
    /* Recreate an existing window only if it has actually been resized */
    if (xwin_) {
        if (width_ != width || height_ != height) {
            XDestroyWindow(xdpy_, xwin_);
            xwin_ = 0;
        }
        else {
            return;
        }
    }

    width_ = width;
    height_ = height;

    if (!ensure_x_window())
        Log::error("Error: Couldn't create X Window!\n");

    glViewport(0, 0, width_, height_);
    projection_ = LibMatrix::Mat4::perspective(60.0, width_ / static_cast<float>(height_),
                                               1.0, 1024.0);
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

        std::stringstream ss;
        ss << gl_version_str.substr(start_pos, point_pos - start_pos + 1);
        ss >> gl_major;
    }

    return gl_major >= 2;
}

/*******************
 * Private methods *
 *******************/

bool
CanvasX11::ensure_x_window()
{
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
    {
        static const char *name("glmark2 "GLMARK_VERSION);
        XSizeHints sizehints;
        sizehints.min_width  = width_;
        sizehints.min_height = height_;
        sizehints.max_width  = width_;
        sizehints.max_height = height_;
        sizehints.flags = PMaxSize | PMinSize;
        XSetNormalHints(xdpy_, xwin_, &sizehints);
        XSetStandardProperties(xdpy_, xwin_, name, name,
                               None, NULL, 0, &sizehints);
    }

    /* Gracefully handle Window Delete event from window manager */
    Atom wmDelete = XInternAtom(xdpy_, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(xdpy_, xwin_, &wmDelete, 1);

    return true;
}

