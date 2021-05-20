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
#include "renderer.h"

BaseRenderer::BaseRenderer() :
    texture_(0), input_texture_(0), fbo_(0), depth_renderbuffer_(0),
    owns_fbo_(false), min_filter_(GL_LINEAR), mag_filter_(GL_LINEAR),
    wrap_s_(GL_CLAMP_TO_EDGE), wrap_t_(GL_CLAMP_TO_EDGE)
{
}

BaseRenderer::~BaseRenderer()
{
    glDeleteTextures(1, &texture_);
    GLExtensions::DeleteRenderbuffers(1, &depth_renderbuffer_);
    if (owns_fbo_)
        GLExtensions::DeleteFramebuffers(1, &fbo_);
}

void
BaseRenderer::setup_onscreen(Canvas& canvas)
{
    size_ = LibMatrix::vec2(canvas.width(), canvas.height());
    recreate(&canvas, false);
}

void
BaseRenderer::setup_offscreen(const LibMatrix::vec2 &size, bool has_depth)
{
    size_ = size;
    recreate(nullptr, has_depth);
}

void
BaseRenderer::setup_texture(GLint min_filter, GLint mag_filter,
                            GLint wrap_s, GLint wrap_t)
{
    min_filter_ = min_filter;
    mag_filter_ = mag_filter;
    wrap_s_ = wrap_s;
    wrap_t_ = wrap_t;
    update_texture_parameters();
}

void
BaseRenderer::make_current()
{
    GLExtensions::BindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glViewport(0, 0, size_.x(), size_.y());
    if (!owns_fbo_ || depth_renderbuffer_) {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
    }
    else {
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
    }
}

void
BaseRenderer::update_mipmap()
{
    if (texture_ && GLExtensions::GenerateMipmap &&
        min_filter_ != GL_NEAREST && min_filter_ != GL_LINEAR) {
        glBindTexture(GL_TEXTURE_2D, texture_);
        GLExtensions::GenerateMipmap(GL_TEXTURE_2D);
    }
}

void
BaseRenderer::recreate(Canvas* canvas, bool has_depth)
{
    if (texture_) {
        glDeleteTextures(1, &texture_);
        texture_ = 0;
    }
    if (owns_fbo_ && fbo_) {
        GLExtensions::DeleteRenderbuffers(1, &depth_renderbuffer_);
        depth_renderbuffer_ = 0;
        GLExtensions::DeleteFramebuffers(1, &fbo_);
        fbo_ = 0;
    }

    if (canvas) {
        fbo_ = canvas->fbo();
        owns_fbo_ = false;
    } else {
        create_texture();
        create_fbo(has_depth);
        owns_fbo_ = true;
    }
}

void
BaseRenderer::create_texture()
{
    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size_.x(), size_.y(), 0,
            GL_RGBA, GL_UNSIGNED_BYTE, 0);
    update_texture_parameters();
}

void
BaseRenderer::update_texture_parameters()
{
    if (texture_) {
        glBindTexture(GL_TEXTURE_2D, texture_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t_);
        if (!GLExtensions::GenerateMipmap) {
            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP,
                            (min_filter_ != GL_NEAREST && min_filter_ != GL_LINEAR) ?
                                GL_TRUE : GL_FALSE);
        }
    }
    update_mipmap();
}

void
BaseRenderer::create_fbo(bool has_depth)
{
    if (has_depth) {
        /* Create a renderbuffer for depth storage */
        GLExtensions::GenRenderbuffers(1, &depth_renderbuffer_);
        GLExtensions::BindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer_);
        GLExtensions::RenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16,
                size_.x(), size_.y());
    }

    GLint prev_fbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prev_fbo);

    /* Create the FBO and attach the texture and the renderebuffer */
    GLExtensions::GenFramebuffers(1, &fbo_);
    GLExtensions::BindFramebuffer(GL_FRAMEBUFFER, fbo_);
    GLExtensions::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, texture_, 0);
    if (has_depth) {
        GLExtensions::FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                GL_RENDERBUFFER, depth_renderbuffer_);
    }

    GLExtensions::BindFramebuffer(GL_FRAMEBUFFER, prev_fbo);
}
