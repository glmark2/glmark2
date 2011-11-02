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

class Benchmark
{
public:
    typedef std::pair<std::string, std::string> OptionPair;

    Benchmark(Scene &scene, const std::vector<OptionPair> &options);
    Benchmark(const std::string &name, const std::vector<OptionPair> &options);
    // Create a benchmark from a description string of the form:
    // scene[:opt1=val1:opt2=val2...]
    Benchmark(const std::string &s);

    Scene &setup_scene();
    void teardown_scene();

    static void register_scene(Scene &scene);
    static Scene &get_scene_by_name(const std::string &name);
    static const std::map<std::string, Scene *> &scenes() { return sceneMap_; }

private:
    Scene &scene_;
    std::vector<OptionPair> options_;

    void load_options();

    static std::map<std::string, Scene *> sceneMap_;
};

#endif
