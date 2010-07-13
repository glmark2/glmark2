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
#ifndef _VECTOR_H
#define _VECTOR_H

#include "oglsdl.h"

#include <stdio.h>
#include <math.h>

class Vector3f
{
    union
    {    
        struct { GLfloat x; GLfloat y; GLfloat z; };
        GLfloat v[3];
    };

public:

    Vector3f();
    Vector3f(GLfloat pX, GLfloat pY, GLfloat pZ);

    void display();
    float length();
    void normalize();
    
    Vector3f &operator=(const Vector3f &pV);
    Vector3f &operator+=(const Vector3f &pV);
    Vector3f &operator-=(const Vector3f &pV);
    Vector3f &operator*=(float pF);
    Vector3f &operator/=(float pF);
    Vector3f &operator*=(double pF);
};

extern Vector3f operator+(const Vector3f &pA, const Vector3f &pB);
extern Vector3f operator-(const Vector3f &pA, const Vector3f &pB);
extern Vector3f operator*(const Vector3f &pA, const float &pB);
extern Vector3f operator/(const Vector3f &pA, const float &pB);

extern Vector3f operator*(const Vector3f &pA, const double &pB);

extern float dot(const Vector3f &pA, const Vector3f &pB);
extern Vector3f cross(const Vector3f &pA, const Vector3f &pB);
extern Vector3f normal(const Vector3f &pA, const Vector3f &pB, const Vector3f &pC);

#endif
