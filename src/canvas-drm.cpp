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
#include "canvas-drm.h"
#include "log.h"
#include "options.h"
#include "util.h"

#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#include <fstream>
#include <sstream>
#include <list>

/******************
 * Public methods *
 ******************/

bool
CanvasDRM::reset()
{
    if (!reset_context())
        return false;

    if (!make_current())
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
    egl_.swap();

    if (!drm_.reset()) {
        return false;
    }

    return true;
}

void
DRMState::fb_destroy_callback(gbm_bo* bo, void* data)
{
    DRMFBState* fb = reinterpret_cast<DRMFBState*>(data);
    if (fb && fb->fb_id) {
        drmModeRmFB(fb->fd, fb->fb_id);
    }
    delete fb;
    gbm_device* dev = gbm_bo_get_device(bo);
    Log::debug("Got GBM device handle %p from buffer object\n", dev);
}

DRMFBState*
DRMState::fb_get_from_bo(gbm_bo* bo)
{
    DRMFBState* fb = reinterpret_cast<DRMFBState*>(gbm_bo_get_user_data(bo));
    if (fb) {
        return fb;
    }

    unsigned int width = gbm_bo_get_width(bo);
    unsigned int height = gbm_bo_get_height(bo);
    unsigned int stride = gbm_bo_get_stride(bo);
    unsigned int handle = gbm_bo_get_handle(bo).u32;
    unsigned int fb_id(0);
    int status = drmModeAddFB(fd_, width, height, 24, 32, stride, handle, &fb_id);
    if (status < 0) {
        Log::error("Failed to create FB: %d\n", status);
        return 0;
    }

    fb = new DRMFBState();
    fb->fd = fd_;
    fb->bo = bo;
    fb->fb_id = fb_id;

    gbm_bo_set_user_data(bo, fb, fb_destroy_callback);
    return fb;
}

