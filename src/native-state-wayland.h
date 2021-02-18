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

#ifndef GLMARK2_NATIVE_STATE_WAYLAND_H_
#define GLMARK2_NATIVE_STATE_WAYLAND_H_

#include <vector>
#include <wayland-client.h>
#include <wayland-egl.h>
#include <wayland-cursor.h>
#include "xdg-shell-client-protocol.h"

#include "native-state.h"

class NativeStateWayland : public NativeState
{
public:
    NativeStateWayland();
    ~NativeStateWayland();

    bool init_display();
    void* display();
    bool create_window(WindowProperties const& properties);
    void* window(WindowProperties& properties);
    void visible(bool v);
    bool should_quit();
    void flip();

private:
    static void quit_handler(int signum);

    static const struct wl_registry_listener registry_listener_;
    static const struct xdg_wm_base_listener xdg_wm_base_listener_;
    static const struct xdg_surface_listener xdg_surface_listener_;
    static const struct xdg_toplevel_listener xdg_toplevel_listener_;
    static const struct wl_output_listener output_listener_;
    static const struct wl_seat_listener seat_listener_;
    static const struct wl_pointer_listener pointer_listener_;
    static const struct wl_keyboard_listener keyboard_listener_;

    static void
    registry_handle_global(void *data, struct wl_registry *registry,
                           uint32_t id, const char *interface,
                           uint32_t version);
    static void
    registry_handle_global_remove(void *data, struct wl_registry *registry,
                                  uint32_t name);

    static void
    output_handle_geometry(void *data, struct wl_output *wl_output,
                           int32_t x, int32_t y, int32_t physical_width,
                           int32_t physical_height, int32_t subpixel,
                           const char *make, const char *model,
                           int32_t transform);

    static void
    output_handle_mode(void *data, struct wl_output *wl_output, uint32_t flags,
             int32_t width, int32_t height, int32_t refresh);

    static void
    output_handle_done(void *data, struct wl_output *wl_output);

    static void
    output_handle_scale(void *data, struct wl_output *wl_output, int32_t factor);

    static void
    xdg_wm_base_handle_ping(void *data, struct xdg_wm_base *xdg_wm_base,
                            uint32_t serial);
    static void
    xdg_toplevel_handle_configure(void *data,
                                  struct xdg_toplevel *xdg_toplevel,
                                  int32_t width, int32_t height,
                                  struct wl_array *states);
    static void
    xdg_toplevel_handle_close(void *data, struct xdg_toplevel *xdg_toplevel);
    static void
    xdg_surface_handle_configure(void *data, struct xdg_surface *xdg_surface,
                                 uint32_t serial);

    static void seat_handle_capabilities(void *data,
                                         struct wl_seat *seat, uint32_t caps);

    static void pointer_handle_enter(void *data, struct wl_pointer *pointer,
                                     uint32_t serial, struct wl_surface *surface,
                                     wl_fixed_t sx, wl_fixed_t sy);

    static void pointer_handle_leave(void *data, struct wl_pointer *pointer,
                                     uint32_t serial, struct wl_surface *surface);

    static void pointer_handle_motion(void *data, struct wl_pointer *pointer,
                                      uint32_t time, wl_fixed_t sx, wl_fixed_t sy);

    static void pointer_handle_button(void *data, struct wl_pointer *wl_pointer,
                                      uint32_t serial, uint32_t time,
                                      uint32_t button, uint32_t state);

    static void pointer_handle_axis(void *data, struct wl_pointer *wl_pointer,
                                    uint32_t time, uint32_t axis, wl_fixed_t value);

    static void keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard,
                                       uint32_t format, int fd, uint32_t size);
    static void keyboard_handle_enter(void *data, struct wl_keyboard *keyboard,
                                      uint32_t serial, struct wl_surface *surface,
                                      struct wl_array *keys);
    static void keyboard_handle_leave(void *data, struct wl_keyboard *keyboard,
                                      uint32_t serial, struct wl_surface *surface);
    static void keyboard_handle_key(void *data, struct wl_keyboard *keyboard,
                                    uint32_t serial, uint32_t time, uint32_t key,
                                    uint32_t state);
    static void keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard,
                                          uint32_t serial, uint32_t mods_depressed,
                                          uint32_t mods_latched, uint32_t mods_locked,
                                          uint32_t group);
    void setup_cursor();

    struct my_output {
        wl_output *output;
        int32_t width, height;
        int32_t refresh;
        int32_t scale;
    };

    typedef std::vector<struct my_output *> OutputsVector;

    struct my_cursor {
        wl_cursor_theme *cursor_theme;
        wl_cursor *default_cursor;
        wl_surface *cursor_surface;
    } *cursor_;

    struct my_display {
        wl_display *display;
        wl_registry *registry;
        wl_compositor *compositor;
        wl_shm *shm;
        wl_seat *seat;
        wl_pointer *pointer;
        wl_keyboard *keyboard;
        struct xdg_wm_base *xdg_wm_base;
        OutputsVector outputs;
    } *display_;

    struct my_window {
        WindowProperties properties;
        bool waiting_for_configure;
        struct wl_surface *surface;
        struct wl_egl_window *native;
        struct xdg_surface *xdg_surface;
        struct xdg_toplevel *xdg_toplevel;
    } *window_;

    static volatile bool should_quit_;
};

#endif /* GLMARK2_NATIVE_STATE_WAYLAND_H_ */
