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

class DisplayConfiguration
{
public:
    DisplayConfiguration(MirConnection* connection)
        : display_config(mir_connection_create_display_configuration(connection))
    {
    }

    ~DisplayConfiguration()
    {
        if (display_config)
            mir_display_config_release(display_config);
    }

    bool is_valid()
    {
        return display_config != nullptr;
    }

    MirOutput const* find_active_output() const
    {
        MirOutput const* output = nullptr;

        auto const num_outputs = mir_display_config_get_num_outputs(display_config);

        for (auto i = 0; i < num_outputs; i++)
        {
            auto const out = mir_display_config_get_output(display_config, i);

            if (mir_output_is_enabled(out) &&
                mir_output_get_connection_state(out) == mir_output_connection_state_connected &&
                mir_output_get_current_mode(out) != nullptr)
            {
                output = out;
                break;
            }
        }

        return output;
    }


    operator MirDisplayConfig*() const
    {
        return display_config;
    }

private:
    MirDisplayConfig* const display_config;
};
}

volatile sig_atomic_t NativeStateMir::should_quit_(false);

NativeStateMir::NativeStateMir()
    : mir_connection_{nullptr},
      mir_render_surface_{nullptr},
      mir_window_{nullptr}
{
}

NativeStateMir::~NativeStateMir()
{
    if (mir_window_)
        mir_window_release_sync(mir_window_);
    if (mir_render_surface_)
        mir_render_surface_release(mir_render_surface_);
    if (mir_connection_)
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

    mir_connection_ = mir_connect_sync(NULL, "glmark2");

    if (!mir_connection_is_valid(mir_connection_))
    {
        Log::error("Couldn't connect to the Mir display server\n");
        return false;
    }

    return true;
}

void*
NativeStateMir::display()
{
    if (mir_connection_is_valid(mir_connection_))
        return static_cast<void*>(mir_connection_);

    return nullptr;
}

bool
NativeStateMir::create_window(WindowProperties const& properties)
{
    static const char *win_name("glmark2 " GLMARK_VERSION);

    if (!mir_connection_is_valid(mir_connection_))
    {
        Log::error("Cannot create a Mir window without a valid connection "
                   "to the Mir display server!\n");
        return false;
    }

    /* Recreate an existing window only if it has actually been resized */
    if (mir_window_ && mir_render_surface_)
    {
        if (properties_.fullscreen == properties.fullscreen &&
            (properties.fullscreen == true ||
             (properties_.width == properties.width &&
              properties_.height == properties.height)))
        {
            return true;
        }
    }

    int output_id = -1;

    properties_ = properties;

    if (properties_.fullscreen)
    {
        DisplayConfiguration display_config(mir_connection_);
        if (!display_config.is_valid())
        {
            Log::error("Couldn't get display configuration from the Mir display server!\n");
            return false;
        }

        auto const active_output = display_config.find_active_output();
        if (active_output == nullptr)
        {
            Log::error("Couldn't find an active output in the Mir display server!\n");
            return false;
        }

        auto const current_mode = mir_output_get_current_mode(active_output);

        properties_.width = mir_output_mode_get_width(current_mode);
        properties_.height = mir_output_mode_get_height(current_mode);
        output_id = mir_output_get_id(active_output);

        Log::debug("Making Mir window fullscreen on output %u (%ux%u)\n",
                   output_id, properties_.width, properties_.height);
    }

    if (!mir_render_surface_)
    {
        mir_render_surface_ =
            mir_connection_create_render_surface_sync(mir_connection_,
                                                      properties_.width,
                                                      properties_.height);
        if (!mir_render_surface_ || !mir_render_surface_is_valid(mir_render_surface_))
        {
            Log::error("Failed to create Mir render surface!\n");
            return false;
        }
    }
    else
    {
        mir_render_surface_set_size(mir_render_surface_,
                                    properties_.width,
                                    properties_.height);
    }

    if (!mir_window_)
    {
        auto const spec =
            mir_create_normal_window_spec(mir_connection_,
                                          properties_.width,
                                          properties_.height);

        mir_window_spec_add_render_surface(spec, mir_render_surface_,
                                           properties_.width, properties_.height,
                                           0, 0);
        mir_window_spec_set_name(spec, win_name);
        if (properties_.fullscreen)
            mir_window_spec_set_fullscreen_on_output(spec, output_id);

        mir_window_ = mir_create_window_sync(spec);

        mir_window_spec_release(spec);

        if (!mir_window_ || !mir_window_is_valid(mir_window_))
        {
            Log::error("Failed to create Mir window!\n");
            return false;
        }
    }
    else
    {
        auto const spec = mir_create_window_spec(mir_connection_);

        mir_window_spec_set_width(spec, properties_.width);
        mir_window_spec_set_height(spec, properties_.height);
        if (properties_.fullscreen)
            mir_window_spec_set_fullscreen_on_output(spec, output_id);

        mir_window_apply_spec(mir_window_, spec);

        mir_window_spec_release(spec);
    }

    return true;
}

void*
NativeStateMir::window(WindowProperties& properties)
{
    properties = properties_;

    if (mir_render_surface_)
        return static_cast<void*>(mir_render_surface_);

    return nullptr;
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
