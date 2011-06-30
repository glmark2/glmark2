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
 */
#include "scene.h"
#include "log.h"
#include "mat.h"
#include "stack.h"
#include <cmath>

SceneBuild::SceneBuild(Canvas &pCanvas) :
    Scene(pCanvas, "build")
{
    mOptions["use-vbo"] = Scene::Option("use-vbo", "true",
                                        "Whether to use VBOs for rendering [true,false]");
}

SceneBuild::~SceneBuild()
{
}

int SceneBuild::load()
{
    static const std::string vtx_shader_filename(GLMARK_DATA_PATH"/shaders/light-basic.vert");
    static const std::string frg_shader_filename(GLMARK_DATA_PATH"/shaders/light-basic.frag");
    Model model;

    if(!model.load_3ds(GLMARK_DATA_PATH"/models/horse.3ds"))
        return 0;

    model.calculate_normals();
    model.convert_to_mesh(&mMesh);

    if (!Scene::load_shaders(mProgram, vtx_shader_filename, frg_shader_filename))
        return 0;

    mVertexAttribLocation = mProgram.getAttribIndex("position");
    mNormalAttribLocation = mProgram.getAttribIndex("normal");
    mTexcoordAttribLocation = mProgram.getAttribIndex("texcoord");

    mRotationSpeed = 36.0f;

    mRunning = false;

    return 1;
}

void SceneBuild::unload()
{
    mMesh.reset();

    mProgram.stop();
    mProgram.release();
}

void SceneBuild::setup()
{
    Scene::setup();

    static const LibMatrix::vec4 lightAmbient(0.0f, 0.0f, 0.0f, 1.0f);
    static const LibMatrix::vec4 lightDiffuse(0.8f, 0.8f, 0.8f, 1.0f);
    static const LibMatrix::vec4 lightPosition(20.0f, 20.0f, 10.0f, 1.0f);
    static const LibMatrix::vec4 materialColor(1.0f, 1.0f, 1.0f, 1.0f);

    mUseVbo = (mOptions["use-vbo"].value == "true");

    if (mUseVbo)
        mMesh.build_vbo();

    mProgram.start();

    // Load lighting and material uniforms
    mProgram.loadUniformVector(lightAmbient, "LightSourceAmbient");
    mProgram.loadUniformVector(lightPosition, "LightSourcePosition");
    mProgram.loadUniformVector(lightDiffuse, "LightSourceDiffuse");
    mProgram.loadUniformVector(materialColor, "MaterialColor");

    mCurrentFrame = 0;
    mRotation = 0.0;
    mRunning = true;
    mStartTime = Scene::get_timestamp_us() / 1000000.0;
    mLastUpdateTime = mStartTime;
}

void
SceneBuild::teardown()
{
    mProgram.stop();

    if (mUseVbo)
        mMesh.delete_vbo();

    Scene::teardown();
}

void SceneBuild::update()
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

    mCurrentFrame++;
}

void SceneBuild::draw()
{
    LibMatrix::Stack4 model_view;

    // Load the ModelViewProjectionMatrix uniform in the shader
    LibMatrix::mat4 model_view_proj(mCanvas.mProjection);

    model_view.translate(0.0f, 0.0f, -2.5f);
    model_view.rotate(mRotation, 0.0f, 1.0f, 0.0f);
    model_view_proj *= model_view.getCurrent();

    mProgram.loadUniformMatrix(model_view_proj, "ModelViewProjectionMatrix");

    // Load the NormalMatrix uniform in the shader. The NormalMatrix is the
    // inverse transpose of the model view matrix.
    LibMatrix::mat4 normal_matrix(model_view.getCurrent());
    normal_matrix.inverse().transpose();
    mProgram.loadUniformMatrix(normal_matrix, "NormalMatrix");

    if (mUseVbo) {
        mMesh.render_vbo(mVertexAttribLocation,
                         mNormalAttribLocation,
                         mTexcoordAttribLocation);
    }
    else {
        mMesh.render_array(mVertexAttribLocation,
                           mNormalAttribLocation,
                           mTexcoordAttribLocation);
    }
}

Scene::ValidationResult
SceneBuild::validate()
{
    static const double radius_3d(std::sqrt(3.0));

    if (mRotation != 0)
        return Scene::ValidationUnknown;

    Canvas::Pixel ref(0xa7, 0xa7, 0xa7, 0xff);
    Canvas::Pixel pixel = mCanvas.read_pixel(mCanvas.mWidth / 2,
                                             mCanvas.mHeight / 2);

    double dist = pixel_value_distance(pixel, ref);
    if (dist < radius_3d + 0.01) {
        return Scene::ValidationSuccess;
    }
    else {
        Log::debug("Validation failed! Expected: 0x%x Actual: 0x%x Distance: %f\n",
                    ref.to_le32(), pixel.to_le32(), dist);
        return Scene::ValidationFailure;
    }
}
