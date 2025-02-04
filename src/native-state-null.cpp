/*
 * Copyright © 2025 Collabora Ltd
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
 */

#include "native-state-null.h"
#include "log.h"

bool
NativeStateNull::init_display()
{
    signal(SIGINT, &NativeStateNull::quit_handler);
    return true;
}

void*
NativeStateNull::display()
{
    return nullptr;
}

bool
NativeStateNull::create_window(WindowProperties const& properties)
{
    if (properties.fullscreen) {
        Log::error("Fullscreen not supported\n");
        return false;
    }

    if (properties.width <= 0 || properties.height <= 0) {
        Log::error("Invalid surface size\n");
        return false;
    }
    properties_ = properties;

    return true;
}

void*
NativeStateNull::window(WindowProperties& properties)
{
    properties = properties_;
    return nullptr;
}

void
NativeStateNull::visible(bool v)
{
}

volatile std::sig_atomic_t NativeStateNull::should_quit_(false);

void
NativeStateNull::quit_handler(int /*signo*/)
{
    should_quit_ = true;
}

bool
NativeStateNull::should_quit()
{
    return should_quit_;
}

void
NativeStateNull::flip()
{
}
