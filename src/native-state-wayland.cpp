/*
 * Copyright © 2013 Rafal Mielniczuk
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
 *  Rafal Mielniczuk <rafal.mielniczuk2@gmail.com>
 */

#include "native-state-wayland.h"

#include <cstring>
#include <csignal>

const struct wl_registry_listener NativeStateWayland::registry_listener_ = {
    NativeStateWayland::registry_handle_global,
    NativeStateWayland::registry_handle_global_remove
};

const struct xdg_toplevel_listener NativeStateWayland::xdg_toplevel_listener_ = {
    NativeStateWayland::xdg_toplevel_handle_configure,
    NativeStateWayland::xdg_toplevel_handle_close
};

const struct xdg_surface_listener NativeStateWayland::xdg_surface_listener_ = {
    NativeStateWayland::xdg_surface_handle_configure
};

const struct xdg_wm_base_listener NativeStateWayland::xdg_wm_base_listener_ = {
    NativeStateWayland::xdg_wm_base_handle_ping
};

const struct wl_output_listener NativeStateWayland::output_listener_ = {
    NativeStateWayland::output_handle_geometry,
    NativeStateWayland::output_handle_mode,
    NativeStateWayland::output_handle_done,
    NativeStateWayland::output_handle_scale
};

volatile bool NativeStateWayland::should_quit_ = false;

NativeStateWayland::NativeStateWayland() : display_(0), window_(0)
{
}

NativeStateWayland::~NativeStateWayland()
{
    if (window_) {
        if (window_->xdg_toplevel)
            xdg_toplevel_destroy(window_->xdg_toplevel);
        if (window_->xdg_surface)
            xdg_surface_destroy(window_->xdg_surface);
        if (window_->native)
            wl_egl_window_destroy(window_->native);
        if (window_->surface)
            wl_surface_destroy(window_->surface);
        delete window_;
    }

    if (display_) {
        if (display_->xdg_wm_base)
            xdg_wm_base_destroy(display_->xdg_wm_base);

        for (OutputsVector::iterator it = display_->outputs.begin();
             it != display_->outputs.end(); ++it) {

            wl_output_destroy((*it)->output);
            delete *it;
        }
        if (display_->compositor)
            wl_compositor_destroy(display_->compositor);
        if (display_->registry)
            wl_registry_destroy(display_->registry);
        if (display_->display) {
            wl_display_flush(display_->display);
            wl_display_disconnect(display_->display);
        }
        delete display_;
    }
}

void
NativeStateWayland::registry_handle_global(void *data, struct wl_registry *registry,
                                           uint32_t id, const char *interface,
                                           uint32_t version)
{
    NativeStateWayland *that = static_cast<NativeStateWayland *>(data);
    if (strcmp(interface, "wl_compositor") == 0) {
        that->display_->compositor =
                static_cast<struct wl_compositor *>(
                    wl_registry_bind(registry,
                                     id, &wl_compositor_interface, std::min(version, 4U)));
    } else if (strcmp(interface, "xdg_wm_base") == 0) {
        that->display_->xdg_wm_base =
                static_cast<struct xdg_wm_base *>(
                    wl_registry_bind(registry,
                                     id, &xdg_wm_base_interface, std::min(version, 2U)));
        xdg_wm_base_add_listener(that->display_->xdg_wm_base, &xdg_wm_base_listener_, that);
    } else if (strcmp(interface, "wl_output") == 0) {
        struct my_output *my_output = new struct my_output();
        memset(my_output, 0, sizeof(*my_output));
        my_output->scale = 1;
        my_output->output =
                static_cast<struct wl_output *>(
                    wl_registry_bind(registry,
                                     id, &wl_output_interface, std::min(version, 2U)));
        that->display_->outputs.push_back(my_output);

        wl_output_add_listener(my_output->output, &output_listener_, my_output);
        wl_display_roundtrip(that->display_->display);
    }
}

void
NativeStateWayland::registry_handle_global_remove(void * /*data*/,
                                                  struct wl_registry * /*registry*/,
                                                  uint32_t /*name*/)
{
}

void
NativeStateWayland::output_handle_geometry(void * /*data*/, struct wl_output * /*wl_output*/,
                                           int32_t /*x*/, int32_t /*y*/, int32_t /*physical_width*/,
                                           int32_t /*physical_height*/, int32_t /*subpixel*/,
                                           const char * /*make*/, const char * /*model*/,
                                           int32_t /*transform*/)
{
}

void
NativeStateWayland::output_handle_mode(void *data, struct wl_output * /*wl_output*/,
                                       uint32_t flags, int32_t width, int32_t height,
                                       int32_t refresh)
{
    /* Only handle output mode events for the shell's "current" mode */
    if (flags & WL_OUTPUT_MODE_CURRENT) {
        struct my_output *my_output = static_cast<struct my_output *>(data);

        my_output->width = width;
        my_output->height = height;
        my_output->refresh = refresh;
    }
}

void
NativeStateWayland::output_handle_done(void * /*data*/, struct wl_output * /*wl_output*/)
{
}

void
NativeStateWayland::output_handle_scale(void *data, struct wl_output * /*wl_output*/,
                                        int32_t factor)
{
    struct my_output *my_output = static_cast<struct my_output *>(data);
    my_output->scale = factor;
}

void
NativeStateWayland::xdg_wm_base_handle_ping(void * /*data*/, struct xdg_wm_base *xdg_wm_base,
                                            uint32_t serial)
{
    xdg_wm_base_pong(xdg_wm_base, serial);
}

