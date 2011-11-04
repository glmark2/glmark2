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
 *  Jesse Barker (glmark2)
 */
#include "scene.h"
#include "log.h"
#include "mat.h"
#include "stack.h"
#include "shader-source.h"
#include "model.h"
#include "texture.h"
#include <cmath>

SceneBump::SceneBump(Canvas &pCanvas) :
    Scene(pCanvas, "bump"),
    texture_(0), rotation_(0.0f), rotationSpeed_(0.0f)
{
    options_["bump-render"] = Scene::Option("bump-render", "off",
                                            "How to render bumps [off, normals, high-poly]");
}

SceneBump::~SceneBump()
{
}

int
SceneBump::load()
{
    rotationSpeed_ = 36.0f;

    running_ = false;

    return 1;
}

void
SceneBump::unload()
{
}

void
SceneBump::setup_model_plain(const std::string &type)
{
    static const std::string vtx_shader_filename(GLMARK_DATA_PATH"/shaders/bump-poly.vert");
    static const std::string frg_shader_filename(GLMARK_DATA_PATH"/shaders/bump-poly.frag");
    static const std::string low_poly_filename("asteroid-low");
    static const std::string high_poly_filename("asteroid-high");
    static const LibMatrix::vec4 lightPosition(20.0f, 20.0f, 10.0f, 1.0f);
    Model model;

    /* Calculate the half vector */
    LibMatrix::vec3 halfVector(lightPosition.x(), lightPosition.y(), lightPosition.z());
    halfVector.normalize();
    halfVector += LibMatrix::vec3(0.0, 0.0, 1.0);
    halfVector.normalize();

    std::string poly_filename = type == "high-poly" ?
                                high_poly_filename : low_poly_filename;

    if(!model.load(poly_filename))
        return;

    model.calculate_normals();

    /* Tell the converter that we only care about position and normal attributes */
    std::vector<std::pair<Model::AttribType, int> > attribs;
    attribs.push_back(std::pair<Model::AttribType, int>(Model::AttribTypePosition, 3));
    attribs.push_back(std::pair<Model::AttribType, int>(Model::AttribTypeNormal, 3));

    model.convert_to_mesh(mesh_, attribs);

    /* Load shaders */
    ShaderSource vtx_source(vtx_shader_filename);
    ShaderSource frg_source(frg_shader_filename);

    /* Add constants to shaders */
    frg_source.add_const("LightSourcePosition", lightPosition);
    frg_source.add_const("LightSourceHalfVector", halfVector);

    if (!Scene::load_shaders_from_strings(program_, vtx_source.str(),
                                          frg_source.str()))
    {
        return;
    }

    std::vector<GLint> attrib_locations;
    attrib_locations.push_back(program_["position"].location());
    attrib_locations.push_back(program_["normal"].location());
    mesh_.set_attrib_locations(attrib_locations);
}

void
SceneBump::setup_model_normals()
{
    static const std::string vtx_shader_filename(GLMARK_DATA_PATH"/shaders/bump-normals.vert");
    static const std::string frg_shader_filename(GLMARK_DATA_PATH"/shaders/bump-normals.frag");
    static const LibMatrix::vec4 lightPosition(20.0f, 20.0f, 10.0f, 1.0f);
    Model model;

    if(!model.load("asteroid-low"))
        return;

    /* Calculate the half vector */
    LibMatrix::vec3 halfVector(lightPosition.x(), lightPosition.y(), lightPosition.z());
    halfVector.normalize();
    halfVector += LibMatrix::vec3(0.0, 0.0, 1.0);
    halfVector.normalize();

    /*
     * We don't care about the vertex normals. We are using a per-fragment
     * normal map (in object space coordinates).
     */
    std::vector<std::pair<Model::AttribType, int> > attribs;
    attribs.push_back(std::pair<Model::AttribType, int>(Model::AttribTypePosition, 3));
    attribs.push_back(std::pair<Model::AttribType, int>(Model::AttribTypeTexcoord, 2));

    model.convert_to_mesh(mesh_, attribs);

    /* Load shaders */
    ShaderSource vtx_source(vtx_shader_filename);
    ShaderSource frg_source(frg_shader_filename);

    /* Add constants to shaders */
    frg_source.add_const("LightSourcePosition", lightPosition);
    frg_source.add_const("LightSourceHalfVector", halfVector);

    if (!Scene::load_shaders_from_strings(program_, vtx_source.str(),
                                          frg_source.str()))
    {
        return;
    }

    std::vector<GLint> attrib_locations;
    attrib_locations.push_back(program_["position"].location());
    attrib_locations.push_back(program_["texcoord"].location());
    mesh_.set_attrib_locations(attrib_locations);

    Texture::load(GLMARK_DATA_PATH"/textures/asteroid-normal-map.png", &texture_,
                  GL_NEAREST, GL_NEAREST, 0);
}

