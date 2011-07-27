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
#include "shader-source.h"

#include <sstream>

static const std::string shader_file_base(GLMARK_DATA_PATH"/shaders/loop");

static const std::string vtx_file(shader_file_base + ".vert");
static const std::string frg_file(shader_file_base + ".frag");
static const std::string step_simple_file(shader_file_base + "-step-simple.all");
static const std::string step_loop_file(shader_file_base + "-step-loop.all");

SceneLoop::SceneLoop(Canvas &pCanvas) :
    SceneGrid(pCanvas, "loop")
{
    mOptions["fragment-steps"] = Scene::Option("fragment-steps", "1",
            "The number of computational steps in the fragment shader");
    mOptions["fragment-loop"] = Scene::Option("fragment-function", "true",
            "Whether to execute the steps in the vertex shader using a for loop");
    mOptions["vertex-steps"] = Scene::Option("vertex-steps", "1",
            "The number of computational steps in the vertex shader");
    mOptions["vertex-loop"] = Scene::Option("vertex-function", "true",
            "Whether to execute the steps in the vertex shader using a for loop");
    mOptions["vertex-uniform"] = Scene::Option("vertex-uniform", "true",
            "Whether to use a uniform in the vertex shader for the number of loop iterations to perform (i.e. vertex-steps)");
    mOptions["fragment-uniform"] = Scene::Option("fragment-uniform", "true",
            "Whether to use a uniform in the fragment shader for the number of loop iterations to perform (i.e. fragment-steps)");
}

SceneLoop::~SceneLoop()
{
}

static std::string
get_fragment_shader_source(int steps, bool loop, bool uniform)
{
    ShaderSource source(frg_file);
    ShaderSource source_main;

    if (loop) {
        source_main.append_file(step_loop_file);
        if (uniform) {
            source_main.replace("$NLOOPS$", "FragmentLoops");
        }
        else {
            std::stringstream ss_steps;
            ss_steps << steps;
            source_main.replace("$NLOOPS$", ss_steps.str());
        }
    }
    else {
        for (int i = 0; i < steps; i++)
            source_main.append_file(step_simple_file);
    }

    source.replace("$MAIN$", source_main.str());

    return source.str();
}

static std::string
get_vertex_shader_source(int steps, bool loop, bool uniform)
{
    ShaderSource source(vtx_file);
    ShaderSource source_main;

    if (loop) {
        source_main.append_file(step_loop_file);
        if (uniform) {
            source_main.replace("$NLOOPS$", "VertexLoops");
        }
        else {
            std::stringstream ss_steps;
            ss_steps << steps;
            source_main.replace("$NLOOPS$", ss_steps.str());
        }
    }
    else {
        for (int i = 0; i < steps; i++)
            source_main.append_file(step_simple_file);
    }

    source.replace("$MAIN$", source_main.str());

    return source.str();
}


void SceneLoop::setup()
{
    SceneGrid::setup();

    /* Parse options */
    bool vtx_loop = mOptions["vertex-loop"].value == "true";
    bool frg_loop = mOptions["fragment-loop"].value == "true";
    bool vtx_uniform = mOptions["vertex-uniform"].value == "true";
    bool frg_uniform = mOptions["fragment-uniform"].value == "true";
    int vtx_steps = 0;
    int frg_steps = 0;

    std::stringstream ss;

    ss << mOptions["vertex-steps"].value;
    ss >> vtx_steps;
    ss.clear();
    ss << mOptions["fragment-steps"].value;
    ss >> frg_steps;

    /* Load shaders */
    std::string vtx_shader(get_vertex_shader_source(vtx_steps, vtx_loop,
                                                    vtx_uniform));
    std::string frg_shader(get_fragment_shader_source(frg_steps, frg_loop,
                                                      frg_uniform));

    if (!Scene::load_shaders_from_strings(mProgram, vtx_shader, frg_shader))
        return;

    mProgram.start();

    mProgram.loadUniformScalar(vtx_steps, "VertexLoops");
    mProgram.loadUniformScalar(frg_steps, "FragmentLoops");

    std::vector<GLint> attrib_locations;
    attrib_locations.push_back(mProgram.getAttribIndex("position"));
    mMesh.set_attrib_locations(attrib_locations);

    mRunning = true;
    mStartTime = Scene::get_timestamp_us() / 1000000.0;
    mLastUpdateTime = mStartTime;
}