void
NativeStateWayland::xdg_toplevel_handle_configure(void *data, struct xdg_toplevel * /*xdg_toplevel*/,
                                                  int32_t width, int32_t height,
                                                  struct wl_array *states)
{
    NativeStateWayland *that = static_cast<NativeStateWayland *>(data);
    uint32_t *state;
    bool want_fullscreen = false;
    uint32_t scale = 1;

    that->window_->waiting_for_configure = false;

    if (!that->display_->outputs.empty()) scale = that->display_->outputs.at(0)->scale;

    for (state = (uint32_t *) states->data;
         state < (uint32_t *) ((char *) states->data + states->size);
         state++) {
        if (*state == XDG_TOPLEVEL_STATE_FULLSCREEN)
            want_fullscreen = true;
    }

    if (want_fullscreen) {
        width *= scale;
        height *= scale;
    }

    if (want_fullscreen == that->window_->properties.fullscreen &&
        (width == 0 || width == that->window_->properties.width) &&
        (height == 0 || height == that->window_->properties.fullscreen)) {
        return;
    }

    that->window_->properties.fullscreen = want_fullscreen;
    that->window_->properties.width = width;
    that->window_->properties.height = height;

    if (!that->window_->native)
        return;

    wl_egl_window_resize(that->window_->native, width, height, 0, 0);

    struct wl_region *opaque_reqion = wl_compositor_create_region(that->display_->compositor);
    wl_region_add(opaque_reqion, 0, 0,
                  that->window_->properties.width,
                  that->window_->properties.height);
    wl_surface_set_opaque_region(that->window_->surface, opaque_reqion);
    wl_region_destroy(opaque_reqion);
}

void
NativeStateWayland::xdg_toplevel_handle_close(void *data,
                                              struct xdg_toplevel * /*xdg_toplevel */)
{
    NativeStateWayland *that = static_cast<NativeStateWayland *>(data);

    that->should_quit_ = true;
}

void
NativeStateWayland::xdg_surface_handle_configure(void * /*data*/,
                                                 struct xdg_surface *xdg_surface,
                                                 uint32_t serial)
{
    xdg_surface_ack_configure(xdg_surface, serial);
}

bool
NativeStateWayland::init_display()
{
    struct sigaction sa;
    sa.sa_handler = &NativeStateWayland::quit_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    display_ = new struct my_display();

    if (!display_) {
        return false;
    }

    display_->display = wl_display_connect(NULL);

    if (!display_->display) {
        return false;
    }

    display_->registry = wl_display_get_registry(display_->display);

    wl_registry_add_listener(display_->registry, &registry_listener_, this);

    wl_display_roundtrip(display_->display);

    return true;
}

void*
NativeStateWayland::display()
{
    return static_cast<void *>(display_->display);
}

bool
NativeStateWayland::create_window(WindowProperties const& properties)
{
    struct my_output *output = 0;
    if (!display_->outputs.empty()) output = display_->outputs.at(0);
    window_ = new struct my_window();
    window_->properties = properties;

    window_->surface = wl_compositor_create_surface(display_->compositor);
    window_->xdg_surface = xdg_wm_base_get_xdg_surface(display_->xdg_wm_base,
                                                       window_->surface);
    xdg_surface_add_listener(window_->xdg_surface, &xdg_surface_listener_, this);
    window_->xdg_toplevel = xdg_surface_get_toplevel(window_->xdg_surface);
    xdg_toplevel_add_listener(window_->xdg_toplevel, &xdg_toplevel_listener_, this);

    xdg_toplevel_set_app_id(window_->xdg_toplevel, "com.github.glmark2.glmark2");
    xdg_toplevel_set_title(window_->xdg_toplevel, "glmark2");
    if (window_->properties.fullscreen && output)
        xdg_toplevel_set_fullscreen(window_->xdg_toplevel, output->output);
    wl_surface_commit(window_->surface);
    window_->waiting_for_configure = true;

    /* xdg-shell requires us to wait for the compositor to tell us what its
     * desired size for us is */
    while (window_->waiting_for_configure)
        wl_display_roundtrip(display_->display);

    if (output && window_->properties.fullscreen) {
        if (window_->properties.width <= 0 ||
             window_->properties.height <= 0) {
            window_->properties.width = output->width * output->scale;
            window_->properties.height = output->height * output->scale;
        }

        if (wl_proxy_get_version((struct wl_proxy *) output) >=
            WL_OUTPUT_SCALE_SINCE_VERSION) {
            wl_surface_set_buffer_scale(window_->surface, output->scale);
        }
    }

    window_->native = wl_egl_window_create(window_->surface,
                                           window_->properties.width,
                                           window_->properties.height);

    struct wl_region *opaque_reqion = wl_compositor_create_region(display_->compositor);
    wl_region_add(opaque_reqion, 0, 0,
                  window_->properties.width,
                  window_->properties.height);
    wl_surface_set_opaque_region(window_->surface, opaque_reqion);
    wl_region_destroy(opaque_reqion);

    return true;
}

void*
NativeStateWayland::window(WindowProperties &properties)
{
    if (window_) {
        properties = window_->properties;
        return window_->native;
    }

    return 0;
}

void
NativeStateWayland::visible(bool /*v*/)
{
}

bool
NativeStateWayland::should_quit()
{
    return should_quit_;
}

void
NativeStateWayland::flip()
{
    int ret = wl_display_roundtrip(display_->display);
    should_quit_ = (ret == -1) || should_quit_;
}

void
NativeStateWayland::quit_handler(int /*signum*/)
{
    should_quit_ = true;
}

