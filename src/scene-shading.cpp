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
#include "mat.h"
#include "stack.h"
#include "vec.h"
#include "log.h"

#include <cmath>

SceneShading::SceneShading(Canvas &pCanvas) :
    Scene(pCanvas, "shading")
{
    mOptions["shading"] = Scene::Option("shading", "gouraud",
                                        "[gouraud, phong]");
}

SceneShading::~SceneShading()
{
}

int SceneShading::load()
{
    Model model;

    if(!model.load_3ds(GLMARK_DATA_PATH"/models/cat.3ds"))
        return 0;

    model.calculate_normals();
    model.convert_to_mesh(&mMesh);

    mMesh.build_vbo();

    mRotationSpeed = 36.0f;

    mRunning = false;

    return 1;
}

void SceneShading::unload()
{
    mMesh.reset();
}

void SceneShading::setup()
{
    Scene::setup();

    static const LibMatrix::vec3 lightAmbient(0.1f, 0.1f, 0.1f);
    static const LibMatrix::vec3 lightDiffuse(0.8f, 0.8f, 0.8f);
    static const LibMatrix::vec3 lightSpecular(0.8f, 0.8f, 0.8f);
    static const LibMatrix::vec4 lightPosition(20.0f, 20.0f, 10.0f, 1.0f);

    static const LibMatrix::vec3 materialAmbient(1.0f, 1.0f, 1.0f);
    static const LibMatrix::vec3 materialDiffuse(1.0f, 1.0f, 1.0f);
    static const LibMatrix::vec3 materialSpecular(1.0f, 1.0f, 1.0f);
    static const LibMatrix::vec4 materialColor(0.0f, 0.0f, 1.0f, 1.0f);

    std::string vtx_shader_filename;
    std::string frg_shader_filename;
    const std::string &shading = mOptions["shading"].value;

    if (shading == "gouraud") {
        vtx_shader_filename = GLMARK_DATA_PATH"/shaders/light-basic.vert";
        frg_shader_filename = GLMARK_DATA_PATH"/shaders/light-basic.frag";
    }
    else if (shading == "phong") {
        vtx_shader_filename = GLMARK_DATA_PATH"/shaders/light-advanced.vert";
        frg_shader_filename = GLMARK_DATA_PATH"/shaders/light-advanced.frag";
    }

    if (!Scene::load_shaders(mProgram, vtx_shader_filename, frg_shader_filename))
        return;

    mProgram.start();

    mVertexAttribLocation = mProgram.getAttribIndex("position");
    mNormalAttribLocation = mProgram.getAttribIndex("normal");
    mTexcoordAttribLocation = mProgram.getAttribIndex("texcoord");

    // Load lighting and material uniforms
    mProgram.loadUniformVector(lightAmbient, "LightSourceAmbient");
    mProgram.loadUniformVector(lightPosition, "LightSourcePosition");
    mProgram.loadUniformVector(lightDiffuse, "LightSourceDiffuse");
    mProgram.loadUniformVector(lightSpecular, "LightSourceSpecular");

    mProgram.loadUniformVector(materialAmbient, "MaterialAmbient");
    mProgram.loadUniformVector(materialDiffuse, "MaterialDiffuse");
    mProgram.loadUniformVector(materialSpecular, "MaterialSpecular");
    mProgram.loadUniformVector(materialColor, "MaterialColor");

    // Calculate and load the half vector
    LibMatrix::vec3 halfVector(lightPosition[0], lightPosition[1], lightPosition[2]);
    halfVector.normalize();
    halfVector += LibMatrix::vec3(0.0, 0.0, 1.0);
    halfVector.normalize();
    mProgram.loadUniformVector(halfVector, "LightSourceHalfVector");

    mCurrentFrame = 0;
    mRotation = 0.0f;
    mRunning = true;
    mStartTime = Scene::get_timestamp_us() / 1000000.0;
    mLastUpdateTime = mStartTime;
}

void SceneShading::teardown()
{
    mProgram.stop();
    mProgram.release();

    Scene::teardown();
}

void SceneShading::update()
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

void SceneShading::draw()
{
    // Load the ModelViewProjectionMatrix uniform in the shader
    LibMatrix::Stack4 model_view;
    LibMatrix::mat4 model_view_proj(mCanvas.mProjection);

    model_view.translate(0.0f, 0.0f, -5.0f);
    model_view.rotate(mRotation, 0.0f, 1.0f, 0.0f);
    model_view_proj *= model_view.getCurrent();

    mProgram.loadUniformMatrix(model_view_proj, "ModelViewProjectionMatrix");

    // Load the NormalMatrix uniform in the shader. The NormalMatrix is the
    // inverse transpose of the model view matrix.
    LibMatrix::mat4 normal_matrix(model_view.getCurrent());
    normal_matrix.inverse().transpose();
    mProgram.loadUniformMatrix(normal_matrix, "NormalMatrix");

    mMesh.render_vbo(mVertexAttribLocation,
                     mNormalAttribLocation,
                     mTexcoordAttribLocation);
}

Scene::ValidationResult
SceneShading::validate()
{
    static const double radius_3d(std::sqrt(3.0));

    if (mRotation != 0) 
        return Scene::ValidationUnknown;

    Canvas::Pixel ref;

    Canvas::Pixel pixel = mCanvas.read_pixel(mCanvas.mWidth / 2,
                                             mCanvas.mHeight / 2);

    const std::string &filter = mOptions["shading"].value;

    if (filter == "gouraud")
        ref = Canvas::Pixel(0x00, 0x00, 0xca, 0xff);
    else if (filter == "phong")
        ref = Canvas::Pixel(0x1a, 0x1a, 0xbb, 0xff);
    else
        return Scene::ValidationUnknown;

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
