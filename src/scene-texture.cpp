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
#include "shader-source.h"

#include <cmath>

SceneTexture::SceneTexture(Canvas &pCanvas) :
    Scene(pCanvas, "texture")
{
    mOptions["texture-filter"] = Scene::Option("texture-filter", "nearest",
                                               "[nearest, linear, mipmap]");
}

SceneTexture::~SceneTexture()
{
}

int SceneTexture::load()
{
    Model::find_models();
    Model model;

    if(!model.load("cube"))
        return 0;

    model.calculate_normals();
    model.convert_to_mesh(mCubeMesh);
    mCubeMesh.build_vbo();

    mRotationSpeed = LibMatrix::vec3(36.0f, 36.0f, 36.0f);

    mRunning = false;

    return 1;
}

void SceneTexture::unload()
{
    mCubeMesh.reset();
}

void SceneTexture::setup()
{
    Scene::setup();

    // Load shaders
    static const std::string vtx_shader_filename(GLMARK_DATA_PATH"/shaders/light-basic.vert");
    static const std::string frg_shader_filename(GLMARK_DATA_PATH"/shaders/light-basic-tex.frag");
    static const LibMatrix::vec4 lightPosition(20.0f, 20.0f, 10.0f, 1.0f);
    static const LibMatrix::vec4 materialDiffuse(1.0f, 1.0f, 1.0f, 1.0f);
    ShaderSource vtx_source(vtx_shader_filename);
    ShaderSource frg_source(frg_shader_filename);

    // Add constants to shaders
    vtx_source.add_const("LightSourcePosition", lightPosition);
    vtx_source.add_const("MaterialDiffuse", materialDiffuse);

    if (!Scene::load_shaders_from_strings(mProgram, vtx_source.str(),
                                          frg_source.str()))
    {
        return;
    }

    std::vector<GLint> attrib_locations;
    attrib_locations.push_back(mProgram["position"].location());
    attrib_locations.push_back(mProgram["normal"].location());
    attrib_locations.push_back(mProgram["texcoord"].location());
    mCubeMesh.set_attrib_locations(attrib_locations);

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

    mCurrentFrame = 0;
    mRotation = LibMatrix::vec3();
    mRunning = true;
    mStartTime = Scene::get_timestamp_us() / 1000000.0;
    mLastUpdateTime = mStartTime;
}

void SceneTexture::teardown()
{
    mProgram.stop();
    mProgram.release();

    glDeleteTextures(1, &mTexture);

    Scene::teardown();
}

void SceneTexture::update()
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

void SceneTexture::draw()
{
    // Load the ModelViewProjectionMatrix uniform in the shader
    LibMatrix::Stack4 model_view;
    LibMatrix::mat4 model_view_proj(mCanvas.projection());

    model_view.translate(0.0f, 0.0f, -5.0f);
    model_view.rotate(mRotation.x(), 1.0f, 0.0f, 0.0f);
    model_view.rotate(mRotation.y(), 0.0f, 1.0f, 0.0f);
    model_view.rotate(mRotation.z(), 0.0f, 0.0f, 1.0f);
    model_view_proj *= model_view.getCurrent();

    mProgram["ModelViewProjectionMatrix"] = model_view_proj;

    // Load the NormalMatrix uniform in the shader. The NormalMatrix is the
    // inverse transpose of the model view matrix.
    LibMatrix::mat4 normal_matrix(model_view.getCurrent());
    normal_matrix.inverse().transpose();
    mProgram["NormalMatrix"] = normal_matrix;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTexture);

    mCubeMesh.render_vbo();
}

Scene::ValidationResult
SceneTexture::validate()
{
    static const double radius_3d(std::sqrt(3.0));

    if (mRotation.x() != 0 || mRotation.y() != 0 || mRotation.z() != 0)
        return Scene::ValidationUnknown;

    Canvas::Pixel ref;

    Canvas::Pixel pixel = mCanvas.read_pixel(mCanvas.width() / 2 + 3,
                                             mCanvas.height() / 2 + 3);

    const std::string &filter = mOptions["texture-filter"].value;

    if (filter == "nearest")
        ref = Canvas::Pixel(0x3b, 0x3a, 0x39, 0xff);
    else if (filter == "linear")
        ref = Canvas::Pixel(0x36, 0x36, 0x34, 0xff);
    else if (filter == "mipmap")
        ref = Canvas::Pixel(0x35, 0x35, 0x34, 0xff);
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
