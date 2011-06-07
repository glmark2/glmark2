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
#include "matrix.h"

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

    const string &shading = mOptions["shading"].value;

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
    Matrix4f model_view(1.0f, 1.0f, 1.0f);
    Matrix4f model_view_proj(mScreen.mProjection);

    model_view.translate(0.0f, 0.0f, -5.0f);
    model_view.rotate(2 * M_PI * mRotation / 360.0, 0.0f, 1.0f, 0.0f);
    model_view_proj *= model_view;

    glUniformMatrix4fv(mShader.mLocations.ModelViewProjectionMatrix, 1,
                       GL_FALSE, model_view_proj.m);

    // Load the NormalMatrix uniform in the shader. The NormalMatrix is the
    // inverse transpose of the model view matrix.
    model_view.invert().transpose();
    glUniformMatrix4fv(mShader.mLocations.NormalMatrix, 1,
                       GL_FALSE, model_view.m);

    mMesh.render_vbo();
}
