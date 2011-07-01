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
 */
#include "canvas-x11.h"
#include "log.h"
#include "options.h"

#include <X11/keysym.h>
#include <fstream>
#include <sstream>

static Window
create_canvas_x_window(Display *xdpy, const char *name, int width, int height,
                       const XVisualInfo *vis_info)
{
    XSetWindowAttributes attr;
    unsigned long mask;
    Window win = 0;
    Window root = RootWindow(xdpy, DefaultScreen(xdpy));

    Log::debug("Creating XWindow W: %d H: %d VisualID: 0x%x\n",
               width, height, (int)vis_info->visualid);
    /* window attributes */
    attr.background_pixel = 0;
    attr.border_pixel = 0;
    attr.colormap = XCreateColormap(xdpy, root, vis_info->visual, AllocNone);
    attr.event_mask = KeyPressMask;
    mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

    win = XCreateWindow(xdpy, root, 0, 0, width, height,
                        0, vis_info->depth, InputOutput,
                        vis_info->visual, mask, &attr);

    /* set hints and properties */
    {
        XSizeHints sizehints;
        sizehints.min_width  = width;
        sizehints.min_height = height;
        sizehints.max_width  = width;
        sizehints.max_height = height;
        sizehints.flags = PMaxSize | PMinSize;
        XSetNormalHints(xdpy, win, &sizehints);
        XSetStandardProperties(xdpy, win, name, name,
                               None, NULL, 0, &sizehints);
    }

    /* Gracefully handle Window Delete event from window manager */
    Atom wmDelete = XInternAtom(xdpy, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(xdpy, win, &wmDelete, 1);

    return win;
}

bool
CanvasX11::init()
{
    xdpy_ = XOpenDisplay(NULL);
    if (!xdpy_)
        return false;

    XVisualInfo *visinfo = get_xvisualinfo();

    xwin_ = create_canvas_x_window(xdpy_, "glmark2 "GLMARK_VERSION, mWidth, mHeight, visinfo);

    XFree(visinfo);

    if (!xwin_)
        return false;

    if (!make_current())
        return false;

    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
#if USE_GL
    glClearDepth(1.0f);
#elif USE_GLESv2
    glClearDepthf(1.0f);
#endif
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glViewport(0, 0, mWidth, mHeight);

    clear();

    mProjection = LibMatrix::Mat4::perspective(60.0, mWidth / (float)mHeight,
                                               1.0, 1024.0);

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
    char *pixels = new char[mWidth * mHeight * 4];

    for (int i = 0; i < mHeight; i++) {
        glReadPixels(0, i, mWidth, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                     &pixels[(mHeight - i - 1) * mWidth * 4]);
    }

    std::ofstream output (filename.c_str(), std::ios::out | std::ios::binary);
    output.write(pixels, 4 * mWidth * mHeight);

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
