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
#ifndef _MATRIX_H
#define _MATRIX_H

#include "oglsdl.h"

#include <stdio.h>
#include <math.h>

class Matrix4f
{

public:
    GLfloat m[16];

    Matrix4f();
    Matrix4f(Matrix4f &mat);
    Matrix4f(GLfloat x, GLfloat y, GLfloat z);

    Matrix4f &translate(GLfloat x, GLfloat y, GLfloat z);
    Matrix4f &rotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
    Matrix4f &transpose();
    Matrix4f &perspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar);
    Matrix4f &identity();
    Matrix4f &invert();

    Matrix4f &operator*=(const Matrix4f &pM);

    void display(const char *str);
};

#endif
