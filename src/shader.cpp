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
#include "shader.h"

char *readShaderFile(const char *FileName)
{
    FILE *fp;
    char *DATA = NULL;

    int flength = 0;

    fp = fopen(FileName,"rt");

    fseek(fp, 0, SEEK_END);

    flength = ftell(fp);

    rewind(fp);


    DATA = (char *)malloc(sizeof(char) * (flength+1));
    flength = fread(DATA, sizeof(char), flength, fp);
    DATA[flength] = '\0';

    fclose(fp);

    return DATA;
}

Shader::~Shader()
{
    unload();
}

void Shader::load(const char *pVertexShaderFileName, const char *pFragmentShaderFileName)
{
    char *vertex_shader_source, *fragment_shader_source;
    char msg[512];
    GLint status;

    mVertexShader = glCreateShader(GL_VERTEX_SHADER);
    mFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    vertex_shader_source = readShaderFile(pVertexShaderFileName);

    fragment_shader_source = readShaderFile(pFragmentShaderFileName);

    const char *vs = vertex_shader_source;
    const char *fs = fragment_shader_source;

    glShaderSource(mVertexShader, 1, &vs, NULL);
    glShaderSource(mFragmentShader, 1, &fs, NULL);

    free(vertex_shader_source);
    free(fragment_shader_source);

    glCompileShader(mVertexShader);
    glGetShaderiv(mVertexShader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        glGetShaderInfoLog(mVertexShader, sizeof msg, NULL, msg);
        fprintf(stderr, "Error compiling %s: %s", pVertexShaderFileName,
                strlen(msg) > 0 ? msg : "[No info]");
    }

    glCompileShader(mFragmentShader);
    glGetShaderiv(mFragmentShader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        glGetShaderInfoLog(mFragmentShader, sizeof msg, NULL, msg);
        fprintf(stderr, "Error compiling %s: %s", pFragmentShaderFileName,
                strlen(msg) > 0 ? msg : "[No info]");
    }

    mShaderProgram = glCreateProgram();
    glAttachShader(mShaderProgram, mFragmentShader);
    glAttachShader(mShaderProgram, mVertexShader);
    glBindAttribLocation(mShaderProgram, VertexAttribLocation, "position");
    glBindAttribLocation(mShaderProgram, NormalAttribLocation, "normal");
    glBindAttribLocation(mShaderProgram, TexCoordAttribLocation, "texcoord");

    glLinkProgram(mShaderProgram);
    glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        glGetProgramInfoLog(mShaderProgram, sizeof msg, NULL, msg);
        fprintf(stderr, "Error linking shader program: %s",
                strlen(msg) > 0 ? msg : "[No info]");
    }

    mLocations.ModelViewProjectionMatrix = glGetUniformLocation(mShaderProgram,
            "ModelViewProjectionMatrix");
    mLocations.NormalMatrix = glGetUniformLocation(mShaderProgram,
            "NormalMatrix");
    mLocations.LightSourcePosition = glGetUniformLocation(mShaderProgram,
            "LightSourcePosition");
    mLocations.LightSourceHalfVector = glGetUniformLocation(mShaderProgram,
            "LightSourceHalfVector");

    mLocations.LightSourceAmbient = glGetUniformLocation(mShaderProgram,
            "LightSourceAmbient");
    mLocations.LightSourceDiffuse = glGetUniformLocation(mShaderProgram,
            "LightSourceDiffuse");
    mLocations.LightSourceSpecular = glGetUniformLocation(mShaderProgram,
            "LightSourceSpecular");

    mLocations.MaterialAmbient = glGetUniformLocation(mShaderProgram,
            "MaterialAmbient");
    mLocations.MaterialDiffuse = glGetUniformLocation(mShaderProgram,
            "MaterialDiffuse");
    mLocations.MaterialSpecular = glGetUniformLocation(mShaderProgram,
            "MaterialSpecular");
    mLocations.MaterialColor = glGetUniformLocation(mShaderProgram,
            "MaterialColor");
    mLocations.MaterialTexture0 = glGetUniformLocation(mShaderProgram,
            "MaterialTexture0");
    glUseProgram(mShaderProgram);
    glUniform1i(mLocations.MaterialTexture0, 0);
    glUseProgram(0);

#ifdef _DEBUG
    printf("Uniform Locations: %d %d %d %d %d %d %d %d %d %d %d %d\n",
            mLocations.ModelViewProjectionMatrix,
            mLocations.NormalMatrix,
            mLocations.LightSourcePosition,
            mLocations.LightSourceHalfVector,
            mLocations.LightSourceAmbient,
            mLocations.LightSourceDiffuse,
            mLocations.LightSourceSpecular,
            mLocations.MaterialAmbient,
            mLocations.MaterialDiffuse,
            mLocations.MaterialSpecular,
            mLocations.MaterialColor,
            mLocations.MaterialTexture0);
#endif

}

void Shader::use()
{
    glUseProgram(mShaderProgram);
}

void Shader::remove()
{
    glUseProgram(0);
}

void Shader::unload()
{
    glDetachShader(mShaderProgram, mVertexShader);
    glDetachShader(mShaderProgram, mFragmentShader);
    
    glDeleteShader(mVertexShader);
    glDeleteShader(mFragmentShader);
    glDeleteProgram(mShaderProgram);

    mVertexShader = 0;
    mFragmentShader = 0;
    mShaderProgram = 0;
}
