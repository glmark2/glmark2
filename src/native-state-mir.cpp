/*
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
 *  Alexandros Frantzis
 */
#include "native-state-mir.h"
#include "log.h"

/******************
 * Public methods *
 ******************/

namespace
{

void
connected_callback(MirConnection *connection, void *client_context)
{
    MirConnection **con = static_cast<MirConnection**>(client_context);
    *con = connection;
}

void
surface_created_callback(MirSurface *surface, void *client_context)
{
    MirSurface **surf = static_cast<MirSurface**>(client_context);
    *surf = surface;
}

void
null_surface_callback(MirSurface * /*obj*/, void * /*client_context*/)
{
}

}

volatile sig_atomic_t NativeStateMir::should_quit_(false);

NativeStateMir::~NativeStateMir()
{
    if (mir_surface_)
        mir_wait_for(mir_surface_release(mir_surface_, null_surface_callback, 0));
    if (mir_connection_is_valid(mir_connection_))
        mir_connection_release(mir_connection_);
}

bool
NativeStateMir::init_display()
{
    struct sigaction sa;
    sa.sa_handler = &NativeStateMir::quit_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    mir_wait_for(mir_connect("/tmp/mir_socket", "glmark2",
                             connected_callback, &mir_connection_));

    if (!mir_connection_is_valid(mir_connection_)) {
        Log::error("Failed to connect to mir\n");
        return false;
    }

    return true;
}

void*
NativeStateMir::display()
{
    if (mir_connection_is_valid(mir_connection_))
        return static_cast<void*>(mir_connection_get_egl_native_display(mir_connection_));

    return 0;
}

bool
NativeStateMir::create_window(WindowProperties const& properties)
{
    static const char *win_name("glmark2 "GLMARK_VERSION);

    if (!mir_connection_is_valid(mir_connection_)) {
        Log::error("No connection to mir!\n");
        return false;
    }

    /* Recreate an existing window only if it has actually been resized */
    if (mir_surface_) {
        if (properties_.fullscreen != properties.fullscreen ||
            (properties.fullscreen == false &&
             (properties_.width != properties.width ||
              properties_.height != properties.height)))
        {
            mir_wait_for(mir_surface_release(mir_surface_, null_surface_callback, 0));
            mir_surface_ = 0;
        }
        else
        {
            return true;
        }
    }

    MirDisplayInfo display_info;
    mir_connection_get_display_info(mir_connection_, &display_info);

    properties_ = properties;

    if (properties_.fullscreen) {
        properties_.width = display_info.width;
        properties_.height = display_info.height;
    }

    MirSurfaceParameters surface_parameters = {
        win_name,
        properties_.width, properties_.height,
        display_info.supported_pixel_format[0],
        mir_buffer_usage_hardware
    };

    mir_wait_for(mir_surface_create(mir_connection_, &surface_parameters,
                                    surface_created_callback, &mir_surface_));

    if (!mir_surface_) {
        Log::error("Failed to create mir surface!\n");
        return false;
    }

    return true;
}

void*
NativeStateMir::window(WindowProperties& properties)
{
    properties = properties_;

    if (mir_surface_)
        return static_cast<void*>(mir_surface_get_egl_native_window(mir_surface_));

    return 0;
}

void
NativeStateMir::visible(bool /*visible*/)
{
}

bool
NativeStateMir::should_quit()
{
    return should_quit_;
}

/*******************
 * Private methods *
 *******************/

void
NativeStateMir::quit_handler(int /*signum*/)
{
    should_quit_ = true;
}
