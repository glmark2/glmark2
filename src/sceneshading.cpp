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
#include "log.h"

#include <cmath>

SceneShading::SceneShading(Screen &pScreen) :
    Scene(pScreen, "shading")
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

    GLfloat lightAmbient[] = {0.1f, 0.1f, 0.1f, 1.0f};
    GLfloat lightDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat lightSpecular[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat lightPosition[] = {20.0f, 20.0f, 10.0f, 1.0f};

    float materialAmbient[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float materialDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float materialSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float materialColor[] = {0.0f, 0.0f, 1.0f, 1.0f};

    const std::string &shading = mOptions["shading"].value;

    if (shading == "gouraud") {
        mShader.load(GLMARK_DATA_PATH"/shaders/light-basic.vert",
                     GLMARK_DATA_PATH"/shaders/light-basic.frag");
    }
    else if (shading == "phong") {
        mShader.load(GLMARK_DATA_PATH"/shaders/light-advanced.vert",
                     GLMARK_DATA_PATH"/shaders/light-advanced.frag");
    }

    mShader.use();

    // Load lighting and material uniforms
    glUniform4fv(mShader.mLocations.LightSourcePosition, 1, lightPosition);

    glUniform3fv(mShader.mLocations.LightSourceAmbient, 1, lightAmbient);
    glUniform3fv(mShader.mLocations.LightSourceDiffuse, 1, lightDiffuse);
    glUniform3fv(mShader.mLocations.LightSourceSpecular, 1, lightSpecular);

    glUniform3fv(mShader.mLocations.MaterialAmbient, 1, materialAmbient);
    glUniform3fv(mShader.mLocations.MaterialDiffuse, 1, materialDiffuse);
    glUniform3fv(mShader.mLocations.MaterialSpecular, 1, materialSpecular);
    glUniform4fv(mShader.mLocations.MaterialColor, 1, materialColor);

    // Calculate and load the half vector
    Vector3f halfVector = Vector3f(lightPosition[0], lightPosition[1], lightPosition[2]);
    halfVector.normalize();
    halfVector += Vector3f(0.0, 0.0, 1.0);
    halfVector.normalize();
    glUniform3fv(mShader.mLocations.LightSourceHalfVector, 1,
                 (GLfloat *)&halfVector);

    mCurrentFrame = 0;
    mRotation = 0.0f;
    mRunning = true;
    mStartTime = SDL_GetTicks() / 1000.0;
    mLastUpdateTime = mStartTime;
}

void SceneShading::teardown()
{
    mShader.remove();
    mShader.unload();

    Scene::teardown();
}

void SceneShading::update()
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

void SceneShading::draw()
{
    // Load the ModelViewProjectionMatrix uniform in the shader
    LibMatrix::Stack4 model_view;
    LibMatrix::mat4 model_view_proj(mScreen.mProjection);

    model_view.translate(0.0f, 0.0f, -5.0f);
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

    mMesh.render_vbo();
}

Scene::ValidationResult
SceneShading::validate()
{
    static const double radius_3d(std::sqrt(3.0));

    if (mRotation != 0) 
        return Scene::ValidationUnknown;

    Screen::Pixel ref;

    Screen::Pixel pixel = mScreen.read_pixel(mScreen.mWidth / 2,
                                             mScreen.mHeight / 2);

    const std::string &filter = mOptions["shading"].value;

    if (filter == "gouraud")
        ref = Screen::Pixel(0x00, 0x00, 0xca, 0xff);
    else if (filter == "phong")
        ref = Screen::Pixel(0x1a, 0x1a, 0xbb, 0xff);
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
