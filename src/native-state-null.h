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

#ifndef GLMARK2_NATIVE_STATE_NULL_H_
#define GLMARK2_NATIVE_STATE_NULL_H_

#include "native-state.h"
#include <csignal>

class NativeStateNull : public NativeState
{
public:
    bool init_display() override;
    void* display() override;
    bool create_window(WindowProperties const& properties) override;
    void* window(WindowProperties& properties) override;
    void visible(bool v) override;
    bool should_quit() override;
    void flip() override;

private:
    static void quit_handler(int signum);
    static volatile std::sig_atomic_t should_quit_;

    WindowProperties properties_;
};

#endif /* GLMARK2_NATIVE_STATE_NULL_H_ */
