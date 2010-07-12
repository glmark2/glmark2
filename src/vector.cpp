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

