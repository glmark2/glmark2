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
    Matrix4f(GLfloat x, GLfloat y, GLfloat z);

    Matrix4f &translate(GLfloat x, GLfloat y, GLfloat z);
    Matrix4f &rotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
    Matrix4f &transpose();

    Matrix4f &operator*=(const Matrix4f &pM);

    void display(const char *str);
};

#endif
