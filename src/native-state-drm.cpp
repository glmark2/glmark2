/*
 * Copyright © 2012 Linaro Limited
 * Copyright © 2013 Canonical Ltd
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
 *  Simon Que 
 *  Jesse Barker
 *  Alexandros Frantzis
 */
#include "native-state-drm.h"
#include "log.h"

#include <fcntl.h>
#include <libudev.h>

/******************
 * Public methods *
 ******************/

bool
NativeStateDRM::init_display()
{
    if (!dev_)
        init();

    return (dev_ != 0);
}

void*
NativeStateDRM::display()
{
    return static_cast<void*>(dev_);
}

bool
NativeStateDRM::create_window(WindowProperties const& /*properties*/)
{
    if (!dev_) {
        Log::error("Error: DRM device has not been initialized!\n");
        return false;
    }

    return true;
}

void*
NativeStateDRM::window(WindowProperties& properties)
{
    properties = WindowProperties(mode_->hdisplay,
                                  mode_->vdisplay,
                                  true, 0);
    return static_cast<void*>(surface_);
}

void
NativeStateDRM::visible(bool /*visible*/)
{
}

bool
NativeStateDRM::should_quit()
{
    return should_quit_;
}

void
NativeStateDRM::flip()
{
    gbm_bo* next = gbm_surface_lock_front_buffer(surface_);
    fb_ = fb_get_from_bo(next);
    unsigned int waiting(1);

    if (!crtc_set_) {
        int status = drmModeSetCrtc(fd_, encoder_->crtc_id, fb_->fb_id, 0, 0,
                                    &connector_->connector_id, 1, mode_);
        if (status >= 0) {
            crtc_set_ = true;
            bo_ = next;
        }
        else {
            Log::error("Failed to set crtc: %d\n", status);
        }
        return;
    }

    int status = drmModePageFlip(fd_, encoder_->crtc_id, fb_->fb_id,
                                 DRM_MODE_PAGE_FLIP_EVENT, &waiting);
    if (status < 0) {
        Log::error("Failed to enqueue page flip: %d\n", status);
        return;
    }

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd_, &fds);
    drmEventContext evCtx;
    memset(&evCtx, 0, sizeof(evCtx));
    evCtx.version = DRM_EVENT_CONTEXT_VERSION;
    evCtx.page_flip_handler = page_flip_handler;

    while (waiting) {
        status = select(fd_ + 1, &fds, 0, 0, 0);
        if (status < 0) {
            // Most of the time, select() will return an error because the
            // user pressed Ctrl-C.  So, only print out a message in debug
            // mode, and just check for the likely condition and release
            // the current buffer object before getting out.
            Log::debug("Error in select\n");
            if (should_quit()) {
                gbm_surface_release_buffer(surface_, bo_);
                bo_ = next;
            }
            return;
        }
        drmHandleEvent(fd_, &evCtx);
    }

    gbm_surface_release_buffer(surface_, bo_);
    bo_ = next;
}

/*******************
 * Private methods *
 *******************/

void
NativeStateDRM::fb_destroy_callback(gbm_bo* bo, void* data)
{
    DRMFBState* fb = reinterpret_cast<DRMFBState*>(data);
    if (fb && fb->fb_id) {
        drmModeRmFB(fb->fd, fb->fb_id);
    }
    delete fb;
    gbm_device* dev = gbm_bo_get_device(bo);
    Log::debug("Got GBM device handle %p from buffer object\n", dev);
}

NativeStateDRM::DRMFBState*
NativeStateDRM::fb_get_from_bo(gbm_bo* bo)
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
NativeStateDRM::init_gbm()
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

char const *
NativeStateDRM::drm_primary_gpu_device_node
(struct udev * __restrict const udev,
 struct udev_enumerate * __restrict const dev_enum)
{
    char const * __restrict result = NULL;

    struct udev_list_entry * device_list =
        udev_enumerate_get_list_entry(dev_enum);

    struct udev_list_entry * __restrict current_element;

