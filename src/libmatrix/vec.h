//
// Copyright (c) 2010 Linaro Limited
//
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the MIT License which accompanies
// this distribution, and is available at
// http://www.opensource.org/licenses/mit-license.php
//
// Contributors:
//     Jesse Barker - original implementation.
//
#ifndef VEC_H_
#define VEC_H_

#include <iostream> // only needed for print() functions...
#include <math.h>

namespace LibMatrix
{
template<typename T>
class tvec2
{
public:
    tvec2() :
        x_(0),
        y_(0) {}
    tvec2(T t) :
        x_(t),
        y_(t) {}
    tvec2(T x, T y) :
        x_(x),
        y_(y) {}
    tvec2(const tvec2& v) :
        x_(v.x_),
        y_(v.y_) {}
    ~tvec2() {}

    void print() const
    {
        std::cout << "| " << x_ << " " << y_ << " |" << std::endl;
    }
    operator const T*() const { return &x_;}

    const T x() const { return x_; }
    const T y() const { return y_; }

    void x(const T& val) { x_ = val; }
    void y(const T& val) { y_ = val; }

    tvec2& operator=(const tvec2& rhs)
    {
        if (this != &rhs)
        {
            x_ = rhs.x_;
            y_ = rhs.y_;
        }
        return *this;
    }

    tvec2& operator/=(const T& rhs)
    {
        x_ /= rhs;
        y_ /= rhs;
        return *this;
    }

    const tvec2 operator/(const T& rhs)
    {
        return tvec2(*this) /= rhs;
    }

    tvec2& operator*=(const T& rhs)
    {
        x_ *= rhs;
        y_ *= rhs;
        return *this;
    }

    const tvec2 operator*(const T& rhs)
    {
        return tvec2(*this) *= rhs;
    }

    tvec2& operator+=(const T& rhs)
    {
        x_ += rhs;
        y_ += rhs;
        return *this;
    }
    
    const tvec2 operator+(const T& rhs)
    {
        return tvec2(*this) += rhs;
    }

    tvec2& operator+=(const tvec2& rhs)
    {
        x_ += rhs.x_;
        y_ += rhs.y_;
        return *this;
    }

    const tvec2 operator+(const tvec2& rhs)
    {
        return tvec2(*this) += rhs;
    }

    tvec2& operator-=(const T& rhs)
    {
        x_ -= rhs;
        y_ -= rhs;
        return *this;
    }
    
    const tvec2 operator-(const T& rhs)
    {
        return tvec2(*this) -= rhs;
    }

    tvec2& operator-=(const tvec2& rhs)
    {
        x_ -= rhs.x_;
        y_ -= rhs.y_;
        return *this;
    }

    const tvec2 operator-(const tvec2& rhs)
    {
        return tvec2(*this) -= rhs;
    }

    float length() const
    {
        return sqrt(dot(*this, *this));
    }

    void normalize()
    {
        float l = length();
        x_ /= l;
        y_ /= l;
    }

    static T dot(const tvec2& v1, const tvec2& v2)
    {
        return (v1.x_ * v2.x_) + (v1.y_ * v2.y_); 
    }

private:
    T x_;
    T y_;
};

template<typename T>
class tvec3
{
public:
    tvec3() :
        x_(0),
        y_(0),
        z_(0) {}
    tvec3(T t) :
        x_(t),
        y_(t),
        z_(t) {}
    tvec3(T x, T y, T z) :
        x_(x),
        y_(y),
        z_(z) {}
    tvec3(const tvec3& v) :
        x_(v.x_),
        y_(v.y_),
        z_(v.z_) {}
    ~tvec3() {}

    void print() const
    {
        std::cout << "| " << x_ << " " << y_ << " " << z_ << " |" << std::endl;
    }
    operator const T*() const { return &x_;}

    const T x() const { return x_; }
    const T y() const { return y_; }
    const T z() const { return z_; }

    void x(const T& val) { x_ = val; }
    void y(const T& val) { y_ = val; }
    void z(const T& val) { z_ = val; }

    tvec3& operator=(const tvec3& rhs)
    {
        if (this != &rhs)
        {
            x_ = rhs.x_;
            y_ = rhs.y_;
            z_ = rhs.z_;
        }
        return *this;
    }

    tvec3& operator/=(const T& rhs)
    {
        x_ /= rhs;
        y_ /= rhs;
        z_ /= rhs;
        return *this;
    }

    const tvec3 operator/(const T& rhs)
    {
        return tvec3(*this) /= rhs;
    }

    tvec3& operator*=(const T& rhs)
    {
        x_ *= rhs;
        y_ *= rhs;
        z_ *= rhs;
        return *this;
    }

    const tvec3 operator*(const T& rhs)
    {
        return tvec3(*this) *= rhs;
    }

    tvec3& operator+=(const T& rhs)
    {
        x_ += rhs;
        y_ += rhs;
        z_ += rhs;
        return *this;
    }

    const tvec3 operator+(const T& rhs)
    {
        return tvec3(*this) += rhs;
    }

    tvec3& operator+=(const tvec3& rhs)
    {
        x_ += rhs.x_;
        y_ += rhs.y_;
        z_ += rhs.z_;
        return *this;
    }

    const tvec3 operator+(const tvec3& rhs)
    {
        return tvec3(*this) += rhs;
    }

    tvec3& operator-=(const T& rhs)
    {
        x_ -= rhs;
        y_ -= rhs;
        z_ -= rhs;
        return *this;
    }

    const tvec3 operator-(const T& rhs)
    {
        return tvec3(*this) -= rhs;
    }

