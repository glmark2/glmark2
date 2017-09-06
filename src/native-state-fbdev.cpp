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
 *  Wladimir J. van der Laan
 */
#include "native-state-fbdev.h"
#include "log.h"
#include "util.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#if HAS_MALI
#include <EGL/fbdev_window.h>
#endif
#ifdef ANDROID
#define FBDEV_DEV "/dev/graphics/fb"
#else
#define FBDEV_DEV "/dev/fb"
#endif

/******************
 * Public methods *
 ******************/

bool
NativeStateFBDEV::init_display()
{
    if (fd == -1)
        return init();
    return true;
}

void*
NativeStateFBDEV::display()
{
    return reinterpret_cast<void*>(fd);
}

bool
NativeStateFBDEV::create_window(WindowProperties const& /*properties*/)
{
    struct fb_var_screeninfo fb_var;
    if (fd == -1) {
        Log::error("Error: display has not been initialized!\n");
        return false;
    }
    if (ioctl(fd, FBIOGET_VSCREENINFO, &fb_var))
    {
        Log::error("Error: cannot get variable frame buffer info\n");
        return false;
    }
    winprops.width = fb_var.xres;
    winprops.height = fb_var.yres;
    winprops.fullscreen = true;
    return true;
}

void*
NativeStateFBDEV::window(WindowProperties& properties)
{
    properties = winprops;
#ifdef HAS_MALI
    native_window.height = winprops.height;
    native_window.width = winprops.width;
    return reinterpret_cast<void*>(&native_window);
#else
    return NULL;
#endif
}

void
NativeStateFBDEV::visible(bool /*visible*/)
{
}

bool
NativeStateFBDEV::should_quit()
{
    return should_quit_;
}

void
NativeStateFBDEV::flip()
{
}

/*******************
 * Private methods *
 *******************/

bool
NativeStateFBDEV::init()
{
    std::string devname;
    int num = 0; /* always fb0 for now */

    devname = std::string(FBDEV_DEV) + Util::toString(num);
    fd = open(devname.c_str(), O_RDWR);
    if(fd == -1)
    {
        Log::error("Error: Cannot open framebuffer device %s\n", devname.c_str());
        return false;
    }

    signal(SIGINT, &NativeStateFBDEV::quit_handler);

    return true;
}

volatile std::sig_atomic_t NativeStateFBDEV::should_quit_(false);

void
NativeStateFBDEV::quit_handler(int /*signo*/)
{
    should_quit_ = true;
}

void
NativeStateFBDEV::cleanup()
{
    close(fd);
    fd = -1;
}
