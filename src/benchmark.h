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
#ifndef GLMARK2_BENCHMARK_H_
#define GLMARK2_BENCHMARK_H_

#include <vector>
#include <string>
#include <map>

#include "scene.h"

using std::vector;
using std::pair;
using std::string;
using std::map;

class Benchmark
{
public:
    typedef pair<string, string> OptionPair;

    Benchmark(Scene &scene, const vector<OptionPair> &options);
    Benchmark(const string &name, const vector<OptionPair> &options);

    Scene &setup_scene();
    void teardown_scene();

    static void register_scene(Scene &scene);
    static Scene &get_scene_by_name(const string &name);

private:
    Scene &mScene;
    vector<OptionPair> mOptions;

    static map<string, Scene *> mSceneMap;
};

#endif
