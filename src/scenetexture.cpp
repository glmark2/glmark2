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

SceneTexture::SceneTexture(Screen &pScreen) :
    Scene(pScreen, "texture")
{
    mOptions["texture-filter"] = Scene::Option("texture-filter", "nearest",
                                               "[nearest, linear, mipmap]");
}

SceneTexture::~SceneTexture()
{
}

int SceneTexture::load()
{
    Model model;

    if(!model.load_3ds(GLMARK_DATA_PATH"/models/cube.3ds"))
        return 0;

    model.calculate_normals();
    model.convert_to_mesh(&mCubeMesh);
    mCubeMesh.build_vbo();

    mShader.load(GLMARK_DATA_PATH"/shaders/light-basic.vert",
                 GLMARK_DATA_PATH"/shaders/light-basic-tex.frag");

    mRotationSpeed = Vector3f(36.0f, 36.0f, 36.0f);

    mRunning = false;

    return 1;
}

void SceneTexture::unload()
{
    mCubeMesh.reset();
    mShader.unload();
}

void SceneTexture::setup()
{
    Scene::setup();

    GLfloat lightAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat lightDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat lightPosition[] = {20.0f, 20.0f, 10.0f, 1.0f};
    GLfloat materialColor[] = {1.0f, 1.0f, 1.0f, 1.0f};

    // Create texture according to selected filtering
    GLint min_filter = GL_NONE;
    GLint mag_filter = GL_NONE;
    const string &filter = mOptions["texture-filter"].value;

    if (filter == "nearest") {
        min_filter = GL_NEAREST;
        mag_filter = GL_NEAREST;
    }
    else if (filter == "linear") {
        min_filter = GL_LINEAR;
        mag_filter = GL_LINEAR;
    }
    else if (filter == "mipmap") {
        min_filter = GL_LINEAR_MIPMAP_LINEAR;
        mag_filter = GL_LINEAR;
    }

    Texture::load(GLMARK_DATA_PATH"/textures/crate-base.bmp", &mTexture,
                  min_filter, mag_filter, 0);

    mShader.use();

    // Load lighting and material uniforms
    glUniform4fv(mShader.mLocations.LightSourcePosition, 1, lightPosition);

    glUniform3fv(mShader.mLocations.LightSourceAmbient, 1, lightAmbient);
    glUniform3fv(mShader.mLocations.LightSourceDiffuse, 1, lightDiffuse);

    glUniform4fv(mShader.mLocations.MaterialColor, 1, materialColor);

    mCurrentFrame = 0;
    mRotation = Vector3f();
    mRunning = true;
    mStartTime = SDL_GetTicks() / 1000.0;
    mLastUpdateTime = mStartTime;
}

void SceneTexture::teardown()
{
    mShader.remove();
    glDeleteTextures(1, &mTexture);

    Scene::teardown();
}

void SceneTexture::update()
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

void SceneTexture::draw()
{
    // Load the ModelViewProjectionMatrix uniform in the shader
    Matrix4f model_view(1.0f, 1.0f, 1.0f);
    Matrix4f model_view_proj(mScreen.mProjection);

    model_view.translate(0.0f, 0.0f, -5.0f);
    model_view.rotate(2 * M_PI * mRotation.x / 360.0, 1.0f, 0.0f, 0.0f);
    model_view.rotate(2 * M_PI * mRotation.y / 360.0, 0.0f, 1.0f, 0.0f);
    model_view.rotate(2 * M_PI * mRotation.z / 360.0, 0.0f, 0.0f, 1.0f);
    model_view_proj *= model_view;

    glUniformMatrix4fv(mShader.mLocations.ModelViewProjectionMatrix, 1,
                       GL_FALSE, model_view_proj.m);

    // Load the NormalMatrix uniform in the shader. The NormalMatrix is the
    // inverse transpose of the model view matrix.
    model_view.invert().transpose();
    glUniformMatrix4fv(mShader.mLocations.NormalMatrix, 1,
                       GL_FALSE, model_view.m);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTexture);

    mCubeMesh.render_vbo();
}
