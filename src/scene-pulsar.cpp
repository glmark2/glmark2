/*
 * Copyright © 2008 Ben Smith
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
 *  Ben Smith (original glmark benchmark)
 *  Alexandros Frantzis (glmark2)
 *  Marc Ordinas i Llopis, Collabora Ltd. (pulsar scene)
 */
#include "scene.h"
#include "mat.h"
#include "stack.h"
#include "vec.h"
#include "log.h"
#include "program.h"
#include "shader-source.h"

#include <cmath>

ScenePulsar::ScenePulsar(Canvas &pCanvas) :
    Scene(pCanvas, "pulsar")
{
}

ScenePulsar::~ScenePulsar()
{
}

int ScenePulsar::load()
{
    static const std::string vtx_shader_filename(GLMARK_DATA_PATH"/shaders/pulsar.vert");
    static const std::string frg_shader_filename(GLMARK_DATA_PATH"/shaders/light-basic.frag");

    std::vector<int> vertex_format;
    vertex_format.push_back(3);
    vertex_format.push_back(4);
    mPlaneMesh.set_vertex_format(vertex_format);

    // Build the plane mesh
    mPlaneMesh.next_vertex();
    mPlaneMesh.set_attrib(0, LibMatrix::vec3(-1.0, -1.0, 0.0));
    mPlaneMesh.set_attrib(1, LibMatrix::vec4(1.0, 0.0, 0.0, 0.4));
    mPlaneMesh.next_vertex();
    mPlaneMesh.set_attrib(0, LibMatrix::vec3(-1.0, 1.0, 0.0));
    mPlaneMesh.set_attrib(1, LibMatrix::vec4(0.0, 1.0, 0.0, 0.4));
    mPlaneMesh.next_vertex();
    mPlaneMesh.set_attrib(0, LibMatrix::vec3(1.0, 1.0, 0.0));
    mPlaneMesh.set_attrib(1, LibMatrix::vec4(0.0, 0.0, 1.0, 0.4));

    mPlaneMesh.next_vertex();
    mPlaneMesh.set_attrib(0, LibMatrix::vec3(-1.0, -1.0, 0.0));
    mPlaneMesh.set_attrib(1, LibMatrix::vec4(1.0, 0.0, 0.0, 0.4));
    mPlaneMesh.next_vertex();
    mPlaneMesh.set_attrib(0, LibMatrix::vec3(1.0, 1.0, 0.0));
    mPlaneMesh.set_attrib(1, LibMatrix::vec4(0.0, 0.0, 1.0, 0.4));
    mPlaneMesh.next_vertex();
    mPlaneMesh.set_attrib(0, LibMatrix::vec3(1.0, -1.0, 0.0));
    mPlaneMesh.set_attrib(1, LibMatrix::vec4(1.0, 1.0, 1.0, 0.4));
    mPlaneMesh.build_vbo();

    // Load shaders
    ShaderSource vtx_source(vtx_shader_filename);
    ShaderSource frg_source(frg_shader_filename);

    if (!Scene::load_shaders_from_strings(mProgram, vtx_source.str(),
                                          frg_source.str()))
    {
        return 0;
    }

    std::vector<GLint> attrib_locations;
    attrib_locations.push_back(mProgram.getAttribIndex("position"));
    attrib_locations.push_back(mProgram.getAttribIndex("vertex_color"));
    mPlaneMesh.set_attrib_locations(attrib_locations);

    mRotationSpeed = LibMatrix::vec3(60.0, 70.0, 0.0);

    mRunning = false;

    return 1;
}

void ScenePulsar::unload()
{
	mPlaneMesh.reset();

    mProgram.stop();
    mProgram.release();
}

void ScenePulsar::setup()
{
    Scene::setup();

    // Disable back-face culling
    glDisable(GL_CULL_FACE);

    mProgram.start();

    mCurrentFrame = 0;
    mScale = LibMatrix::vec3(1.0, 1.0, 1.0);
    mRotation = LibMatrix::vec3();
    mRunning = true;
    mStartTime = Scene::get_timestamp_us() / 1000000.0;
    mLastUpdateTime = mStartTime;
}

void ScenePulsar::teardown()
{
    mProgram.stop();
    Scene::teardown();
}

void ScenePulsar::update()
{
    double current_time = Scene::get_timestamp_us() / 1000000.0;
    double dt = current_time - mLastUpdateTime;
    double elapsed_time = current_time - mStartTime;

    mLastUpdateTime = current_time;

    if (elapsed_time >= mDuration) {
        mAverageFPS = mCurrentFrame / elapsed_time;
        mRunning = false;
    }

    mRotation += mRotationSpeed * dt;
    mScale = LibMatrix::vec3(cos(current_time/0.360)*10.0, sin(current_time/0.360)*10.0, 1.0);
    mCurrentFrame++;
}

void ScenePulsar::draw()
{
    // Load the ModelViewProjectionMatrix uniform in the shader
    LibMatrix::Stack4 model_view;
    LibMatrix::mat4 model_view_proj(mCanvas.projection());
    model_view.scale(mScale.x(), mScale.y(), mScale.z());
    model_view.translate(0.0f, 0.0f, -10.0f);
    model_view.rotate(mRotation.x(), 1.0f, 0.0f, 0.0f);
    model_view.rotate(mRotation.y(), 0.0f, 1.0f, 0.0f);
    model_view.rotate(mRotation.z(), 0.0f, 0.0f, 1.0f);
    model_view_proj *= model_view.getCurrent();
    mProgram.loadUniformMatrix(model_view_proj, "ModelViewProjectionMatrix");

    mPlaneMesh.render_vbo();
}

Scene::ValidationResult
ScenePulsar::validate()
{
	return ValidationUnknown;
}
