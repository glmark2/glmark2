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

SceneBuild::SceneBuild(Screen &pScreen) :
    Scene(pScreen, "build")
{
    mOptions["use-vbo"] = Scene::Option("use-vbo", "true",
                                        "Whether to use VBOs for rendering [true,false]");
}

SceneBuild::~SceneBuild()
{
}

int SceneBuild::load()
{
    Model model;

    if(!model.load_3ds(GLMARK_DATA_PATH"/models/horse.3ds"))
        return 0;

    model.calculate_normals();
    model.convert_to_mesh(&mMesh);

    mShader.load(GLMARK_DATA_PATH"/shaders/light-basic.vert",
                 GLMARK_DATA_PATH"/shaders/light-basic.frag");

    mRotationSpeed = 36.0f;

    mRunning = false;

    return 1;
}

void SceneBuild::unload()
{
    mMesh.reset();
    mShader.remove();
    mShader.unload();
}

void SceneBuild::setup()
{
    Scene::setup();

    GLfloat lightAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat lightDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat lightPosition[] = {20.0f, 20.0f, 10.0f, 1.0f};
    GLfloat materialColor[] = {1.0f, 1.0f, 1.0f, 1.0f};


    mUseVbo = (mOptions["use-vbo"].value == "true");

    if (mUseVbo)
        mMesh.build_vbo();

    mShader.use();

    // Load lighting and material uniforms
    glUniform4fv(mShader.mLocations.LightSourcePosition, 1, lightPosition);

    glUniform3fv(mShader.mLocations.LightSourceAmbient, 1, lightAmbient);
    glUniform3fv(mShader.mLocations.LightSourceDiffuse, 1, lightDiffuse);

    glUniform4fv(mShader.mLocations.MaterialColor, 1, materialColor);

    mCurrentFrame = 0;
    mRotation = 0.0;
    mRunning = true;
    mStartTime = SDL_GetTicks() / 1000.0;
    mLastUpdateTime = mStartTime;
}

void
SceneBuild::teardown()
{
    mShader.remove();

    if (mUseVbo)
        mMesh.delete_vbo();

    Scene::teardown();
}

void SceneBuild::update()
{
    double current_time = SDL_GetTicks() / 1000.0;
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
    LibMatrix::mat4 model_view_proj(mScreen.mProjection);

    model_view.translate(0.0f, 0.0f, -2.5f);
    model_view.rotate(mRotation, 0.0f, 1.0f, 0.0f);
    model_view_proj *= model_view.getCurrent();

    glUniformMatrix4fv(mShader.mLocations.ModelViewProjectionMatrix, 1,
                       GL_FALSE, model_view_proj);

    // Load the NormalMatrix uniform in the shader. The NormalMatrix is the
    // inverse transpose of the model view matrix.
    LibMatrix::mat4 normal_matrix(model_view.getCurrent());
    normal_matrix.inverse().transpose();
    glUniformMatrix4fv(mShader.mLocations.NormalMatrix, 1,
                       GL_FALSE, normal_matrix);

    if (mUseVbo)
        mMesh.render_vbo();
    else
        mMesh.render_array();
}

Scene::ValidationResult
SceneBuild::validate()
{
    static const double radius_3d(std::sqrt(3.0));

    if (mRotation != 0)
        return Scene::ValidationUnknown;

    Screen::Pixel ref(0xa7, 0xa7, 0xa7, 0xff);
    Screen::Pixel pixel = mScreen.read_pixel(mScreen.mWidth / 2,
                                             mScreen.mHeight / 2);

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
