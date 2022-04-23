/*
 * Copyright © 2022 Erico Nunes
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

#include "native-state-gbm.h"
#include "log.h"

#include <fcntl.h>
#include <cstring>

bool
NativeStateGBM::init_display()
{
    if (dev_)
        return true;

    // TODO: The user should be able to define which device node to open.
    int fd = open("/dev/dri/renderD128", O_RDWR);
    if (fd < 0) {
        Log::error("Failed to find a suitable GBM device\n");
        return false;
    }

    fd_ = fd;

    dev_ = gbm_create_device(fd_);
    if (!dev_) {
        Log::error("Failed to create GBM device\n");
        return false;
    }

    signal(SIGINT, &NativeStateGBM::quit_handler);

    return (dev_ != 0);
}

void*
NativeStateGBM::display()
{
    return static_cast<void*>(dev_);
}

bool
NativeStateGBM::create_window(WindowProperties const& properties)
{
    if (!dev_) {
        Log::error("GBM device has not been initialized\n");
        return false;
    }

    if (properties.fullscreen) {
        Log::error("Fullscreen not supported\n");
        return false;
    }

    if (properties.width <= 0 || properties.height <= 0) {
        Log::error("Invalid surface size\n");
        return false;
    }

    width = properties.width;
    height = properties.height;

    surface_ = gbm_surface_create(dev_, width, height,
                                  properties.visual_id,
                                  GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
    if (!surface_) {
        Log::error("Failed to create GBM surface\n");
        return false;
    }

    return true;
}

void*
NativeStateGBM::window(WindowProperties& properties)
{
    properties = WindowProperties(width, height, true, 0);
    return static_cast<void*>(surface_);
}

void
NativeStateGBM::visible(bool v)
{
}

volatile std::sig_atomic_t NativeStateGBM::should_quit_(false);

void
NativeStateGBM::quit_handler(int /*signo*/)
{
    should_quit_ = true;
}

bool
NativeStateGBM::should_quit()
{
    return should_quit_;
}

void
NativeStateGBM::flip()
{
}

void
NativeStateGBM::cleanup()
{
    if (surface_) {
        gbm_surface_destroy(surface_);
        surface_ = 0;
    }
    if (dev_) {
        gbm_device_destroy(dev_);
        dev_ = 0;
    }
    fd_ = 0;
}
