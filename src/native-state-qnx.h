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
 * Dec 19, 2017: Leo Xu: add QNX native state support.
 *
 */

#ifndef GLMARK2_NATIVE_STATE_QNX_H_
#define GLMARK2_NATIVE_STATE_QNX_H_

#include "native-state.h"
#include <screen/screen.h>

class NativeStateQnx : public NativeState
{
public:
	NativeStateQnx();
	~NativeStateQnx();

	bool init_display();
	void* display();
	bool create_window(WindowProperties const& properties);
	void* window(WindowProperties& properties);
	void visible(bool v);
	bool should_quit();
	void flip();

private:
	void destroy_window();
	screen_context_t screen_ctx_;
	screen_window_t screen_win_;
	screen_event_t screen_ev_;
	int win_visible_;
	WindowProperties win_properties_;
};

#endif /* GLMARK2_NATIVE_STATE_QNX_H_ */
