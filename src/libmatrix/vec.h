//
// Copyright (c) 2010-2011 Linaro Limited
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
#include <array>
#include <bit>
#include <numeric>
#include <utility>

template<typename T>
concept scalar = std::integral<T> || std::floating_point<T>;

template<typename T>
concept fscalar = std::floating_point<T>;

template<typename T>
concept iscalar = std::integral<T>;

template<typename T>
concept uscalar = std::unsigned_integral<T>;

namespace LibMatrix
{
enum class align : size_t
{
	none     = 0,
	element  = 1,
	vector   = 2,
	adaptive = 3
};

// aligned n-element vector based on std:array compatible with every kind of SIMD optimization
template<scalar T, size_t N = 3, enum align A = align::adaptive, size_t N_POW2 = std::bit_ceil<size_t>(N)>
struct alignas(((N == N_POW2 && A != align::element) || A == align::vector) ? N_POW2 * sizeof(T) : sizeof(T)) tvec : std::array<T,N>
{
    tvec() { (*this).fill((T)0); }
    tvec(const T& t) { (*this).fill((T)t); }

    template<scalar... I> requires((sizeof...(I) > 1) && (sizeof...(I) <= N))
    tvec(const I... args) : std::array<T,N>{{ (T)args... }} {}

    template<size_t N_LHS, enum align A_RHS = align::adaptive> 
    operator tvec<T,N_LHS,A_RHS>() { return *reinterpret_cast<tvec<T,N_LHS,A_RHS>*>(this); }
    template<size_t N_LHS, enum align A_RHS = align::adaptive> 
    operator const tvec<T,N_LHS,A_RHS>() const { return *reinterpret_cast<const tvec<T,N_LHS,A_RHS>*>(this); }

    template<enum align A_RHS = align::adaptive>
    tvec(const tvec<T,3>& src, const T w = 1) requires (N > 3) { (*this).fill((T)0); (*this)[0] = src[0]; (*this)[1] = src[1]; (*this)[2] = src[2]; (*this)[3] = w; };

    void print() const
    {
        std::cout << "| ";
        for(T& i : (*this))
		std::cout << i << " ";
	std::cout << "|" << std::endl;
    }

    operator const T*() const { return (*this).data(); }

    // Get and set access members for the individual elements.
    const T x() const                 { return (*this)[0]; }
    const T y() const requires(N > 1) { return (*this)[1]; }
    const T z() const requires(N > 2) { return (*this)[2]; }
    const T w() const requires(N > 3) { return (*this)[3]; }

    void x(const T& val)                 { (*this)[0] = val; }
    void y(const T& val) requires(N > 1) { (*this)[1] = val; }
    void z(const T& val) requires(N > 2) { (*this)[2] = val; }
    void w(const T& val) requires(N > 3) { (*this)[3] = val; }

    const tvec<T,3,A> yzx() const requires(N > 2) { return { (*this)[1], (*this)[2], (*this)[0] }; }
    const tvec<T,3,A> zxy() const requires(N > 2) { return reinterpret_cast<tvec<T,3,A> const &>(*this); }
    const tvec<T,3,A> xyz(T w = 1) requires (N == 4) { w *= w(); return (w != (T)0 && w != (T)1) ? reinterpret_cast<tvec<T,3,A> const &>(*this) / w : reinterpret_cast<tvec<T,3,A> const &>(*this); }
    const tvec<T,3,A> yxz() const requires(N > 2) { return { (*this)[1], (*this)[0], (*this)[2] }; }
    const tvec<T,4,A> xyzw() const requires(N > 3) { return reinterpret_cast<tvec<T,4,A> const &>(*this); }

    const tvec<T,4,A> position() requires(N > 2) { return tvec<T,4,A>(xyz(), 1); }
    const tvec<T,4,A> direction() requires(N > 2) { return tvec<T,4,A>(xyz(), 0); }

