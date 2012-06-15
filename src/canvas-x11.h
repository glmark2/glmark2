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
#ifndef GLMARK2_CANVAS_X11_H_
#define GLMARK2_CANVAS_X11_H_

#include "canvas.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>

/**
 * Canvas for rendering with GL to an X11 window.
 */
class CanvasX11 : public Canvas
{
public:
    ~CanvasX11() {}

    virtual bool init();
    virtual bool reset();
    virtual void visible(bool visible);
    virtual void clear();
    virtual void update();
    virtual void print_info();
    virtual Pixel read_pixel(int x, int y);
    virtual void write_to_file(std::string &filename);
    virtual bool should_quit();
    virtual void resize(int width, int height);
    virtual unsigned int fbo();

protected:
    CanvasX11(int width, int height) :
        Canvas(width, height), xwin_(0), xdpy_(0), fullscreen_(false),
        gl_color_format_(0), gl_depth_format_(0),
        color_renderbuffer_(0), depth_renderbuffer_(0), fbo_(0) {}

    /**
     * Gets the XVisualInfo to use for creating the X window with.
     *
     * The caller should XFree() the returned XVisualInfo when done.
     *
     * This method should be implemented in derived classes.
     *
     * @return the XVisualInfo
     */
    virtual XVisualInfo *get_xvisualinfo() = 0;

    /**
     * Makes the canvas the current target for GL rendering.
     *
     * This method should be implemented in derived classes.
     *
     * @return whether the operation succeeded
     */
    virtual bool make_current() = 0;

    /**
     * Resets the underlying GL context for rendering.
     *
     * This method should be implemented in derived classes.
     *
     * @return whether the operation succeeded
     */
    virtual bool reset_context() = 0;

    /**
     * Swaps the GL buffers (assuming double buffering is used).
     *
     * This method should be implemented in derived classes.
     *
     * @return whether the operation succeeded
     */
    virtual void swap_buffers() = 0;

    /**
     * Gets information about the GL visual used for this canvas.
     *
     * This method should be implemented in derived classes.
     */
    virtual void get_glvisualconfig(GLVisualConfig &visual_config) = 0;

    /**
     * Whether the current implementation supports GL(ES) 2.0.
     *
     * @return true if it supports GL(ES) 2.0, false otherwise
     */
    bool supports_gl2();


    /** The X window associated with this canvas. */
    Window xwin_;
    /** The X display associated with this canvas. */
    Display *xdpy_;

private:
    void resize_no_viewport(int width, int height);
    bool ensure_x_window();
    bool do_make_current();
    bool ensure_gl_formats();
    bool ensure_fbo();
    void release_fbo();

    const char *get_gl_format_str(GLenum f);

    bool fullscreen_;
    GLenum gl_color_format_;
    GLenum gl_depth_format_;
    GLuint color_renderbuffer_;
    GLuint depth_renderbuffer_;
    GLuint fbo_;
};

#endif

