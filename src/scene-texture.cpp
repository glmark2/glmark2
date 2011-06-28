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

#include "program.h"
#include <cmath>

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
    static const std::string vtx_shader_filename(GLMARK_DATA_PATH"/shaders/light-basic.vert");
    static const std::string frg_shader_filename(GLMARK_DATA_PATH"/shaders/light-basic-tex.frag");
    Model model;

    if(!model.load_3ds(GLMARK_DATA_PATH"/models/cube.3ds"))
        return 0;

    model.calculate_normals();
    model.convert_to_mesh(&mCubeMesh);
    mCubeMesh.build_vbo();

    if (!Scene::load_shaders(mProgram, vtx_shader_filename, frg_shader_filename))
        return 0;

    mVertexAttribLocation = mProgram.getAttribIndex("position");
    mNormalAttribLocation = mProgram.getAttribIndex("normal");
    mTexcoordAttribLocation = mProgram.getAttribIndex("texcoord");

    mRotationSpeed = LibMatrix::vec3(36.0f, 36.0f, 36.0f);

    mRunning = false;

    return 1;
}

void SceneTexture::unload()
{
    mCubeMesh.reset();

    mProgram.stop();
    mProgram.release();
}

void SceneTexture::setup()
{
    Scene::setup();

    static const LibMatrix::vec4 lightAmbient(0.0f, 0.0f, 0.0f, 1.0f);
    static const LibMatrix::vec4 lightDiffuse(0.8f, 0.8f, 0.8f, 1.0f);
    static const LibMatrix::vec4 lightPosition(20.0f, 20.0f, 10.0f, 1.0f);
    static const LibMatrix::vec4 materialColor(1.0f, 1.0f, 1.0f, 1.0f);

    // Create texture according to selected filtering
    GLint min_filter = GL_NONE;
    GLint mag_filter = GL_NONE;
    const std::string &filter = mOptions["texture-filter"].value;

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

    Texture::load(GLMARK_DATA_PATH"/textures/crate-base.png", &mTexture,
                  min_filter, mag_filter, 0);

    mProgram.start();

    // Load lighting and material uniforms
    mProgram.loadUniformVector(lightAmbient, "LightSourceAmbient");
    mProgram.loadUniformVector(lightPosition, "LightSourcePosition");
    mProgram.loadUniformVector(lightDiffuse, "LightSourceDiffuse");
    mProgram.loadUniformVector(materialColor, "MaterialColor");

    mCurrentFrame = 0;
    mRotation = LibMatrix::vec3();
    mRunning = true;
    mStartTime = SDL_GetTicks() / 1000.0;
    mLastUpdateTime = mStartTime;
}

void SceneTexture::teardown()
{
    mProgram.stop();
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
    LibMatrix::Stack4 model_view;
    LibMatrix::mat4 model_view_proj(mScreen.mProjection);

    model_view.translate(0.0f, 0.0f, -5.0f);
    model_view.rotate(mRotation.x(), 1.0f, 0.0f, 0.0f);
    model_view.rotate(mRotation.y(), 0.0f, 1.0f, 0.0f);
    model_view.rotate(mRotation.z(), 0.0f, 0.0f, 1.0f);
    model_view_proj *= model_view.getCurrent();

    mProgram.loadUniformMatrix(model_view_proj, "ModelViewProjectionMatrix");

    // Load the NormalMatrix uniform in the shader. The NormalMatrix is the
    // inverse transpose of the model view matrix.
    LibMatrix::mat4 normal_matrix(model_view.getCurrent());
    normal_matrix.inverse().transpose();
    mProgram.loadUniformMatrix(normal_matrix, "NormalMatrix");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTexture);

    mCubeMesh.render_vbo(mVertexAttribLocation,
                         mNormalAttribLocation,
                         mTexcoordAttribLocation);
}

Scene::ValidationResult
SceneTexture::validate()
{
    static const double radius_3d(std::sqrt(3.0));

    if (mRotation.x() != 0 || mRotation.y() != 0 || mRotation.z() != 0)
        return Scene::ValidationUnknown;

    Screen::Pixel ref;

    Screen::Pixel pixel = mScreen.read_pixel(mScreen.mWidth / 2 - 3,
                                             mScreen.mHeight / 2 - 3);

    const std::string &filter = mOptions["texture-filter"].value;

    if (filter == "nearest")
        ref = Screen::Pixel(0x2b, 0x2a, 0x28, 0xff);
    else if (filter == "linear")
        ref = Screen::Pixel(0x2c, 0x2b, 0x29, 0xff);
    else if (filter == "mipmap")
        ref = Screen::Pixel(0x2d, 0x2c, 0x2a, 0xff);
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
