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
 *  Alexandros Frantzis
 */

/* Changes
 * Dec 19, 2017: Leo Xu: add native state support on QNX platform.
 *
 */

#include <sys/keycodes.h>
#include <EGL/egl.h>
#include "native-state-qnx.h"
#include "log.h"

NativeStateQnx::NativeStateQnx() :
	screen_ctx_(nullptr),
	screen_win_(nullptr),
	screen_ev_(nullptr),
	win_visible_(1)
{
}

NativeStateQnx::~NativeStateQnx()
{
	destroy_window();
	if (screen_ev_) {
		screen_destroy_event(screen_ev_);
	}
	if(screen_ctx_){
		screen_destroy_context(screen_ctx_);
	}
}

bool
NativeStateQnx::init_display()
{
	int rc;
	rc = screen_create_context(&screen_ctx_, 0);
	if (rc) {
		Log::error("screen_create_context failed, rc=%d", rc);
		return false;
	}

	rc = screen_create_event(&screen_ev_);
	if (rc) {
		Log::error("screen_create_event failed, rc=%d", rc);
		return false;
	}

	return true;
}

void*
NativeStateQnx::display()
{
	return (void *)EGL_DEFAULT_DISPLAY;
}

bool
NativeStateQnx::create_window(WindowProperties const& properties)
{
	int rc;
	static const char *win_name("glmark2 " GLMARK_VERSION);

	destroy_window();

	rc = screen_create_window(&screen_win_, screen_ctx_);
	if (rc) {
		Log::error("screen_create_window failed, rc=%d", rc);
		return false;
	}

	rc = screen_set_window_property_cv(screen_win_,
				SCREEN_PROPERTY_ID_STRING, strlen(win_name), win_name);
	if (rc) {
		Log::error("screen_set_window_property_cv(SCREEN_PROPERTY_ID_STRING) failed, rc=%d", rc);
		return false;
	}

	int size[2] = { properties.width, properties.height };
	if (!properties.fullscreen && size[0] > 0 && size[1] > 0) {
		rc = screen_set_window_property_iv(screen_win_, SCREEN_PROPERTY_SIZE, size);
		if (rc) {
			Log::error("screen_set_window_property_iv(SCREEN_PROPERTY_SIZE) failed. rc=%d, size={%d,%d}",
					rc, size[0], size[1]);
			return false;
		}
		win_properties_.fullscreen = false;
	} else {
		win_properties_.fullscreen = true;
	}

	int usage = SCREEN_USAGE_OPENGL_ES2;
	rc = screen_set_window_property_iv(screen_win_, SCREEN_PROPERTY_USAGE, &usage);
	if (rc) {
		Log::error("screen_set_window_property_iv(SCREEN_PROPERTY_USAGE) failed. rc=%d", rc);
		return false;
	}

	int format = properties.visual_id;
	switch(format) {
	case SCREEN_FORMAT_RGBA8888:
	case SCREEN_FORMAT_RGBX8888:
	case SCREEN_FORMAT_RGB565:
		win_properties_.visual_id = format;
		break;
	default:
		Log::error("unsupported native visual_id. %d", properties.visual_id);
		return false;
	}

	rc = screen_set_window_property_iv(screen_win_, SCREEN_PROPERTY_FORMAT, &format);
	if (rc) {
		Log::error("screen_set_window_property_iv(SCREEN_PROPERTY_FORMAT), rc=%d", rc);
		return false;
	}

    Log::debug("Creating Window buffers W: %d H: %d FMT: %d\n",
               properties.width, properties.height, format);

	int nbuffers = 2;
	rc = screen_create_window_buffers(screen_win_, nbuffers);
	if (rc) {
		Log::error("screen_create_window_buffers failed, rc=%d", rc);
		return false;
	}

	if (win_properties_.fullscreen) {
		rc = screen_get_window_property_iv(screen_win_, SCREEN_PROPERTY_SIZE, size);
		if (rc) {
			Log::error("screen_get_window_property_iv(SCREEN_PROPERTY_SIZE)  failed, rc=%d", rc);
			return false;
		}
	}
	win_properties_.width  = size[0];
	win_properties_.height = size[1];

	rc = screen_set_window_property_iv(screen_win_, SCREEN_PROPERTY_VISIBLE, &win_visible_);
	if (rc) {
		Log::error("screen_set_window_property_iv(SCREEN_PROPERTY_VISIBLE)  failed, rc=%d", rc);
	}

	return true;
}

void*
NativeStateQnx::window(WindowProperties& properties)
{
	properties = win_properties_;
	return (void *)screen_win_;
}

void
NativeStateQnx::visible(bool visible)
{
	win_visible_ = visible ? 1 : 0;
}

bool
NativeStateQnx::should_quit()
{
	int rc, val;
	if(!screen_get_event(screen_ctx_, screen_ev_, 0)) {
		rc = screen_get_event_property_iv(screen_ev_, SCREEN_PROPERTY_TYPE, &val);
		if (rc || val == SCREEN_EVENT_NONE) {
			return false;
		}
		switch (val) {
		case SCREEN_EVENT_CLOSE:
			return true;
		case SCREEN_EVENT_KEYBOARD:
			screen_get_event_property_iv(screen_ev_, SCREEN_PROPERTY_FLAGS, &val);
			if (val & KEY_DOWN) {
				switch (val) {
				case KEYCODE_ESCAPE:
					return true;
				};
			}
			break;
		default:
			break;
		}
	}
	return false;
}

void
NativeStateQnx::flip()
{
}

void
NativeStateQnx::destroy_window()
{
	if (screen_win_){
		screen_destroy_window(screen_win_);
		screen_win_ = nullptr;
	}
}
