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
