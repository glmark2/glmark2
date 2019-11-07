/*
 * Copyright © 2010-2011 Linaro Limited
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
 *  Federico Giovanardi
 */
#include "native-state-fb.h"
#include <EGL/eglvivante.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <iostream>
#include "log.h"

bool NativeStateFb::init_display()
{
    theNativeDisplay = fbGetDisplayByIndex(0);
    if( !theNativeDisplay )
        return false;
    fbGetDisplayGeometry(theNativeDisplay,&width,&height);
    theNativeWindow  = fbCreateWindow(theNativeDisplay, 0, 1, width, height);
    return  true;
}
void* NativeStateFb::display()
{
    return theNativeDisplay;
}
bool NativeStateFb::create_window(WindowProperties const& )
{
    return true;
}
void* NativeStateFb::window(WindowProperties& properties)
{
    properties.width = width;
    properties.height = height;
    properties.fullscreen = true;
    properties.visual_id = 0;
    return theNativeWindow;
}
void NativeStateFb::visible(bool )
{
}
bool NativeStateFb::should_quit()
{
    return false;
}
void NativeStateFb::flip()
{
}
void NativeStateFb::cleanup()
{
    fbDestroyWindow(theNativeWindow); theNativeWindow = 0;
    fbDestroyDisplay(theNativeDisplay); theNativeDisplay = 0;
}
