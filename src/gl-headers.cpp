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
 */
#include "gl-headers.h"

void* (GLAD_API_PTR *GLExtensions::MapBuffer) (GLenum target, GLenum access) = 0;
GLboolean (GLAD_API_PTR *GLExtensions::UnmapBuffer) (GLenum target) = 0;

void (GLAD_API_PTR *GLExtensions::GenFramebuffers)(GLsizei n, GLuint *framebuffers) = 0;
void (GLAD_API_PTR *GLExtensions::DeleteFramebuffers)(GLsizei n, const GLuint * framebuffers) = 0;
void (GLAD_API_PTR *GLExtensions::BindFramebuffer)(GLenum target, GLuint framebuffer) = 0;
void (GLAD_API_PTR *GLExtensions::FramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) = 0;
void (GLAD_API_PTR *GLExtensions::FramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) = 0;
GLenum (GLAD_API_PTR *GLExtensions::CheckFramebufferStatus)(GLenum target) = 0;

void (GLAD_API_PTR *GLExtensions::GenRenderbuffers)(GLsizei n, GLuint * renderbuffers) = 0;
void (GLAD_API_PTR *GLExtensions::DeleteRenderbuffers)(GLsizei n, const GLuint * renderbuffers) = 0;
void (GLAD_API_PTR *GLExtensions::BindRenderbuffer)(GLenum target, GLuint renderbuffer) = 0;
void (GLAD_API_PTR *GLExtensions::RenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) = 0;

void (GLAD_API_PTR *GLExtensions::GenerateMipmap)(GLenum target) = 0;

bool
GLExtensions::support(const std::string &ext)
{
    std::string ext_string;
    const char* exts = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
    if (exts) {
        ext_string = exts;
    }

    const size_t ext_size = ext.size();
    size_t pos = 0;

    while ((pos = ext_string.find(ext, pos)) != std::string::npos) {
        char c = ext_string[pos + ext_size];
        if (c == ' ' || c == '\0')
            break;
    }

    return pos != std::string::npos;
}
