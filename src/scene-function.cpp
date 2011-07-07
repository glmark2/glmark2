/*
 * Copyright © 2010-2011 Linaro Limited
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
#include "scene.h"
#include "mat.h"
#include "stack.h"
#include "vec.h"
#include "log.h"

#include <sstream>

static const std::string shader_file_base(GLMARK_DATA_PATH"/shaders/function");

static const std::string vtx_file(shader_file_base + ".vert");
static const std::string frg_file(shader_file_base + ".frag");
static const std::string call_file(shader_file_base + "-call.all");
static const std::string step_low_file(shader_file_base + "-step-low.all");
static const std::string step_medium_file(shader_file_base + "-step-medium.all");

SceneFunction::SceneFunction(Canvas &pCanvas) :
    SceneGrid(pCanvas, "function")
{
    mOptions["fragment-steps"] = Scene::Option("fragment-steps", "1",
            "The number of computational steps in the fragment shader");
    mOptions["fragment-function"] = Scene::Option("fragment-function", "true",
            "Whether each computational step includes a function call");
    mOptions["vertex-steps"] = Scene::Option("vertex-steps", "1",
            "The number of computational steps in the vertex shader");
    mOptions["vertex-function"] = Scene::Option("vertex-function", "true",
            "Whether each computational step includes an if-else clause");
    mOptions["vertex-complexity"] = Scene::Option("vertex-complexity", "low",
            "The complexity of each computational step in the vertex shader");
    mOptions["fragment-complexity"] = Scene::Option("fragment-complexity", "low",
            "The complexity of each computational step in the fragment shader");
}

SceneFunction::~SceneFunction()
{
}

static std::string &
replace_string(std::string &str, const std::string &remove, const std::string &insert)
{
    std::string::size_type pos = 0;

    while ((pos = str.find(remove, pos)) != std::string::npos) {
        str.replace(pos, remove.size(), insert);
        pos++;
    }

    return str;
}

static std::string
get_vertex_shader_source(int steps, bool function, std::string &complexity)
{
    std::string vtx_string, step_low_string, step_medium_string, call_string;

    if (!gotSource(vtx_file, vtx_string) ||
        !gotSource(step_low_file, step_low_string) ||
        !gotSource(step_medium_file, step_medium_string) ||
        !gotSource(call_file, call_string))
    {
        return "";
    }

    std::stringstream ss_main;
    std::string process_string;

    if (complexity == "low")
        process_string = step_low_string;
    else if (complexity == "medium")
        process_string = step_medium_string;

    for (int i = 0; i < steps; i++) {
        if (function)
            ss_main << call_string;
        else
            ss_main << process_string;
    }

    replace_string(vtx_string, "$PROCESS$", function ? process_string : "");
    replace_string(vtx_string, "$MAIN$", ss_main.str());

    return vtx_string;
}

static std::string
get_fragment_shader_source(int steps, bool function, std::string &complexity)
{
    std::string frg_string, step_low_string, step_medium_string, call_string;

    if (!gotSource(frg_file, frg_string) ||
        !gotSource(step_low_file, step_low_string) ||
        !gotSource(step_medium_file, step_medium_string) ||
        !gotSource(call_file, call_string))
    {
        return "";
    }

    std::stringstream ss_main;
    std::string process_string;

    if (complexity == "low")
        process_string = step_low_string;
    else if (complexity == "medium")
        process_string = step_medium_string;

    for (int i = 0; i < steps; i++) {
        if (function)
            ss_main << call_string;
        else
            ss_main << process_string;
    }

    replace_string(frg_string, "$PROCESS$", function ? process_string : "");
    replace_string(frg_string, "$MAIN$", ss_main.str());

    return frg_string;
}

void SceneFunction::setup()
{
    SceneGrid::setup();

    /* Parse options */
    bool vtx_function = mOptions["vertex-function"].value == "true";
    bool frg_function = mOptions["fragment-function"].value == "true";
    std::string vtx_complexity = mOptions["vertex-complexity"].value;
    std::string frg_complexity = mOptions["fragment-complexity"].value;
    int vtx_steps = 0;
    int frg_steps = 0;

    std::stringstream ss;

    ss << mOptions["vertex-steps"].value;
    ss >> vtx_steps;
    ss.clear();
    ss << mOptions["fragment-steps"].value;
    ss >> frg_steps;

    /* Load shaders */
    std::string vtx_shader(get_vertex_shader_source(vtx_steps, vtx_function,
                                                    vtx_complexity));
    std::string frg_shader(get_fragment_shader_source(frg_steps, frg_function,
                                                      frg_complexity));

    if (!Scene::load_shaders_from_strings(mProgram, vtx_shader, frg_shader))
        return;

    mProgram.start();

    std::vector<GLint> attrib_locations;
    attrib_locations.push_back(mProgram.getAttribIndex("position"));
    mMesh.set_attrib_locations(attrib_locations);

    mRunning = true;
    mStartTime = Scene::get_timestamp_us() / 1000000.0;
    mLastUpdateTime = mStartTime;
}