bool
DRMState::init_gbm()
{
    dev_ = gbm_create_device(fd_);
    if (!dev_) {
        Log::error("Failed to create GBM device\n");
        return false;
    }

    surface_ = gbm_surface_create(dev_, mode_->hdisplay, mode_->vdisplay,
                                  GBM_FORMAT_XRGB8888,
                                  GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
    if (!surface_) {
        Log::error("Failed to create GBM surface\n");
        return false;
    }

    return true;
}

bool
DRMState::init()
{
    // TODO: Replace this with something that explicitly probes for the loaded
    // driver (udev?).
    static const char* drm_modules[] = {
        "i915",
        "nouveau",
        "radeon",
        "vmgfx",
        "omapdrm",
        "exynos"
    };

    unsigned int num_modules(sizeof(drm_modules)/sizeof(drm_modules[0]));
    for (unsigned int m = 0; m < num_modules; m++) {
        fd_ = drmOpen(drm_modules[m], 0);
        if (fd_ < 0) {
            Log::debug("Failed to open DRM module '%s'\n", drm_modules[m]);
            continue;
        }
        Log::debug("Opened DRM module '%s'\n", drm_modules[m]);
        break;
    }

    if (fd_ < 0) {
        Log::error("Failed to find a suitable DRM device\n");
        return false;
    }

    resources_ = drmModeGetResources(fd_);
    if (!resources_) {
        Log::error("drmModeGetResources failed\n");
        return false;
    }

    // Find a connected connector
    for (int c = 0; c < resources_->count_connectors; c++) {
        connector_ = drmModeGetConnector(fd_, resources_->connectors[c]);
        if (DRM_MODE_CONNECTED == connector_->connection) {
            break;
        }
        drmModeFreeConnector(connector_);
        connector_ = 0;
    }

    if (!connector_) {
        Log::error("Failed to find a suitable connector\n");
        return false;
    }

    // Find the best resolution (we will always operate full-screen).
    unsigned int bestArea(0);
    for (int m = 0; m < connector_->count_modes; m++) {
        drmModeModeInfo* curMode = &connector_->modes[m];
        unsigned int curArea = curMode->hdisplay * curMode->vdisplay;
        if (curArea > bestArea) {
            mode_ = curMode;
            bestArea = curArea;
        }
    }

    if (!mode_) {
        Log::error("Failed to find a suitable mode\n");
        return false;
    }

    // Find a suitable encoder
    for (int e = 0; e < resources_->count_encoders; e++) {
        encoder_ = drmModeGetEncoder(fd_, resources_->encoders[e]);
        if (encoder_ && encoder_->encoder_id == connector_->encoder_id) {
            break;
        }
        drmModeFreeEncoder(encoder_);
        encoder_ = 0;
    }

    if (!encoder_) {
        Log::error("Failed to find a suitable encoder\n");
        return false;
    }

    if (!init_gbm()) {
        return false;
    }

    crtc_ = drmModeGetCrtc(fd_, encoder_->crtc_id);
    if (!crtc_) {
        Log::error("Failed to get current CRTC\n");
        return false;
    }

    return true;
}

bool
DRMState::reset()
{
    if (bo_) {
        gbm_surface_release_buffer(surface_, bo_);
    }

    bo_ = gbm_surface_lock_front_buffer(surface_);
    fb_ = fb_get_from_bo(bo_);

    int status = drmModeSetCrtc(fd_, encoder_->crtc_id, fb_->fb_id, 0, 0,
                                &connector_->connector_id, 1, mode_);
    if (status < 0) {
        Log::error("Failed to set CRTC: %d\n", status);
        return false;
    }

    return true;
}

void
DRMState::page_flip_handler(int fd, unsigned int frame, unsigned int sec, unsigned int usec, void* data)
{
    unsigned int* waiting = reinterpret_cast<unsigned int*>(data);
    *waiting = 0;
    // Deal with unused parameters
    static_cast<void>(fd);
    static_cast<void>(frame);
    static_cast<void>(sec);
    static_cast<void>(usec);
}

void
DRMState::do_flip()
{
    gbm_bo* next = gbm_surface_lock_front_buffer(surface_);
    fb_ = fb_get_from_bo(next);
    unsigned int waiting(1);
    int status = drmModePageFlip(fd_, encoder_->crtc_id, fb_->fb_id,
                                 DRM_MODE_PAGE_FLIP_EVENT, &waiting);
    if (status < 0) {
        Log::error("Failed to enqueue page flip: %d\n", status);
        return;
    }

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    FD_SET(fd_, &fds);
    drmEventContext evCtx;
    evCtx.version = DRM_EVENT_CONTEXT_VERSION;
    evCtx.page_flip_handler = page_flip_handler;

    while (waiting) {
        status = select(fd_ + 1, &fds, 0, 0, 0);
        if (status < 0) {
            Log::error("Error in select: %d\n", status);
            return;
        }
        else if (status == 0) {
            Log::info("Timeout in select\n");
            return;
        }
        else if (FD_ISSET(0, &fds)) {
            Log::info("User interrupt received\n");
            return;
        }
        drmHandleEvent(fd_, &evCtx);
    }

    gbm_surface_release_buffer(surface_, bo_);
    bo_ = next;
}

void
DRMState::cleanup()
{
    // Restore CRTC state if necessary
    if (crtc_) {
        int status = drmModeSetCrtc(fd_, crtc_->crtc_id, crtc_->buffer_id,
                                    crtc_->x, crtc_->y, &connector_->connector_id,
                                    1, &crtc_->mode);
        if (status < 0) {
            Log::error("Failed to restore original CRTC: %d\n", status);
        }
        drmModeFreeCrtc(crtc_);
        crtc_ = 0;
    }
    if (surface_) {
        gbm_surface_destroy(surface_);
        surface_ = 0;
    }
    if (dev_) {
        gbm_device_destroy(dev_);
        dev_ = 0;
    }
    if (connector_) {
        drmModeFreeConnector(connector_);
        connector_ = 0;
    }
    if (encoder_) {
        drmModeFreeEncoder(encoder_);
        encoder_ = 0;
    }
    if (resources_) {
        drmModeFreeResources(resources_);
        resources_ = 0;
    }
    if (fd_ > 0) {
        drmClose(fd_);
    }
    fd_ = 0;
    mode_ = 0;
}

bool
CanvasDRM::init()
{
    signal(SIGINT, &CanvasDRM::quit_handler);

    if (!drm_.init()) {
        Log::error("Failed to initialize the DRM canvas\n");
        drm_.cleanup();
        return false;
    }

    width_ = drm_.mode_width();
    height_ = drm_.mode_height();
    resize_no_viewport(width_, height_);

    if (!egl_.init_display(drm_.device(), visual_config_)) {
        return false;
    }
    if (!egl_.init_surface(drm_.surface())) {
        return false;
    }
    if (!egl_.valid()) {
        return false;
    }

    return reset();
}


CanvasDRM::~CanvasDRM()
{
    drm_.cleanup();
}

void
CanvasDRM::visible(bool /* visible */)
{
}

void
CanvasDRM::clear()
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
CanvasDRM::update()
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
CanvasDRM::print_info()
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
CanvasDRM::read_pixel(int x, int y)
{
    uint8_t pixel[4];
    glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    return Canvas::Pixel(pixel[0], pixel[1], pixel[2], pixel[3]);
}

void
CanvasDRM::write_to_file(std::string &filename)
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
CanvasDRM::should_quit()
{
    return should_quit_;
}

void
CanvasDRM::resize(int width, int height)
{
    resize_no_viewport(width, height);
    glViewport(0, 0, width_, height_);
}

/*********************
 * Protected methods *
 *********************/

bool
CanvasDRM::supports_gl2()
{
    std::string gl_version_str(
        reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    int gl_major = 0;

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

bool
CanvasDRM::make_current()
{
    if (!egl_.valid()) {
        Log::error("CanvasDRM: Invalid EGL state\n");
        return false;
    }

    init_gl_extensions();

    return true;
}

bool
CanvasDRM::reset_context()
{
    return egl_.reset();
}

void
CanvasDRM::swap_buffers()
{
    egl_.swap();
    drm_.do_flip();
}

void
CanvasDRM::init_gl_extensions()
{
#if USE_GLESv2
    /*
     * Parse the extensions we care about from the extension string.
     * Don't even bother to get function pointers until we know the
     * extension is present.
     */
    std::string extString;
    const char* exts =
        reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
    if (exts)
        extString = exts;

    if (extString.find("GL_OES_mapbuffer") != std::string::npos) {
        GLExtensions::MapBuffer = reinterpret_cast<PFNGLMAPBUFFEROESPROC>(
            eglGetProcAddress("glMapBufferOES"));
        GLExtensions::UnmapBuffer = reinterpret_cast<PFNGLUNMAPBUFFEROESPROC>(
            eglGetProcAddress("glUnmapBufferOES"));
    }
#elif USE_GL
    GLExtensions::MapBuffer = glMapBuffer;
    GLExtensions::UnmapBuffer = glUnmapBuffer;
#endif
}


/*******************
 * Private methods *
 *******************/

void
CanvasDRM::resize_no_viewport(int width, int height)
{
    width_ = drm_.mode_width();
    height_ = drm_.mode_height();

    if (!width_)
        width_ = width;
    if (!height_)
        height_ = height;

    projection_ =
        LibMatrix::Mat4::perspective(60.0, width_ / static_cast<float>(height_),
                                     1.0, 1024.0);
}

bool CanvasDRM::should_quit_ = false;

void
CanvasDRM::quit_handler(int /* signum */)
{
    should_quit_ = true;
}
