/*
 * Copyright © 2011 Linaro Limited
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
 *  Alexandros Frantzis (glmark2)
 */

#include "benchmark.h"

map<string, Scene *> Benchmark::mSceneMap;

void
Benchmark::register_scene(Scene &scene)
{
    mSceneMap[scene.name()] = &scene;
}

Scene &
Benchmark::get_scene_by_name(const string &name)
{
    return *mSceneMap[name];
}

Benchmark::Benchmark(Scene &scene, OptionVector &options) :
    mScene(scene), mOptions(options)
{
}

Benchmark::Benchmark(const string &name, OptionVector &options) :
    mScene(Benchmark::get_scene_by_name(name)), mOptions(options)
{
}

Scene &
Benchmark::setup_scene()
{
    // TODO: Set the options

    mScene.load();
    mScene.start();

    return mScene;
}

void
Benchmark::teardown_scene()
{
    mScene.unload();

    // TODO: Reset options
}