void
SceneBump::setup()
{
    Scene::setup();

    const std::string &bump_render = options_["bump-render"].value;

    Model::find_models();
    if (bump_render == "normals")
        setup_model_normals();
    else if (bump_render == "off" || bump_render == "high-poly")
        setup_model_plain(bump_render);


    mesh_.build_vbo();

    program_.start();

    // Load texture sampler value
    program_["NormalMap"] = 0;

    currentFrame_ = 0;
    rotation_ = 0.0;
    running_ = true;
    startTime_ = Scene::get_timestamp_us() / 1000000.0;
    lastUpdateTime_ = startTime_;
}

void
SceneBump::teardown()
{
    mesh_.reset();

    program_.stop();
    program_.release();

    glDeleteTextures(1, &texture_);
    texture_ = 0;

    Scene::teardown();
}

void
SceneBump::update()
{
    double current_time = Scene::get_timestamp_us() / 1000000.0;
    double dt = current_time - lastUpdateTime_;
    double elapsed_time = current_time - startTime_;

    lastUpdateTime_ = current_time;

    if (elapsed_time >= duration_) {
        averageFPS_ = currentFrame_ / elapsed_time;
        running_ = false;
    }

    rotation_ += rotationSpeed_ * dt;

    currentFrame_++;
}

void
SceneBump::draw()
{
    LibMatrix::Stack4 model_view;

    // Load the ModelViewProjectionMatrix uniform in the shader
    LibMatrix::mat4 model_view_proj(canvas_.projection());

    model_view.translate(0.0f, 0.0f, -3.5f);
    model_view.rotate(rotation_, 0.0f, 1.0f, 0.0f);
    model_view_proj *= model_view.getCurrent();

    program_["ModelViewProjectionMatrix"] = model_view_proj;

    // Load the NormalMatrix uniform in the shader. The NormalMatrix is the
    // inverse transpose of the model view matrix.
    LibMatrix::mat4 normal_matrix(model_view.getCurrent());
    normal_matrix.inverse().transpose();
    program_["NormalMatrix"] = normal_matrix;

    mesh_.render_vbo();
}

Scene::ValidationResult
SceneBump::validate()
{
    static const double radius_3d(std::sqrt(3.0));

    if (rotation_ != 0)
        return Scene::ValidationUnknown;

    Canvas::Pixel ref;

    Canvas::Pixel pixel = canvas_.read_pixel(canvas_.width() / 2,
                                             canvas_.height() / 2);

    const std::string &bump_render = options_["bump-render"].value;

    if (bump_render == "off")
        ref = Canvas::Pixel(0x81, 0x81, 0x81, 0xff);
    else if (bump_render == "high-poly")
        ref = Canvas::Pixel(0x9c, 0x9c, 0x9c, 0xff);
    else if (bump_render == "normals")
        ref = Canvas::Pixel(0xa4, 0xa4, 0xa4, 0xff);
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