    udev_list_entry_foreach(current_element, device_list) {
        
        char const * __restrict current_element_sys_path =
            udev_list_entry_get_name(current_element);
        
        struct udev_device * __restrict const current_device =
            udev_device_new_from_syspath(udev, current_element_sys_path);
        
        struct udev_device * __restrict const potential_primary_gpu =
            udev_device_get_parent_with_subsystem_devtype(
                current_device, "pci", NULL
            );
        
        if (potential_primary_gpu) {
            char const * __restrict const value = 
                udev_device_get_sysattr_value(
                    potential_primary_gpu, "boot_vga"
                );
            if (value) {
                result = udev_device_get_devnode(current_device);
                break;
            }
        }
        
        current_element = udev_list_entry_get_next(current_element);
    }

    return result;
}

/* Inspired by KWin detection mechanism */
char const *
NativeStateDRM::udev_main_gpu_drm_node_path()
{
    Log::debug("Using Udev to detect the right DRM node to use\n");
    struct udev * const __restrict udev = udev_new();
    struct udev_enumerate * const __restrict dev_enumeration =
        udev_enumerate_new(udev);
    
    udev_enumerate_add_match_subsystem(dev_enumeration, "drm");
    udev_enumerate_add_match_sysname(dev_enumeration, "card[0-9]*");
    udev_enumerate_scan_devices(dev_enumeration);
    
    const char * __restrict node_path =
        drm_primary_gpu_device_node(udev, dev_enumeration);

    udev_enumerate_unref(dev_enumeration);
    udev_unref(udev);
    
    return node_path;
}

int
NativeStateDRM::open_using_udev_scan()
{
    char const * __restrict const dev_path =
        udev_main_gpu_drm_node_path();
    
    int fd = -1;
    if (dev_path) {
        Log::debug("Trying to use the DRM node %s\n", dev_path);
        fd = open(dev_path, O_RDWR);
    }
    else {
        Log::error(
            "Can't determine the main graphic card DRM device node\n"
        );
    }
    
    if (!is_valid_fd(fd)) {
        Log::error(
            "Tried to use '%s' but failed.\nReason : %m",
            dev_path
        );
    }
    else Log::debug("Success !\n");
    
    return fd;
}

int
NativeStateDRM::open_using_module_checking()
{
    // TODO: Replace this with something that explicitly probes for the loaded
    // driver (udev?).
    static const char* drm_modules[] = {
        "i915",
        "imx-drm",
        "nouveau",
        "radeon",
        "vmgfx",
        "omapdrm",
        "exynos",
        "pl111",
        "vc4",
    };

    int fd = -1;
    unsigned int num_modules(sizeof(drm_modules)/sizeof(drm_modules[0]));
    for (unsigned int m = 0; m < num_modules; m++) {
        fd = drmOpen(drm_modules[m], 0);
        if (fd < 0) {
            Log::debug("Failed to open DRM module '%s'\n", drm_modules[m]);
            continue;
        }
        Log::debug("Opened DRM module '%s'\n", drm_modules[m]);
        break;
    }
    
    return fd;
}

bool
NativeStateDRM::init()
{

    // TODO: The user should be able to define *exactly* which device
    //       node to open and the program should try to open only
    //       this node. This would mainly be used by software renderer
    //       users (Virtual GEM).
    int fd = open_using_udev_scan();
    
    if (!is_valid_fd(fd)) {
        fd = open_using_module_checking();
    }
    
    if (!is_valid_fd(fd)) {
        Log::error("Failed to find a suitable DRM device\n");
        return false;
    }

    fd_ = fd;
    
    resources_ = drmModeGetResources(fd);
    if (!resources_) {
        Log::error("drmModeGetResources failed\n");
        return false;
    }

    // Find a connected connector
    for (int c = 0; c < resources_->count_connectors; c++) {
        connector_ = drmModeGetConnector(fd, resources_->connectors[c]);
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

    signal(SIGINT, &NativeStateDRM::quit_handler);

    return true;
}

volatile std::sig_atomic_t NativeStateDRM::should_quit_(false);

void
NativeStateDRM::quit_handler(int /*signo*/)
{
    should_quit_ = true;
}

void
NativeStateDRM::page_flip_handler(int/*  fd */, unsigned int /* frame */, unsigned int /* sec */, unsigned int /* usec */, void* data)
{
    unsigned int* waiting = reinterpret_cast<unsigned int*>(data);
    *waiting = 0;
}

void
NativeStateDRM::cleanup()
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