    tvec3& operator-=(const tvec3& rhs)
    {
        x_ -= rhs.x_;
        y_ -= rhs.y_;
        z_ -= rhs.z_;
        return *this;
    }

    const tvec3 operator-(const tvec3& rhs)
    {
        return tvec3(*this) -= rhs;
    }

    float length() const
    {
        return sqrt(dot(*this, *this));
    }

    void normalize()
    {
        float l = length();
        x_ /= l;
        y_ /= l;
        z_ /= l;
    }

    static T dot(const tvec3& v1, const tvec3& v2)
    {
        return (v1.x_ * v2.x_) + (v1.y_ * v2.y_) + (v1.z_ * v2.z_); 
    }

    static tvec3 cross(const tvec3& u, const tvec3& v)
    {
        return tvec3((u.y_ * v.z_) - (u.z_ * v.y_),
                    (u.z_ * v.x_) - (u.x_ * v.z_),
                    (u.x_ * v.y_) - (u.y_ * v.x_));
    }

private:
    T x_;
    T y_;
    T z_;
};

template<typename T>
class tvec4
{
public:
    tvec4() :
        x_(0),
        y_(0),
        z_(0),
        w_(0) {}
    tvec4(T t) :
        x_(t),
        y_(t),
        z_(t),
        w_(t) {}
    tvec4(T x, T y, T z, T w) :
        x_(x),
        y_(y),
        z_(z),
        w_(w) {}
    tvec4(const tvec4& v) :
        x_(v.x_),
        y_(v.y_),
        z_(v.z_),
        w_(v.w_) {}
    ~tvec4() {}

    void print() const
    {
        std::cout << "| " << x_ << " " << y_ << " " << z_ << " " << w_ << " |" << std::endl;
    }
    operator const T*() const { return &x_;}

    const T x() const { return x_; }
    const T y() const { return y_; }
    const T z() const { return z_; }
    const T w() const { return w_; }

    void x(const T& val) { x_ = val; }
    void y(const T& val) { y_ = val; }
    void z(const T& val) { z_ = val; }
    void w(const T& val) { w_ = val; }

    tvec4& operator=(const tvec4& rhs)
    {
        if (this != &rhs)
        {
            x_ = rhs.x_;
            y_ = rhs.y_;
            z_ = rhs.z_;
            w_ = rhs.w_;
        }
        return *this;
    }

    tvec4& operator/=(const T& rhs)
    {
        x_ /= rhs;
        y_ /= rhs;
        z_ /= rhs;
        w_ /= rhs;
        return *this;
    }

    const tvec4 operator/(const T& rhs)
    {
        return tvec4(*this) /= rhs;
    }

    tvec4& operator*=(const T& rhs)
    {
        x_ *= rhs;
        y_ *= rhs;
        z_ *= rhs;
        w_ *= rhs;
        return *this;
    }

    const tvec4 operator*(const T& rhs)
    {
        return tvec4(*this) *= rhs;
    }

    tvec4& operator+=(const T& rhs)
    {
        x_ += rhs;
        y_ += rhs;
        z_ += rhs;
        w_ += rhs;
        return *this;
    }

    const tvec4 operator+(const T& rhs)
    {
        return tvec4(*this) += rhs;
    }

    tvec4& operator+=(const tvec4& rhs)
    {
        x_ += rhs.x_;
        y_ += rhs.y_;
        z_ += rhs.z_;
        w_ += rhs.w_;
        return *this;
    }

    const tvec4 operator+(const tvec4& rhs)
    {
        return tvec4(*this) += rhs;
    }

    tvec4& operator-=(const T& rhs)
    {
        x_ -= rhs;
        y_ -= rhs;
        z_ -= rhs;
        w_ -= rhs;
        return *this;
    }

    const tvec4 operator-(const T& rhs)
    {
        return tvec4(*this) -= rhs;
    }

    tvec4& operator-=(const tvec4& rhs)
    {
        x_ -= rhs.x_;
        y_ -= rhs.y_;
        z_ -= rhs.z_;
        w_ -= rhs.w_;
        return *this;
    }

    const tvec4 operator-(const tvec4& rhs)
    {
        return tvec4(*this) -= rhs;
    }

    float length() const
    {
        return sqrt(dot(*this, *this));
    }

    void normalize()
    {
        float l = length();
        x_ /= l;
        y_ /= l;
        z_ /= l;
        w_ /= l;
    }

    static T dot(const tvec4& v1, const tvec4& v2)
    {
        return (v1.x_ * v2.x_) + (v1.y_ * v2.y_) + (v1.z_ * v2.z_) + (v1.w_ * v2.w_); 
    }

private:
    T x_;
    T y_;
    T z_;
    T w_;
};

//
// Convenience typedefs.  These are here to present a homogeneous view of these
// objects with respect to shader source.
//
typedef tvec2<float> vec2;
typedef tvec3<float> vec3;
typedef tvec4<float> vec4;

typedef tvec2<double> dvec2;
typedef tvec3<double> dvec3;
typedef tvec4<double> dvec4;

typedef tvec2<int> ivec2;
typedef tvec3<int> ivec3;
typedef tvec4<int> ivec4;

typedef tvec2<unsigned int> uvec2;
typedef tvec3<unsigned int> uvec3;
typedef tvec4<unsigned int> uvec4;

typedef tvec2<bool> bvec2;
typedef tvec3<bool> bvec3;
typedef tvec4<bool> bvec4;

} // namespace LibMatrix

#endif // VEC_H_
