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
#ifndef GLMARK2_NATIVE_STATE_FBDEV_H_
#define GLMARK2_NATIVE_STATE_FBDEV_H_

#include "native-state.h"
#include <csignal>
#include <cstring>

#ifdef HAS_MALI
#include <EGL/fbdev_window.h>
#endif

class NativeStateFBDEV : public NativeState
{
public:
    NativeStateFBDEV() :
        fd(-1) {}
    ~NativeStateFBDEV() { cleanup(); }

    bool init_display();
    void* display();
    bool create_window(WindowProperties const& properties);
    void* window(WindowProperties& properties);
    void visible(bool v);
    bool should_quit();
    void flip();

private:
    static void quit_handler(int signum);
    static volatile std::sig_atomic_t should_quit_;
    int fd;
    WindowProperties winprops;
#ifdef HAS_MALI
    struct fbdev_window native_window;
#endif
    bool init();
    void cleanup();
};

#endif /* GLMARK2_NATIVE_STATE_FBDEV_H_ */
