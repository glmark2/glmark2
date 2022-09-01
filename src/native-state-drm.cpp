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
#include "options.h"

#include <errno.h>
#include <fcntl.h>
#include <libudev.h>
#include <cstring>
#include <string>

/******************
 * Public methods *
 ******************/

namespace
{

std::string const drm_device_opt{"drm-device"};

std::string get_drm_device_option()
{
    std::string drm_device{""};

    for (auto const& opt : Options::winsys_options)
    {
        if (opt.name == drm_device_opt)
            drm_device = opt.value;
    }

    return drm_device;
}

}

NativeStateDRM::NativeStateDRM() :
    fd_(0),
    resources_(0),
    connector_(0),
    encoder_(0),
    crtc_(0),
    mode_(0),
    dev_(0),
    surface_(0),
    pending_bo_(0),
    flipped_bo_(0),
    presented_bo_(0),
    crtc_set_(false),
    use_async_flip_(false)
{
    Options::winsys_options_help =
       "  drm-device=DRM-DEVICE  The DRM device to use (autodetected if unspecified)\n";
}

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
NativeStateDRM::create_window(WindowProperties const& properties)
{
    if (!dev_) {
        Log::error("Error: DRM device has not been initialized!\n");
        return false;
    }

    /* egl config's native visual id is drm fourcc */
    surface_ = gbm_surface_create(dev_, mode_->hdisplay, mode_->vdisplay,
                                  properties.visual_id,
                                  GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
    if (!surface_) {
        Log::error("Failed to create GBM surface\n");
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
    if (!crtc_set_ && drmSetMaster(fd_) < 0) {
        Log::error("Failed to become DRM master "
                   "(hint: glmark2-drm needs to be run in a VT)\n");
        should_quit_ = true;
        return;
    }

    if (pending_bo_)
        gbm_surface_release_buffer(surface_, pending_bo_);
    pending_bo_ = gbm_surface_lock_front_buffer(surface_);

    DRMFBState* pending_fb = fb_get_from_bo(pending_bo_);

    if (!pending_bo_ || !pending_fb) {
        Log::error("Failed to get gbm front buffer\n");
        return;
    }

    if (Options::swap_mode == Options::SwapModeFIFO || use_async_flip_)
    {
        /* When using either FIFO mode (vsync) or an async flip, wait for the
         * current flip to finish. */
        while (flipped_bo_ && check_for_page_flip(-1) >= 0)
            continue;
    }
    else
    {
        /* When not using async flips, i.e., mailbox-like presentation,
         * and the flip is still not done, we can continue potentially
         * without flipping. */
        check_for_page_flip(0);
    }

    /* If a flip is not in progress we can schedule another one. */
    if (!flipped_bo_) {
        if (!crtc_set_) {
            int status = drmModeSetCrtc(fd_, encoder_->crtc_id, pending_fb->fb_id, 0, 0,
                                        &connector_->connector_id, 1, mode_);
            if (status >= 0) {
                crtc_set_ = true;
                if (presented_bo_)
                    gbm_surface_release_buffer(surface_, presented_bo_);
                presented_bo_ = pending_bo_;
                flipped_bo_ = nullptr;
                pending_bo_ = nullptr;
            }
            else {
                Log::error("Failed to set crtc: %d\n", status);
            }
            return;
        }

        uint32_t flip_flags = DRM_MODE_PAGE_FLIP_EVENT;
        if (use_async_flip_)
            flip_flags |= DRM_MODE_PAGE_FLIP_ASYNC;

        int status = drmModePageFlip(fd_, encoder_->crtc_id, pending_fb->fb_id,
                                     flip_flags, this);
        if (status < 0) {
            Log::error("Failed to enqueue page flip: %d\n", status);
            return;
        }

        flipped_bo_ = pending_bo_;
        pending_bo_ = nullptr;
    }

    /* We need to ensure our surface has a free buffer, otherwise GL will
     * have no buffer to render on. */
    while (!gbm_surface_has_free_buffers(surface_) &&
           check_for_page_flip(-1) >= 0)
    {
        continue;
    }
}

/*******************
 * Private methods *
 *******************/

/* Simple helpers */

inline static bool valid_fd(int fd)
{
    return fd >= 0;
}

inline static bool valid_drm_node_path(std::string const& provided_node_path)
{
    return !provided_node_path.empty();
}

inline static bool invalid_drm_node_path(std::string const& provided_node_path)
{
    return !(valid_drm_node_path(provided_node_path));
}

/* Udev methods */
// Udev detection functions
#define UDEV_TEST_FUNC_SIGNATURE(udev_identifier, device_identifier, syspath_identifier) \
    struct udev * __restrict const udev_identifier, \
    struct udev_device * __restrict const device_identifier, \
    char const * __restrict syspath_identifier

/* Omitting the parameter names is kind of ugly but is the only way
 * to force G++ to forget about the unused parameters.
 * Having big warnings during the compilation isn't very nice.
 *
 * These functions will be used as function pointers and should have
 * the same signature to avoid weird stack related issues.
 *
 * Another way to deal with that issue will be to mark unused parameters
 * with __attribute__((unused))
 */
static bool udev_drm_test_virtual(
    UDEV_TEST_FUNC_SIGNATURE(,,tested_node_syspath))
{
    return strstr(tested_node_syspath, "virtual") != NULL;
}

static bool udev_drm_test_not_virtual(
    UDEV_TEST_FUNC_SIGNATURE(udev, current_device, tested_node_syspath))
{
    return !udev_drm_test_virtual(udev,
                                  current_device,
                                  tested_node_syspath);
}

static bool
udev_drm_test_primary_gpu(UDEV_TEST_FUNC_SIGNATURE(, current_device,))
{
    bool is_main_gpu = false;

    auto const drm_node_parent = udev_device_get_parent(current_device);

    /* While tempting, using udev_device_unref will generate issues
     * when unreferencing the child in udev_get_node_that_pass_in_enum
     *
     * udev_device_unref WILL unreference the parent, so avoid doing
     * that here.
     *
     * ( See udev sources : src/libudev/libudev-device.c )
     */
    if (drm_node_parent != NULL) {
        is_main_gpu =
            (udev_device_get_sysattr_value(drm_node_parent, "boot_vga")
             != NULL);
    }

    return is_main_gpu;
}

/* Test if the drm-device is actually modeset capable.
 * Render-only devices cannot drive an actual display,
 * so the GETRESOURCES ioctl will fail in that case.
 */
static bool udev_drm_test_modeset(std::string const& dev_path)
{
    struct drm_mode_card_res res {};
    int fd, ret;

    fd = open(dev_path.c_str(), O_RDWR);
    if (!valid_fd(fd))
        return false;

    ret = drmIoctl(fd, DRM_IOCTL_MODE_GETRESOURCES, &res);
    drmClose(fd);

    return !ret;
}

static std::string
udev_get_node_that_pass_in_enum(
    struct udev * __restrict const udev,
    struct udev_enumerate * __restrict const dev_enum,
    bool (* check_function)(UDEV_TEST_FUNC_SIGNATURE(,,)))
{
    std::string result;

    auto current_element = udev_enumerate_get_list_entry(dev_enum);

    while (current_element && result.empty()) {
        char const * __restrict current_element_sys_path =
            udev_list_entry_get_name(current_element);

        if (current_element_sys_path) {
            struct udev_device * current_device =
                udev_device_new_from_syspath(udev,
                                             current_element_sys_path);
            auto check_passed = check_function(
                udev, current_device, current_element_sys_path);

            if (check_passed) {
                const char * device_node_path =
                    udev_device_get_devnode(current_device);

                if (device_node_path &&
                    udev_drm_test_modeset(device_node_path)) {
                        result = device_node_path;
                }

            }

            udev_device_unref(current_device);
        }

        current_element = udev_list_entry_get_next(current_element);
    }

    return result;
}

/* Inspired by KWin detection mechanism */
/* And yet KWin got it wrong too, it seems.
 * 1 - Looking for the primary GPU by checking the flag 'boot_vga'
 *     won't get you far with some embedded chipsets, like Rockchip.
 * 2 - Looking for a GPU plugged in PCI will fail on various embedded
 *     devices !
 * 3 - Looking for a render node is not guaranteed to work on some
 *     poorly maintained DRM drivers, which plague some embedded
 *     devices too...
 *
 * So, we won't play too smart here.
 * - We first check for a primary GPU plugged in PCI with the 'boot_vga'
 *   attribute, to take care of Desktop users using multiple GPU.
 * - Then, we just check for a DRM node that is not virtual
 * - At least, we use the first virtual node we get, if we didn't find
 *   anything yet.
 * This should take care of almost every potential use case.
 *
 * The remaining ones will be dealt with an additional option to
 * specify the DRM dev node manually.
 */
static std::string udev_main_gpu_drm_node_path()
{
    Log::debug("Using Udev to detect the right DRM node to use\n");
    auto udev = udev_new();
    auto dev_enumeration = udev_enumerate_new(udev);

    udev_enumerate_add_match_subsystem(dev_enumeration, "drm");
    udev_enumerate_add_match_sysname(dev_enumeration, "card[0-9]*");
    udev_enumerate_scan_devices(dev_enumeration);

    Log::debug("Looking for the main GPU DRM node...\n");
    std::string node_path = udev_get_node_that_pass_in_enum(
        udev, dev_enumeration, udev_drm_test_primary_gpu);

    if (invalid_drm_node_path(node_path)) {
        Log::debug("Not found!\n");
        Log::debug("Looking for a concrete GPU DRM node...\n");
        node_path = udev_get_node_that_pass_in_enum(
            udev, dev_enumeration, udev_drm_test_not_virtual);
    }
    if (invalid_drm_node_path(node_path)) {
        Log::debug("Not found!?\n");
        Log::debug("Looking for a virtual GPU DRM node...\n");
        node_path = udev_get_node_that_pass_in_enum(
            udev, dev_enumeration, udev_drm_test_virtual);
    }
    if (invalid_drm_node_path(node_path)) {
        Log::debug("Not found.\n");
        Log::debug("Cannot find a single DRM node using UDEV...\n");
    }

    if (valid_drm_node_path(node_path)) {
        Log::debug("Success!\n");
    }

    udev_enumerate_unref(dev_enumeration);
    udev_unref(udev);

    return node_path;
}

static int open_using_udev_scan()
{
    auto dev_path = udev_main_gpu_drm_node_path();

    int fd = -1;
    if (valid_drm_node_path(dev_path)) {
        Log::debug("Trying to use the DRM node %s\n", dev_path.c_str());
        fd = open(dev_path.c_str(), O_RDWR);
    }
    else {
        Log::error("Can't determine the main graphic card "
                   "DRM device node\n");
    }

    if (!valid_fd(fd)) {
        // %m is GLIBC specific... Maybe use strerror here...
        Log::error("Tried to use '%s' but failed.\nReason : %m\n",
                   dev_path.c_str());
    }
    else
        Log::debug("Success!\n");

    return fd;
}

/* -- End of Udev helpers -- */

/*
 * This method is there to take care of cases that would not be handled
 * by open_using_udev_scan. This should not happen.
 *
 * If your driver defines a /dev/dri/cardX node and open_using_udev_scan
 * were not able to detect it, you should probably file an issue.
 *
 */
static int open_using_module_checking()
{
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
        "msm",
        "meson",
        "rockchip",
        "sun4i-drm",
        "stm",
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
    if (!bo) {
        return NULL;
    }

    DRMFBState* fb = reinterpret_cast<DRMFBState*>(gbm_bo_get_user_data(bo));
    if (fb) {
        return fb;
    }

    unsigned int width = gbm_bo_get_width(bo);
    unsigned int height = gbm_bo_get_height(bo);
    unsigned int handles[4] = { 0, }, strides[4] = { 0, }, offsets[4] = { 0, };
    unsigned int format = gbm_bo_get_format(bo);
    unsigned int fb_id(0);
    int status;

#ifdef GBM_HAS_PLANES
    int i;
    for (i = 0; i < gbm_bo_get_plane_count(bo); i++) {
        handles[i] = gbm_bo_get_handle_for_plane(bo, i).u32;
        strides[i] = gbm_bo_get_stride_for_plane(bo, i);
        offsets[i] = gbm_bo_get_offset(bo, i);
    }
#else
    handles[0] = gbm_bo_get_handle(bo).u32;
    strides[0] = gbm_bo_get_stride(bo);
#endif

    status = drmModeAddFB2(fd_, width, height, format, handles,
                           strides, offsets, &fb_id, 0);
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

    return true;
}

bool
NativeStateDRM::init()
{
    int fd = -1;
    std::string drm_device = get_drm_device_option();

    if (valid_drm_node_path(drm_device)) {
        Log::debug("Trying to use DRM node %s from --winsys-options\n",
                   drm_device.c_str());
        fd = open(drm_device.c_str(), O_RDWR);
        if (!valid_fd(fd)) {
            Log::error("Tried to use '%s' but failed.\nReason : %s\n",
                       drm_device.c_str(), strerror(errno));
            return false;
        }
    }

    if (!valid_fd(fd)) {
        fd = open_using_udev_scan();
    }

    if (!valid_fd(fd)) {
        fd = open_using_module_checking();
    }

    if (!valid_fd(fd)) {
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
        bool found = false;
        encoder_ = drmModeGetEncoder(fd_, resources_->encoders[e]);
        for (int ce = 0; ce < connector_->count_encoders; ce++) {
            if (encoder_ && encoder_->encoder_id == connector_->encoders[ce]) {
                found = true;
                break;
            }
        }
        if (found)
            break;
        drmModeFreeEncoder(encoder_);
        encoder_ = 0;
    }

    // If encoder is not connected to the connector try to find
    // a suitable one
    if (!encoder_) {
        for (int e = 0; e < connector_->count_encoders; e++) {
            encoder_ = drmModeGetEncoder(fd, connector_->encoders[e]);
            for (int c = 0; c < resources_->count_crtcs; c++) {
                if (encoder_->possible_crtcs & (1 << c)) {
                    encoder_->crtc_id = resources_->crtcs[c];
                    break;
                }
            }
            if (encoder_->crtc_id) {
                break;
            }

            drmModeFreeEncoder(encoder_);
            encoder_ = 0;
        }
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
        // if there is no current CRTC, make sure to attach a suitable one
        for (int c = 0; c < resources_->count_crtcs; c++) {
            if (encoder_->possible_crtcs & (1 << c)) {
                encoder_->crtc_id = resources_->crtcs[c];
                break;
            }
        }
    }

    uint64_t cap_async;
    if (Options::swap_mode == Options::SwapModeDefault ||
        Options::swap_mode == Options::SwapModeImmediate) {
        use_async_flip_ = drmGetCap(fd_, DRM_CAP_ASYNC_PAGE_FLIP, &cap_async) == 0 &&
                          cap_async == 1;
        if (!use_async_flip_) {
            Log::info("Warning: DRM_CAP_ASYNC_PAGE_FLIP not supported, falling"
                      " back to 'mailbox' mode for SwapInterval(0).\n");
        }
    } else {
        use_async_flip_ = false;
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
    NativeStateDRM* state = reinterpret_cast<NativeStateDRM*>(data);
    if (state->presented_bo_)
        gbm_surface_release_buffer(state->surface_, state->presented_bo_);
    state->presented_bo_ = state->flipped_bo_;
    state->flipped_bo_ = nullptr;
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

int
NativeStateDRM::check_for_page_flip(int timeout_ms)
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd_, &fds);

    drmEventContext evCtx;
    memset(&evCtx, 0, sizeof(evCtx));
    evCtx.version = 2;
    evCtx.page_flip_handler = page_flip_handler;

    struct timeval timeout = {0, timeout_ms * 1000};
    int status = select(fd_ + 1, &fds, 0, 0,
                        timeout_ms < 0 ? nullptr : &timeout);
    if (status == 1)
        drmHandleEvent(fd_, &evCtx);

    return status;
}
