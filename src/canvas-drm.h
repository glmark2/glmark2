//
// Copyright © 2012 Linaro Limited
//
// This file is part of the glmark2 OpenGL (ES) 2.0 benchmark.
//
// glmark2 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// glmark2 is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// glmark2.  If not, see <http://www.gnu.org/licenses/>.
//
// Authors:
//  Simon Que 
//  Jesse Barker
//
#ifndef GLMARK2_CANVAS_DRM_H_
#define GLMARK2_CANVAS_DRM_H_

#include <cstring>
#include <gbm.h>
#include <drm.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include "canvas.h"
#include "egl-state.h"

struct DRMFBState
{
    int fd;
    gbm_bo* bo;
    uint32_t fb_id;
};

class DRMState
{
    static void page_flip_handler(int fd, unsigned int frame, unsigned int sec,
                                  unsigned int usec, void* data);
    static void fb_destroy_callback(gbm_bo* bo, void* data);
    DRMFBState* fb_get_from_bo(gbm_bo* bo);
    bool init_gbm();
    int fd_;
    drmModeRes* resources_;
    drmModeConnector* connector_;
    drmModeEncoder* encoder_;
    drmModeCrtcPtr crtc_;
    drmModeModeInfo* mode_;
    gbm_device* dev_;
    gbm_surface* surface_;
    gbm_bo* bo_;
    DRMFBState* fb_;

public:
    DRMState() :
        fd_(0),
        resources_(0),
        connector_(0),
        encoder_(0),
        mode_(0),
        dev_(0),
        surface_(0),
        bo_(0),
        fb_(0) {}
    ~DRMState() { cleanup(); }
    void cleanup();
    bool init();
    bool reset();
    void do_flip();
    gbm_device* device() const { return dev_; }
    gbm_surface* surface() const { return surface_; }
    unsigned int mode_width() const
    { 
        if (mode_) {
            return mode_->hdisplay;
        }
        return 0;
    }
    unsigned int mode_height() const
    {
        if (mode_) {
            return mode_->vdisplay;
        }
        return 0;
    }
};

/**
 * Canvas for direct rendering with EGL.
 */
class CanvasDRM: public Canvas
{
public:
    CanvasDRM(int width, int height) :
        Canvas(width, height) {}
    ~CanvasDRM();

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

protected:
    virtual bool make_current();
    virtual bool reset_context();
    virtual void swap_buffers();
    virtual bool supports_gl2();

private:
    DRMState drm_;
    EGLState egl_;

    void resize_no_viewport(int width, int height);
    void init_gl_extensions();

    static void quit_handler(int signum);
    static bool should_quit_;
};

#endif
