/*
 * Copyright © 2008 Ben Smith
 * Copyright © 2010 Linaro
 *
 * This file is part of the glmark2 OpenGL (ES) 2.0 benchmark.
 *
 * glmark2 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 3, as published by the Free
 * Software Foundation.
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
#ifndef _SHADER_H
#define _SHADER_H

#include "oglsdl.h"

#include <fcntl.h>
#include <unistd.h>

class Shader
{
public:
    ~Shader();
    void load(const char *pVertexShaderFileName, const char *pFragmentShaderFileName);
    void unload();

    void use();
    void remove();
    
    GLuint mVertexShader;
    GLuint mFragmentShader;
    int mShaderProgram;
    struct {
        GLint ModelViewProjectionMatrix;
        GLint NormalMatrix;
        GLint LightSourcePosition;
        GLint LightSourceHalfVector;
        GLint LightSourceAmbient;
        GLint LightSourceDiffuse;
        GLint LightSourceSpecular;
        GLint MaterialAmbient;
        GLint MaterialDiffuse;
        GLint MaterialSpecular;
        GLint MaterialColor;
        GLint MaterialTexture0;
    } mLocations;

    enum {
        VertexAttribLocation = 0,
        NormalAttribLocation = 1,
        TexCoordAttribLocation = 2
    };
};

#endif
