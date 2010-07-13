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
#include "vector.h"

Vector3f::Vector3f()
{
    x = 0.0f;  y = 0.0f;  z = 0.0f;
}

Vector3f::Vector3f(GLfloat pX, GLfloat pY, GLfloat pZ)
{
    x = pX; y = pY; z = pZ;
}

void Vector3f::display()
{
    printf("(%f, %f, %f)\n", x, y, z);
}

float Vector3f::length()
{
    return sqrt(x * x + y * y + z * z);
}

void Vector3f::normalize()
{
    GLfloat l = length();
    x /= l; y /= l; z /= l;
}

Vector3f &Vector3f::operator=(const Vector3f &pV)
{
    x = pV.x;
    y = pV.y;
    z = pV.z;
    return *this;
}

Vector3f &Vector3f::operator+=(const Vector3f &pV)
{
    x += pV.x;
    y += pV.y;
    z += pV.z;
    return *this;
}

Vector3f &Vector3f::operator-=(const Vector3f &pV)
{
    x -= pV.x;
    y -= pV.y;
    z -= pV.z;
    return *this;
}

Vector3f &Vector3f::operator*=(float pF)
{
    x *= pF;
    y *= pF;
    z *= pF;
    return *this;
}

Vector3f &Vector3f::operator*=(double pF)
{
    x *= pF;
    y *= pF;
    z *= pF;
    return *this;
}

Vector3f &Vector3f::operator/=(float pF)
{
    x /= pF;
    y /= pF;
    z /= pF;
    return *this;
}

Vector3f operator+(const Vector3f &pA, const Vector3f &pB)
{
    Vector3f v = pA;
    return v += pB;
}

Vector3f operator-(const Vector3f &pA, const Vector3f &pB)
{
    Vector3f v = pA;
    return v -= pB;
}

Vector3f operator*(const Vector3f &pA, const float &pB)
{
    Vector3f v = pA;
    return v *= pB;
}

Vector3f operator/(const Vector3f &pA, const float &pB)
{
    Vector3f v = pA;
    return v /= pB;
}

Vector3f operator*(const Vector3f &pA, const double &pB)
{
    Vector3f v = pA;
    return v *= pB;
}

Vector3f cross(const Vector3f &pA, const Vector3f &pB)
{
    return Vector3f(pA.y * pB.z - pA.z * pB.y,
                    pA.z * pB.x - pA.x * pB.z,
                    pA.x * pB.y - pA.y * pB.x);
}

float dot(const Vector3f &pA, const Vector3f &pB)
{
    return pA.x * pB.x + pA.y * pB.y + pA.z * pB.z;
}

Vector3f normal(const Vector3f &pA, const Vector3f &pB, const Vector3f &pC)
{
    Vector3f n = cross(pB - pA, pC - pA);
    n.normalize();
    return n;
}

