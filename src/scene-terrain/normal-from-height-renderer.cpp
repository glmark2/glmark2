/*
 * Copyright © 2012 Linaro Limited
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
 *  Alexandros Frantzis
 */
#include "options.h"
#include "scene.h"
#include "renderer.h"
#include "shader-source.h"

NormalFromHeightRenderer::NormalFromHeightRenderer() :
    TextureRenderer(*normal_from_height_program(true))
{
    normal_from_height_program_ = normal_from_height_program(false);
}

void
NormalFromHeightRenderer::setup_onscreen(Canvas& canvas)
{
    TextureRenderer::setup_onscreen(canvas);

    normal_from_height_program_->start();
    (*normal_from_height_program_)["resolution"] =
        LibMatrix::vec2(canvas.width(), canvas.height());
    normal_from_height_program_->stop();
}

void
NormalFromHeightRenderer::setup_offscreen(const LibMatrix::vec2 &size, bool has_depth)
{
    TextureRenderer::setup_offscreen(size, has_depth);

    normal_from_height_program_->start();
    (*normal_from_height_program_)["resolution"] = size;
    normal_from_height_program_->stop();
}

Program *
NormalFromHeightRenderer::normal_from_height_program(bool create_new)
{
    static Program *normal_from_height_program(0);
    if (create_new)
        normal_from_height_program = 0;

    if (!normal_from_height_program) {
        normal_from_height_program = new Program();
        ShaderSource vtx_shader(Options::data_path + "/shaders/terrain-texture.vert");
        ShaderSource frg_shader(Options::data_path + "/shaders/terrain-normalmap.frag");

        Scene::load_shaders_from_strings(*normal_from_height_program,
                                         vtx_shader.str(), frg_shader.str());

        normal_from_height_program->start();
        (*normal_from_height_program)["heightMap"] = 0;
        (*normal_from_height_program)["height"] = 0.05f;
        (*normal_from_height_program)["uvOffset"] = LibMatrix::vec2(0.0f, 0.0f);
        (*normal_from_height_program)["uvScale"] = LibMatrix::vec2(1.0f, 1.0f);
        normal_from_height_program->stop();
    }

    return normal_from_height_program;
}

