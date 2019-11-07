/*
 * Copyright © 2019 Linaro
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
 *  Federico Giovanardi
 */
#ifndef GLMARK2_NATIVE_STATE_FB_H_
#define GLMARK2_NATIVE_STATE_FB_H_

#include "native-state.h"
#include <csignal>
#include <cstring>

class NativeStateFb : public NativeState
{
public:
    NativeStateFb() :
        theNativeWindow(0),
        theNativeDisplay(0)
    {}
    ~NativeStateFb() { cleanup(); }

    bool init_display();
    void* display();
    bool create_window(WindowProperties const& properties);
    void* window(WindowProperties& properties);
    void visible(bool v);
    bool should_quit();
    void flip();

private:
    void * theNativeWindow;
    void * theNativeDisplay;
    int    width,height;
    void cleanup();
};

#endif /* GLMARK2_NATIVE_STATE_FB_H_ */
