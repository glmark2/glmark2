//
// Copyright (c) 2011 Linaro Limited
//
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the MIT License which accompanies
// this distribution, and is available at
// http://www.opensource.org/licenses/mit-license.php
//
// Contributors:
//     Jesse Barker - original implementation.
//
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <GL/glew.h>
#include "program.h"

using std::string;
using LibMatrix::mat4;
using LibMatrix::vec2;
using LibMatrix::vec3;
using LibMatrix::vec4;

bool
gotSource(const string& filename, string& source)
{
    using std::ifstream;
    ifstream inputFile(filename.c_str());
    if (!inputFile)
    {
        std::cerr << "Failed to open \"" << filename << "\"" << std::endl;
        return false;
    }

    string curLine;
    while (getline(inputFile, curLine))
    {
        source += curLine;
        source += '\n';
    }

    return true;
}

Shader::Shader(unsigned int type, const string& source) :
    handle_(0),
    type_(type),
    source_(source),
    ready_(false),
    valid_(false)
{
    // Create our shader and setup the source code.
    handle_ = glCreateShader(type);
    if (!handle_)
    {
        message_ = string("Failed to create the new shader.");
        return;
    }
    const GLchar* shaderSource = source_.c_str();
    glShaderSource(handle_, 1, &shaderSource, NULL);
    GLint param = 0;
    glGetShaderiv(handle_, GL_SHADER_SOURCE_LENGTH, &param);
    if (static_cast<unsigned int>(param) != source_.length() + 1)
    {
        std::ostringstream o(string("Expected shader source length "));
        o << source_.length() << ", but got " << param << std::endl;
        message_ = o.str();
        return;
    }
    valid_ = true;
}

Shader::~Shader()
{
    handle_ = 0;
    type_ = 0;
    ready_ = false;
    valid_ = false;
}

void
Shader::compile()
{
    // Make sure we have a good shader and haven't already compiled it.
    if (!valid_ || ready_)
    {
        return;
    }
    glCompileShader(handle_);
    GLint param = 0;
    glGetShaderiv(handle_, GL_COMPILE_STATUS, &param);
    if (param == GL_FALSE)
    {
        glGetShaderiv(handle_, GL_INFO_LOG_LENGTH, &param);
        GLchar* infoLog = new GLchar[param + 1];
        glGetShaderInfoLog(handle_, param + 1, NULL, infoLog);
        message_ = infoLog;
        delete [] infoLog;
        return;
    }
    ready_ = true;
}

void
Shader::attach(unsigned int program)
{
    // Shader must be valid and compiled to be attached to a program.
    if (!valid_ || !ready_)
    {
        return;
    }
    glAttachShader(program, handle_);
}

void
Shader::release()
{
    if (handle_)
    {
        glDeleteShader(handle_);
    }
    handle_ = 0;
    type_ = 0;
    ready_ = false;
    valid_ = false;
}

Program::Program() :
    handle_(0),
    ready_(false),
    valid_(false)
{
}

Program::~Program()
{
    // First release all of the shader resources attached to us and clean up
    // our handle.
    release();
}

void
Program::init()
{
    handle_ = glCreateProgram();
    if (!handle_)
    {
        message_ = string("Failed to create the new program");
        return;
    }

    valid_ = true;
}

void
Program::release()
{
    // First delete all of the shader resources attached to us.
    for (std::vector<Shader>::iterator shaderIt = shaders_.begin(); shaderIt != shaders_.end(); shaderIt++)
    {
        shaderIt->release();
    }

    // Clear out the shader vector so we're ready to reuse it.
    shaders_.clear();

    // Clear out the error string to make sure we don't return anything stale.
    message_.clear();

    if (handle_)
    {
        glDeleteProgram(handle_);
    }
    handle_ = 0;
    ready_ = false;
    valid_ = false;
}
void
Program::addShader(unsigned int type, const string& source)
{
    if (!valid_)
    {
        return;
    }

    Shader shader(type, source);
    if (!shader.valid())
    {
        message_ = shader.errorMessage();
        valid_ = false;
        return;
    }

    shader.compile();

    if (!shader.ready())
    {
        message_ = shader.errorMessage();
        valid_ = false;
        return;
    }

    shader.attach(handle_);
    shaders_.push_back(shader);
    return;
}

void
Program::build()
{
    if (!valid_ || ready_)
    {
        return;
    }

    if (shaders_.empty())
    {
        message_ = string("There are no shaders attached to this program");
        return;
    }

    glLinkProgram(handle_);
    GLint param = 1;
    glGetProgramiv(handle_, GL_LINK_STATUS, &param);
    if (param == GL_FALSE)
    {
        glGetProgramiv(handle_, GL_INFO_LOG_LENGTH, &param);
        GLchar* infoLog = new GLchar[param + 1];
        glGetProgramInfoLog(handle_, param + 1, NULL, infoLog);
        message_ = infoLog;
        delete [] infoLog;
        return;
    }
    ready_ = true;
}

void
Program::start()
{
    if (!valid_ || !ready_)
    {
        return;
    }
    glUseProgram(handle_);
}

void
Program::stop()
{
    glUseProgram(0);
}

void
Program::loadUniformMatrix(const mat4& m, const string& name)
{
    ready_ = false;
    GLint location = glGetUniformLocation(handle_, name.c_str());
    if (location < 0)
    {
        message_ = string("Failed to get uniform location for \"") + name +
            string("\"");
        return;
    }

    // Our matrix representation is column-major, so transpose is false here.
    glUniformMatrix4fv(location, 1, GL_FALSE, m);
    ready_ = true;
}

void
Program::loadUniformVector(const vec2& v, const string& name)
{
    ready_ = false;
    GLint location = glGetUniformLocation(handle_, name.c_str());
    if (location < 0)
    {
        message_ = string("Failed to get uniform location for \"") + name +
            string("\"");
        return;
    }

    glUniform2fv(location, 1, v);
    ready_ = true;
}

void
Program::loadUniformVector(const vec3& v, const string& name)
{
    ready_ = false;
    GLint location = glGetUniformLocation(handle_, name.c_str());
    if (location < 0)
    {
        message_ = string("Failed to get uniform location for \"") + name +
            string("\"");
        return;
    }

    glUniform3fv(location, 1, v);
    ready_ = true;
}

void
Program::loadUniformVector(const vec4& v, const string& name)
{
    ready_ = false;
    GLint location = glGetUniformLocation(handle_, name.c_str());
    if (location < 0)
    {
        message_ = string("Failed to get uniform location for \"") + name +
            string("\"");
        return;
    }

    glUniform4fv(location, 1, v);
    ready_ = true;
}

void
Program::loadUniformScalar(const float& f, const string& name)
{
    ready_ = false;
    GLint location = glGetUniformLocation(handle_, name.c_str());
    if (location < 0)
    {
        message_ = string("Failed to get uniform location for \"") + name +
            string("\"");
        return;
    }

    glUniform1f(location, f);
    ready_ = true;
}

void
Program::loadUniformScalar(const int& i, const string& name)
{
    ready_ = false;
    GLint location = glGetUniformLocation(handle_, name.c_str());
    if (location < 0)
    {
        message_ = string("Failed to get uniform location for \"") + name +
            string("\"");
        return;
    }

    glUniform1i(location, i);
    ready_ = true;
}

int
Program::getAttribIndex(const string& name)
{
    GLint index = glGetAttribLocation(handle_, name.c_str());
    if (index < 0)
    {
        message_ = string("Failed to get attribute location for \"") + name +
            string("\"");
    }
    return index;
}
