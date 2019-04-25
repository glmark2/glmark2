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
 *  Alexandros Frantzis (glmark2)
 */
#include <stdlib.h>
#include "scene.h"
#include "mat.h"
#include "stack.h"
#include "vec.h"
#include "log.h"
#include "shader-source.h"
#include "util.h"
#include "texture.h"
#include <cmath>
#include <cstring>

using LibMatrix::vec2;
using LibMatrix::vec3;
using LibMatrix::vec4;
using LibMatrix::mat4;
using LibMatrix::Stack4;

SceneUbo::SceneUbo(Canvas &pCanvas) :
    Scene(pCanvas, "ubo"),
    numQuads_(0)
{
    options_["quads"] = Scene::Option("quads", "2", "Number of quads to render");
}

SceneUbo::~SceneUbo()
{
}

bool
SceneUbo::load()
{
    running_ = false;

    return true;
}

void
SceneUbo::unload()
{
}

bool
SceneUbo::setup()
{
    if (!Scene::setup())
        return false;

    numQuads_ = Util::fromString<int>(options_["quads"].value);

    // Load shaders
    std::string vtx_shader_filename;
    std::string frg_shader_filename;

    vtx_shader_filename = GLMARK_DATA_PATH"/shaders/ubo.vert";
    frg_shader_filename = GLMARK_DATA_PATH"/shaders/light-basic.frag";

    ShaderSource vtx_source(vtx_shader_filename);
    ShaderSource frg_source(frg_shader_filename);

    if (!Scene::load_shaders_from_strings(program_, vtx_source.str(),
                                          frg_source.str()))
    {
        return false;
    }

    glGenBuffers(1, &ubo_);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_);
    glBufferData(GL_UNIFORM_BUFFER, 16, NULL, GL_DYNAMIC_DRAW);

    GLuint ublock = glGetUniformBlockIndex(program_.handle_, "ublock");   
    glUniformBlockBinding(program_.handle_, ublock, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_); 

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    create_and_setup_mesh();

    program_.start();

    currentFrame_ = 0;

    running_ = true;
    startTime_ = Util::get_timestamp_us() / 1000000.0;
    lastUpdateTime_ = startTime_;

    return true;
}

void
SceneUbo::teardown()
{
    program_.stop();
    program_.release();

    // Re-enable back-face culling
    glEnable(GL_CULL_FACE);
    // Disable alpha blending
    glDisable(GL_BLEND);

    mesh_.clear();

    Scene::teardown();
}

void
SceneUbo::update()
{
    Scene::update();
}

void
SceneUbo::draw()
{
    vec4 color[] = {
        vec4(1.0, 0.0, 0.0, 1.0),
        vec4(0.0, 1.0, 0.0, 1.0),
        vec4(1.0, 1.0, 0.0, 1.0),
        vec4(0.0, 0.0, 1.0, 1.0),
        vec4(1.0, 0.0, 1.0, 1.0),
        vec4(1.0, 1.0, 1.0, 1.0),
    };

    for (size_t i = 0; i < mesh_.size(); ++i) {
        glBindBuffer(GL_UNIFORM_BUFFER, ubo_);
        auto ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, 16,
                                    GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT |
                                    GL_MAP_FLUSH_EXPLICIT_BIT);

        memcpy(ptr, &color[i], sizeof(color[i]));
        glFlushMappedBufferRange(GL_UNIFORM_BUFFER, 0, 16);
        glUnmapBuffer(GL_UNIFORM_BUFFER);
        program_.start();
        mesh_[i].render_vbo();
    }
}

Scene::ValidationResult
SceneUbo::validate()
{
    return Scene::ValidationUnknown;
}

void
SceneUbo::create_and_setup_mesh()
{
    mesh_.resize(numQuads_);

    std::vector<int> vertex_format;
    vertex_format.push_back(3);
    std::vector<GLint> attrib_locations;
    attrib_locations.push_back(program_["position"].location());

    double size = 2.0 / numQuads_;
    double offset = size / 2.0 - 1.0;
    for (auto& m : mesh_) {
        m.set_vertex_format(vertex_format);
        m.make_grid(1, 1, size, size, 0.0, offset);
        offset += size;
        m.build_vbo();
        m.set_attrib_locations(attrib_locations);
    }
}