    template<scalar T_RHS = T, size_t N_RHS = N, enum align A_RHS = A, scalar T_DST = decltype((T)1 * (T_RHS)1)>
    inline constexpr const tvec<T,N,A> operator*(const tvec<T_RHS,N_RHS,A_RHS>& rhs) const
    {
        tvec<T, std::min((*this).size(), rhs.size())> dst = {};
        std::transform((*this).cbegin(),(*this).cbegin() + dst.size(), rhs.cbegin(), dst.begin(), std::multiplies<>{});
	return dst;
    }
    template<scalar T_RHS = T, size_t N_RHS = N, enum align A_RHS = A, scalar T_DST = decltype((T)1 / (T_RHS)1)>
    inline constexpr const tvec<T,N,A> operator/(const tvec<T_RHS,N_RHS,A_RHS>& rhs) const
    {
        tvec<T, std::min((*this).size(), rhs.size())> dst = {};
	std::transform((*this).cbegin(),(*this).cbegin() + dst.size(), rhs.cbegin(), dst.begin(), std::divides<>{});
	return dst;
    }
    template<scalar T_RHS = T, size_t N_RHS = N, enum align A_RHS = A, scalar T_DST = decltype((T)1 + (T_RHS)1)>
    inline constexpr const tvec<T_DST,N,A> operator+(const tvec<T_RHS,N_RHS,A_RHS>& rhs) const
    {
        tvec<T, std::min((*this).size(), rhs.size())> dst = {};
	std::transform((*this).cbegin(),(*this).cbegin() + dst.size(), rhs.cbegin(), dst.begin(), std::plus<>{});
	return dst;
    }
    template<scalar T_RHS = T, size_t N_RHS = N, enum align A_RHS = A, scalar T_DST = decltype((T)1 - (T_RHS)1)>
    inline constexpr const tvec<T_DST,N,A> operator-(const tvec<T_RHS,N_RHS,A_RHS>& rhs) const
    {
        tvec<T, std::min((*this).size(), rhs.size())> dst = {};
	std::transform((*this).cbegin(),(*this).cbegin() + dst.size(), rhs.cbegin(), dst.begin(), std::minus<>{});
	return dst;
    }
    /* arithmetic scalar operators with constructor fill */
    template<scalar T_RHS = T>
    inline constexpr const tvec<T,N,A> operator*(const T_RHS& rhs) const
    {
        tvec<T, N, A> dst((T)rhs);
	std::transform((*this).cbegin(),(*this).cend(), dst.cbegin(), dst.begin(), std::multiplies<>{});
	return dst;
    }
    template<scalar T_RHS = T>
    inline constexpr const tvec<T,N,A> operator/(const T_RHS& rhs) const
    {
        tvec<T, N, A> dst((T)rhs);
	std::transform((*this).cbegin(),(*this).cend(), dst.cbegin(), dst.begin(), std::divides<>{});
	return dst;
    }
    template<scalar T_RHS = T>
    inline constexpr const tvec<T,N,A> operator+(const T_RHS& rhs) const
    {
        tvec<T, N, A> dst((T)rhs);
	std::transform((*this).cbegin(),(*this).cend(), dst.cbegin(), dst.begin(), std::plus<>{});
	return dst;
    }
    template<scalar T_RHS = T>
    inline constexpr const tvec<T,N,A> operator-(const T_RHS& rhs) const
    {
        tvec<T, N, A> dst((T)rhs);
	std::transform((*this).cbegin(),(*this).cend(), dst.cbegin(), dst.begin(), std::minus<>{});
	return dst;
    }


    template<scalar T_RHS = T, size_t N_RHS = N, enum align A_RHS = A>
    tvec<T,N,A>& operator*=(const tvec<T_RHS,N_RHS,A_RHS>& rhs) { (*this) = (*this) * rhs; return (*this); }
    template<scalar T_RHS = T, size_t N_RHS = N, enum align A_RHS = A>
    tvec<T,N,A>& operator/=(const tvec<T_RHS,N_RHS,A_RHS>& rhs) { (*this) = (*this) / rhs; return (*this); }
    template<scalar T_RHS = T, size_t N_RHS = N, enum align A_RHS = A>
    tvec<T,N,A>& operator+=(const tvec<T_RHS,N_RHS,A_RHS>& rhs) { (*this) = (*this) + rhs; return (*this); }
    template<scalar T_RHS = T, size_t N_RHS = N, enum align A_RHS = A>
    tvec<T,N,A>& operator-=(const tvec<T_RHS,N_RHS,A_RHS>& rhs) { (*this) = (*this) - rhs; return (*this); }

    template<scalar T_RHS = T>
    tvec<T,N,A>& operator*=(const T_RHS& rhs) { (*this) *= tvec<T,N,A>((T)rhs); return *this; }
    template<scalar T_RHS = T>
    tvec<T,N,A>& operator/=(const T_RHS& rhs) { (*this) /= tvec<T,N,A>((T)rhs); return *this; }
    template<scalar T_RHS = T>
    tvec<T,N,A>& operator+=(const T_RHS& rhs) { (*this) += tvec<T,N,A>((T)rhs); return *this; }
    template<scalar T_RHS = T>
    tvec<T,N,A>& operator-=(const T_RHS& rhs) { (*this) -= tvec<T,N,A>((T)rhs); return *this; }

    // Compute the length of this and return it.
    float length() const
    {
        return sqrt(dot(*this, *this));
    }

    // Make this a unit vector.
    void normalize()
    {
        float l = length();
	if(l != 0 && l != 1)
		(*this) /= l;
    }

    // Compute the dot product of two vectors.
    template<scalar T_V1 = float, scalar T_V2 = float, size_t N_V1 = 3, size_t N_V2 = 3, enum align A_V1 = align::adaptive, enum align A_V2 = align::adaptive, scalar T_DST = decltype((T_V1)1*(T_V2)1 + (T_V1)1*(T_V2)1)>
    static T_DST dot(const tvec<T_V1,N_V1,A_V1>& v1, const tvec<T_V2,N_V2,A_V2>& v2)
    {
    	tvec<T_DST, std::min(N_V1,N_V2)> dst = v1 * v2;
	return std::accumulate(dst.begin(), dst.end(), (T_DST)0);
    }

    // Compute the cross product of two vectors.
    template<scalar T_U = float, scalar T_V = float, size_t N_U = 3, size_t N_V = 3, enum align A_U = align::adaptive, enum align A_V = align::adaptive, scalar T_DST = decltype((T_U)1*(T_V)1)>
    static tvec<T_DST,3,A> cross(const tvec<T_U,N_U,A_U>& u, const tvec<T_V,N_V,A_V>& v)
    {
        return (u * v.yzx() - u.yzx() * v).yzx();
    }
};

template<scalar T, enum align A = align::adaptive>
using tvec2 = tvec<T, 2, A>;
template<scalar T, enum align A = align::adaptive>
using tvec3 = tvec<T, 3, A>;
template<scalar T, enum align A = align::adaptive>
using tvec4 = tvec<T, 4, A>;

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

// Global operators to allow for things like defining a new vector in terms of
// a product of a scalar and a vector
template<scalar T, size_t N>
const LibMatrix::tvec<T,N> operator*(const T t, const LibMatrix::tvec<T,N>& v)
{
    return v * t;
}

#endif // VEC_H_
