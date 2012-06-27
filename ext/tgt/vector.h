/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2008 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#ifndef TGT_VECTOR_H
#define TGT_VECTOR_H

#include <iostream>
#include <algorithm>
#include <cmath>

#include "tgt/config.h"
#include "tgt/math.h"

/*
    Performance remarks:

    - Most functions and operators are programmed with loops.
    Compilers can unroll that.
    gcc for instance will do this with -funroll-loops.

    - Modern compilers can even vectorize these loops
    and do these caclulations with SIMD-instructions.
    (does not work with gcc 4.0 and 4.1, maybe later versions will do it)

    - Almost all functions are inline!
    This can have signifcant advantages in codegeneration

    - The return-value optimization which is supported by all
    modern compilers (gcc, icc and msvc for instance)
    will optimize unnecessary constructor-overhead away.
*/

namespace tgt {

/**
    This is a 2-dimensional Vector class similar to vec2 of GLSL.
    All standard-operators are overloaded and go componentwisely
    similar to GLSL. For dotproduct use the function dot.
    <br>
    <br>
    So you can do this: (vec is a Vector\<T\>, base is T) <br>
    <br>
    vec = vec  + vec <br>
    vec = base + vec <br>
    vec = vec  + base <br>
    <br>
    vec += vec <br>
    vec += base <br>
    <br>
    <br>
    vec = vec  - vec <br>
    vec = base - vec <br>
    vec = vec  - base <br>
    <br>
    vec -= vec <br>
    vec -= base <br>
    <br>
    <br>
    vec = vec  * vec <br>
    vec = base * vec <br>
    vec = vec  * base <br>
    <br>
    vec *= vec <br>
    vec *= base <br>
    <br>
    <br>
    vec = vec  / vec <br>
    vec = base / vec <br>
    vec = vec  / base <br>
    <br>
    vec /= vec <br>
    vec /= base <br>
    <br>
    <br>
    vec = vec  % vec <br>
    vec = base % vec <br>
    vec = vec  % base <br>
    <br>
    vec %= vec <br>
    vec %= base <br>
    <br>
    <br>
    vec = vec  & vec <br>
    vec = base & vec <br>
    vec = vec  & base <br>
    <br>
    vec &= vec <br>
    vec &= base <br>
    <br>
    <br>
    vec = vec  | vec <br>
    vec = base | vec <br>
    vec = vec  | base <br>
    <br>
    vec |= vec <br>
    vec |= base <br>
    <br>
    <br>
    vec = vec  ^ vec <br>
    vec = base ^ vec <br>
    vec = vec  ^ base <br>
    <br>
    vec ^= vec <br>
    vec ^= base <br>
    <br>
    <br>
    bool operator == (vec, vec) <br>
    bool operator != (vec, vec) <br>
    bvec lessThan(vec, vec) <br>
    bvec lessThanEqual(vec, vec) <br>
    bvec greaterThan(vec, vec) <br>
    bvec greaterThanEqual(vec, vec) <br>
    bvec equal(vec, vec) <br>
    bvec notEqual(vec, vec) <br>
    <br>
    <br>
    base dot(vec, vec) <br>
    vec3 cross(vec3, vec3) <br>
    base length(vec) <br>
    base lengthSq(vec) <br>
    base distance(vec, vec) <br>
    base distanceSq(vec, vec) <br>
    vec normalize(vec) <br>
    base min(vec) <br>
    vec min(vec, vec) <br>
    vec min(vec, base) <br>
    base max(vec) <br>
    vec max(vec, vec) <br>
    vec max(vec, base) <br>
    vec clamp(vec in, vec min, vec max) <br>
    vec clamp(vec in, base min, base max) <br>
    vec abs(vec) <br>
    <br>
    <br>
    horizontal calculations: <br>
    T hadd(vec) <br>
    T hsub(vec) <br>
    T hmul(vec) <br>
    T hdiv(vec) <br>
    T hmod(vec) <br>
    T hand(vec) <br>
    T hor(vec)  <br>
    T hxor(vec) <br>
*/
template<class T>
struct Vector2 {
    enum {
        size = 2
    };

    union {
        struct {
            T x;
            T y;
        };
        struct {
            T s;
            T t;
        };
        struct {
            T r;
            T g;
        };
        T elem[size];
    };

    /*
        constructors
    */
    /// Default constructor
    Vector2() {}
    /// Init all elements with the same value
    explicit Vector2(T t) {
        for (size_t i = 0; i < size; ++i)
            elem[i] = t;
    }
    /// Init from array with equal size
    explicit Vector2(const T* t) {
        for (size_t i = 0; i < size; ++i)
            elem[i] = t[i];
    }
    /// Init componentwisely
    Vector2(T t1, T t2) {
        elem[0] = t1;
        elem[1] = t2;
    }

    /// Init with another Vector of another type
    template<class U>
    Vector2(const Vector2<U>& v) {
        for (size_t i = 0; i < v.size; ++i)
            elem[i] = T(v.elem[i]);
    }

    /// Index operator
    const T& operator [] (size_t index) const {
        return elem[index];
    }
    /// Index operator
    T& operator [] (size_t index) {
        return elem[index];
    }

    static Vector2<T> zero;
};

/// init statics
template<class T>
Vector2<T> Vector2<T>::zero = Vector2<T>(T(0), T(0));


/**
    This is a 3-dimensional Vector class similar to vec3 of GLSL.
    All standard-operators are overloaded and go componentwisely
    similar to GLSL. For dot- and crossproduct use the functions dot and cross.
*/
template<class T>
struct Vector3 {
    enum {
        size = 3
    };

    union {
        struct {
            T x;
            T y;
            T z;
        };
        struct {
            T s;
            T t;
            T p;
        };
        struct {
            T r;
            T g;
            T b;
        };
        T elem[size];
    };

    /*
        constructors
    */
    /// default constructor
    Vector3() {}
    /// Init all elements with the same value
    explicit Vector3(T t) {
        for (size_t i = 0; i < size; ++i)
            elem[i] = t;
    }
    /// Init from array with equal size
    explicit Vector3(const T* t) {
        for (size_t i = 0; i < size; ++i)
            elem[i] = t[i];
    }
    /// Init componentwisely
    Vector3(T t1, T t2, T t3) {
        elem[0] = t1;
        elem[1] = t2;
        elem[2] = t3;
    }
    /// Init x and y with v, z with _z
    Vector3(const Vector2<T>& v, T _z) {
        elem[0] = v.elem[0];
        elem[1] = v.elem[1];
        elem[2] = _z;
    }
    /// Init y and z with vec2, x with _x
    Vector3(T _x, const Vector2<T>& v) {
        elem[0] = _x;
        elem[1] = v.elem[1];
        elem[2] = v.elem[2];
    }

    /// Init with another Vector of another type
    template<class U>
    Vector3(const Vector3<U>& v) {
        for (size_t i = 0; i < v.size; ++i)
            elem[i] = T(v.elem[i]);
    }

    /// Index operator
    const T& operator [] (size_t index) const {
        return elem[index];
    }
    /// Index operator
    T& operator [] (size_t index) {
        return elem[index];
    }

/*
    sub-vector getters
*/
    const Vector2<T>& xy() const {
        return *((Vector2<T>*) elem);
    }
    Vector2<T>& xy() {
        return *((Vector2<T>*) elem);
    }
    const Vector2<T>& yz() const {
        return *((Vector2<T>*) (elem + 1));
    }
    Vector2<T>& yz() {
        return *((Vector2<T>*) (elem + 1));
    }

/*
    statics
*/
    static Vector3<T> zero;
};

/// init statics
template<class T>
Vector3<T> Vector3<T>::zero = Vector3<T>(T(0), T(0), T(0));

/**
    This is a 4-dimensional Vector class similar to vec4 of GLSL.
    All standard-operators are overloaded and go componentwisely
    similar to GLSL. For dotproduct use the function dot.
*/
template<class T>
struct Vector4 {
    enum {
        size = 4
    };

    union {
        struct {
            T x;
            T y;
            T z;
            T w;
        };
        struct {
            T s;
            T t;
            T p;
            T q;
        };
        struct {
            T r;
            T g;
            T b;
            T a;
        };
        T elem[size];
    };

/*
    constructors
*/
    /// default constructor
    Vector4() {}
    /// Init all elements with the same value
    explicit Vector4(T t) {
        for (size_t i = 0; i < size; ++i)
            elem[i] = t;
    }
    /// Init from array with equal size
    explicit Vector4(const T* t) {
        for (size_t i = 0; i < size; ++i)
            elem[i] = t[i];
    }
    /// Init componentwisely
    Vector4(T t1, T t2, T t3, T t4) {
        elem[0] = t1;
        elem[1] = t2;
        elem[2] = t3;
        elem[3] = t4;
    }
    /// Init x and y with v1, z and w with v2
    Vector4(const Vector2<T>& v1, const Vector2<T>& v2) {
        elem[0] = v1.elem[0];
        elem[1] = v1.elem[1];
        elem[2] = v2.elem[0];
        elem[3] = v2.elem[1];
    }
    /// Init x and y with vec2, z and w with _z and _w
    Vector4(const Vector2<T>& v, T _z, T _w) {
        elem[0] = v.elem[0];
        elem[1] = v.elem[1];
        elem[2] = _z;
        elem[3] = _w;
    }
    /// Init x and y with _x and _y, z and w with _z and _w
    Vector4(T _x, T _y, const Vector2<T>& v) {
        elem[0] = _x;
        elem[1] = _y;
        elem[2] = v.elem[0];
        elem[3] = v.elem[1];
    }
    /// Init x, y, z with v and w with _w
    Vector4(const Vector3<T>& v, T _w) {
        elem[0] = v.elem[0];
        elem[1] = v.elem[1];
        elem[2] = v.elem[2];
        elem[3] = _w;
    }
    /// Init x with _x and y, z and w with v
    Vector4(T _x, const Vector2<T>& v) {
        elem[0] = _x;
        elem[1] = v.elem[0];
        elem[2] = v.elem[1];
        elem[3] = v.elem[2];
    }

    /// Init with another Vector of another type
    template<class U>
    Vector4(const Vector4<U>& v) {
        for (size_t i = 0; i < v.size; ++i)
            elem[i] = T(v.elem[i]);
    }

/*
    index operator
*/
    /// Index operator
    const T& operator [] (size_t index) const {
        return elem[index];
    }
    /// Index operator
    T& operator [] (size_t index) {
        return elem[index];
    }

/*
    sub-vector getters
*/

    const Vector2<T>& xy() const {
        return *((Vector2<T>*) elem);
    }
    const Vector2<T>& yz() const {
        return *((Vector2<T>*) (elem + 1));
    }
    const Vector2<T>& zw() const {
        return *((Vector2<T>*) (elem + 2));
    }
    const Vector3<T>& xyz() const {
        return *((Vector3<T>*) elem);
    }
    const Vector3<T>& yzw() const {
        return *((Vector3<T>*) (elem + 1));
    }
    Vector2<T>& xy() {
        return *((Vector2<T>*) elem);
    }
    Vector2<T>& yz() {
        return *((Vector2<T>*) (elem + 1));
    }
    Vector2<T>& zw() {
        return *((Vector2<T>*) (elem + 2));
    }
    Vector3<T>& xyz() {
        return *((Vector3<T>*) elem);
    }
    Vector3<T>& yzw() {
        return *((Vector3<T>*) (elem + 1));
    }

/*
    statics
*/
    static const Vector4<float> white;
    static const Vector4<float> black;
    static const Vector4<float> gray;
    static const Vector4<float> red;
    static const Vector4<float> green;
    static const Vector4<float> blue;
    static const Vector4<float> brown;
    static const Vector4<float> yellow;

    static Vector4<T> zero;
};


template<class T> const Vector4<float> Vector4<T>::white (1.f, 1.f, 1.f, 1.f);
template<class T> const Vector4<float> Vector4<T>::black (0.f, 0.f, 0.f, 1.f);
template<class T> const Vector4<float> Vector4<T>::gray  (.5f, .5f, .5f, 1.f);
template<class T> const Vector4<float> Vector4<T>::red   (1.f, 0.f, 0.f, 1.f);
template<class T> const Vector4<float> Vector4<T>::green (0.f, 1.f, 0.f, 1.f);
template<class T> const Vector4<float> Vector4<T>::blue  (0.f, 0.f, 1.f, 1.f);
template<class T> const Vector4<float> Vector4<T>::yellow(1.f, 1.f, 0.f, 1.f);
template<class T> const Vector4<float> Vector4<T>::brown (0.6f, 0.4f, 0.f, 1.f);

/// init statics
template<class T>
Vector4<T> Vector4<T>::zero = Vector4<T>(T(0), T(0), T(0), T(0));

/*
    typedefs for easy usage
*/

typedef Vector2<float>   Vector2f;
typedef Vector2<double>  Vector2d;
typedef Vector3<float>   Vector3f;
typedef Vector3<double>  Vector3d;
typedef Vector4<float>   Vector4f;
typedef Vector4<double>  Vector4d;

typedef Vector3f        Vector;
typedef Vector4f        Color;

/*
    types like in GLSL
*/

typedef Vector2<float>   vec2;
typedef Vector3<float>   vec3;
typedef Vector4<float>   vec4;
typedef Vector2<double> dvec2; // currently not supported in GLSL but a reserved keyword
typedef Vector3<double> dvec3; // currently not supported in GLSL but a reserved keyword
typedef Vector4<double> dvec4; // currently not supported in GLSL but a reserved keyword
typedef Vector2<int>    ivec2;
typedef Vector3<int>    ivec3;
typedef Vector4<int>    ivec4;
typedef Vector2<bool>   bvec2;
typedef Vector3<bool>   bvec3;
typedef Vector4<bool>   bvec4;

/*
    uint8_t types
*/

typedef Vector3<uint8_t> col3;
typedef Vector4<uint8_t> col4;

/*
    Prepare to implement 3 * 5 * 8 = 120 operators and dozens of functions
    the lazy way with evil voodoo macro magic
*/

#define VEC_OP_VEC \
template<class T> inline BASE_TYPE<T> operator VEC_OP (const BASE_TYPE<T>& v1, const BASE_TYPE<T>& v2) { \
    BASE_TYPE<T> vRes; \
    for (size_t i = 0; i < v1.size; ++i) \
        vRes.elem[i] = v1.elem[i] VEC_OP v2.elem[i]; \
    return vRes; \
}

#define VEC_OP_BASE \
template<class T> inline BASE_TYPE<T> operator VEC_OP (const BASE_TYPE<T>& v, T t) { \
    BASE_TYPE<T> vRes; \
    for (size_t i = 0; i < v.size; ++i) \
        vRes.elem[i] = v.elem[i] VEC_OP t; \
    return vRes; \
}

#define BASE_OP_VEC \
template<class T> inline BASE_TYPE<T> operator VEC_OP (T t, const BASE_TYPE<T>& v) { \
    BASE_TYPE<T> vRes; \
    for (size_t i = 0; i < v.size; ++i) \
        vRes.elem[i] = t VEC_OP v.elem[i]; \
    return vRes; \
}

#define VEC_OPEQ_VEC \
template<class T> inline BASE_TYPE<T>& operator VEC_OPEQ (BASE_TYPE<T>& v1, const BASE_TYPE<T>& v2) { \
    for (size_t i = 0; i < v1.size; ++i) \
        v1.elem[i] VEC_OPEQ v2.elem[i]; \
    return v1; \
}

#define VEC_OPEQ_BASE \
template<class T> inline BASE_TYPE<T>& operator VEC_OPEQ (BASE_TYPE<T>& v, T t) { \
    for (size_t i = 0; i < v.size; ++i) \
        v.elem[i] VEC_OPEQ t; \
    return v; \
}

#define IMPLEMENT_OPERATORS \
        VEC_OP_VEC \
        VEC_OP_BASE \
        BASE_OP_VEC \
        VEC_OPEQ_VEC \
        VEC_OPEQ_BASE

/*
    relational operators
*/
#define VEC_LESS_THAN \
template<class T> \
inline BASE_TYPE<bool> lessThan (const BASE_TYPE<T>& v1, const BASE_TYPE<T>& v2) { \
    BASE_TYPE<bool> vRes; \
    for (size_t i = 0; i < vRes.size; ++i) \
        vRes.elem[i] = (v1.elem[i] < v2.elem[i]); \
    return vRes; \
}

#define VEC_LESS_THAN_EQUAL \
template<class T> \
inline BASE_TYPE<bool> lessThanEqual (const BASE_TYPE<T>& v1, const BASE_TYPE<T>& v2) { \
    BASE_TYPE<bool> vRes; \
    for (size_t i = 0; i < vRes.size; ++i) \
        vRes.elem[i] = (v1.elem[i] <= v2.elem[i]); \
    return vRes; \
}

#define VEC_GREATER_THAN \
template<class T> \
inline BASE_TYPE<bool> greaterThan (const BASE_TYPE<T>& v1, const BASE_TYPE<T>& v2) { \
    BASE_TYPE<bool> vRes; \
    for (size_t i = 0; i < vRes.size; ++i) \
        vRes.elem[i] = (v1.elem[i] > v2.elem[i]); \
    return vRes; \
}

#define VEC_GREATER_THAN_EQUAL \
template<class T> \
inline BASE_TYPE<bool> greaterThanEqual (const BASE_TYPE<T>& v1, const BASE_TYPE<T>& v2) { \
    BASE_TYPE<bool> vRes; \
    for (size_t i = 0; i < vRes.size; ++i) \
        vRes.elem[i] = (v1.elem[i] >= v2.elem[i]); \
    return vRes; \
}

#define VEC_EQUAL \
template<class T> \
inline BASE_TYPE<bool> equal (const BASE_TYPE<T>& v1, const BASE_TYPE<T>& v2) { \
    BASE_TYPE<bool> vRes; \
    for (size_t i = 0; i < vRes.size; ++i) \
        vRes.elem[i] = (v1.elem[i] == v2.elem[i]); \
    return vRes; \
}

#define VEC_NOT_EQUAL \
template<class T> \
inline BASE_TYPE<bool> notEqual (const BASE_TYPE<T>& v1, const BASE_TYPE<T>& v2) { \
    BASE_TYPE<bool> vRes; \
    for (size_t i = 0; i < vRes.size; ++i) \
        vRes.elem[i] = (v1.elem[i] != v2.elem[i]); \
    return vRes; \
}

#define VEC_REL_OP_EQUAL \
template<class T> \
inline bool operator == (const BASE_TYPE<T>& v1, const BASE_TYPE<T>& v2) { \
    bool result = true; \
    for (size_t i = 0; i < v1.size; ++i) \
        result &= (v1.elem[i] == v2.elem[i]); \
    return result; \
}

#define VEC_REL_OP_NOT_EQUAL \
template<class T> \
inline bool operator != (const BASE_TYPE<T>& v1, const BASE_TYPE<T>& v2) { \
    return !(v1 == v2); \
}

/*
    further vector functions
*/

#define VEC_DOT_PRODUCT \
template<class T> inline T dot(const BASE_TYPE<T>& v1, const BASE_TYPE<T>& v2) { \
    T result = 0; \
    for (size_t i = 0; i < v1.size; ++i) \
        result += v1.elem[i] * v2.elem[i]; \
    return result; \
}

#define VEC_LENGTH_SQ \
template<class T> inline T lengthSq(const BASE_TYPE<T>& v) { \
    T result = 0; \
    for (size_t i = 0; i < v.size; ++i) \
        result += (v.elem[i] * v.elem[i]); \
    return result; \
}

#define VEC_LENGTH \
template<class T> inline T length(const BASE_TYPE<T>& v) { \
    return sqrt(lengthSq(v)); \
}

#define VEC_NORMALIZE \
template<class T> inline BASE_TYPE<T> normalize(const BASE_TYPE<T>& v) { \
    BASE_TYPE<T> vRes(v); \
    T recipLength = 1 / length(v); \
    vRes *= recipLength; \
    return vRes; \
}

#define VEC_DISTANCE \
template<class T> inline T distance(const BASE_TYPE<T>& v1, const BASE_TYPE<T>& v2) { \
    return length(v2 - v1); \
}

#define VEC_DISTANCE_SQ \
template<class T> inline T distanceSq(const BASE_TYPE<T>& v1, const BASE_TYPE<T>& v2) { \
    return lengthSq(v2 - v1); \
}

#define VEC_UNARY_MINUS \
template<class T> inline BASE_TYPE<T> operator - (const BASE_TYPE<T>& v) { \
    BASE_TYPE<T> vRes(v); \
    for (size_t i = 0; i < v.size; ++i) \
        vRes.elem[i] = -v.elem[i]; \
    return vRes; \
}

#define VEC_MIN \
template<class T> inline BASE_TYPE<T> min(const BASE_TYPE<T>& v1, const BASE_TYPE<T>& v2) { \
    BASE_TYPE<T> vRes; \
    for (size_t i = 0; i < v1.size; ++i) \
        vRes.elem[i] = std::min(v1.elem[i], v2.elem[i]); \
    return vRes; \
}

#define VEC_MAX \
template<class T> inline BASE_TYPE<T> max(const BASE_TYPE<T>& v1, const BASE_TYPE<T>& v2) { \
    BASE_TYPE<T> vRes; \
    for (size_t i = 0; i < v1.size; ++i) \
        vRes.elem[i] = std::max(v1.elem[i], v2.elem[i]); \
    return vRes; \
}

#define VEC_MIN_SINGLE \
template<class T> inline BASE_TYPE<T> min(const BASE_TYPE<T>& v, T t) { \
    BASE_TYPE<T> vRes; \
    for (size_t i = 0; i < v.size; ++i) \
        vRes.elem[i] = std::min(v.elem[i], t); \
    return vRes; \
}

#define VEC_MAX_SINGLE \
template<class T> inline BASE_TYPE<T> max(const BASE_TYPE<T>& v, T t) { \
    BASE_TYPE<T> vRes; \
    for (size_t i = 0; i < v.size; ++i) \
        vRes.elem[i] = std::max(v.elem[i], t); \
    return vRes; \
}

#define VEC_MIN_SELF \
template<class T> inline T min(const BASE_TYPE<T>& v) { \
    T res = v.elem[0]; \
    for (size_t i = 1; i < v.size; ++i) \
        res = std::min(v.elem[i], res); \
    return res; \
}

#define VEC_MAX_SELF \
template<class T> inline T max(const BASE_TYPE<T>& v) { \
    T res = v.elem[0]; \
    for (size_t i = 1; i < v.size; ++i) \
        res = std::max(v.elem[i], res); \
    return res; \
}

#define VEC_CLAMP \
template<class T> inline BASE_TYPE<T> clamp(const BASE_TYPE<T>& v, const BASE_TYPE<T>& minVal, const BASE_TYPE<T>& maxVal) { \
    return min( max(v, minVal), maxVal ); \
}

#define VEC_CLAMP_SINGLE \
template<class T> inline BASE_TYPE<T> clamp(const BASE_TYPE<T>& v, T minVal, T maxVal) { \
    return min( max(v, minVal), maxVal ); \
}

#define VEC_CEIL \
template<class T> inline BASE_TYPE<T> ceil(const BASE_TYPE<T>& v) { \
    using std::ceil; \
    BASE_TYPE<T> result; \
    for (size_t i = 0; i < v.size; ++i) \
        result[i] = ceil(v.elem[i]); \
    return result; \
}

#define VEC_FLOOR \
template<class T> inline BASE_TYPE<T> floor(const BASE_TYPE<T>& v) { \
    using std::floor; \
    BASE_TYPE<T> result; \
    for (size_t i = 0; i < v.size; ++i) \
        result[i] = floor(v.elem[i]); \
    return result; \
}

#define VEC_FIX \
template<class T> inline BASE_TYPE<T> fix(const BASE_TYPE<T>& v) { \
    BASE_TYPE<T> result; \
    for (size_t i = 0; i < v.size; ++i) \
        result[i] = fix(v.elem[i]); \
    return result; \
}

#define VEC_ROUND \
template<class T> inline BASE_TYPE<T> round(const BASE_TYPE<T>& v) { \
    BASE_TYPE<T> result; \
    for (size_t i = 0; i < v.size; ++i) \
        result[i] = round(v.elem[i]); \
    return result; \
}

#define VEC_SIGN \
template<class T> inline BASE_TYPE<T> sign(const BASE_TYPE<T>& v) { \
    BASE_TYPE<T> result; \
    for (size_t i = 0; i < v.size; ++i) \
        result[i] = sign(v.elem[i]); \
    return result; \
}

#define VEC_ICEIL \
template<class T> inline BASE_TYPE<int> iceil(const BASE_TYPE<T>& v) { \
    BASE_TYPE<int> result; \
    for (size_t i = 0; i < v.size; ++i) \
        result[i] = iceil(v.elem[i]); \
    return result; \
}

#define VEC_IFLOOR \
template<class T> inline BASE_TYPE<int> ifloor(const BASE_TYPE<T>& v) { \
    BASE_TYPE<int> result; \
    for (size_t i = 0; i < v.size; ++i) \
        result[i] = ifloor(v.elem[i]); \
    return result; \
}

#define VEC_IROUND \
template<class T> inline BASE_TYPE<int> iround(const BASE_TYPE<T>& v) { \
    BASE_TYPE<int> result; \
    for (size_t i = 0; i < v.size; ++i) \
        result[i] = iround(v.elem[i]); \
    return result; \
}

#define VEC_IFIX \
template<class T> inline BASE_TYPE<int> ifix(const BASE_TYPE<T>& v) { \
    BASE_TYPE<int> result; \
    for (size_t i = 0; i < v.size; ++i) \
        result[i] = ifix(v.elem[i]); \
    return result; \
}

#define VEC_ISIGN \
template<class T> inline BASE_TYPE<int> isign(const BASE_TYPE<T>& v) { \
    BASE_TYPE<int> result; \
    for (size_t i = 0; i < v.size; ++i) \
        result[i] = isign(v.elem[i]); \
    return result; \
}

#define VEC_MIX \
template<class T> inline BASE_TYPE<T> mix(const BASE_TYPE<T>& v, const BASE_TYPE<T>& v2, T t) { \
    BASE_TYPE<T> result; \
    for (size_t i = 0; i < v.size; ++i) \
        result.elem[i] = (v.elem[i]*(1-t)) + (v2.elem[i]*t); \
    return result; \
}

/*
    horizontally working functions
*/

#define VEC_HADD \
template<class T> inline T hadd(const BASE_TYPE<T>& v) { \
    T result = v.elem[0]; \
    for (size_t i = 1; i < v.size; ++i) \
        result += v.elem[i]; \
    return result; \
}

#define VEC_HSUB \
template<class T> inline T hsub(const BASE_TYPE<T>& v) { \
    T result = v.elem[0]; \
    for (size_t i = 1; i < v.size; ++i) \
        result -= v.elem[i]; \
    return result; \
}

#define VEC_HMUL \
template<class T> inline T hmul(const BASE_TYPE<T>& v) { \
    T result = v.elem[0]; \
    for (size_t i = 1; i < v.size; ++i) \
        result *= v.elem[i]; \
    return result; \
}

#define VEC_HDIV \
template<class T> inline T hdiv(const BASE_TYPE<T>& v) { \
    T result = v.elem[0]; \
    for (size_t i = 1; i < v.size; ++i) \
        result /= v.elem[i]; \
    return result; \
}

#define VEC_HMOD \
template<class T> inline T hmod(const BASE_TYPE<T>& v) { \
    T result = v.elem[0]; \
    for (size_t i = 1; i < v.size; ++i) \
        result %= v.elem[i]; \
    return result; \
}

#define VEC_HAND \
template<class T> inline bool hand(const BASE_TYPE<T>& v) { \
    bool result = v.elem[0] && v.elem[1]; \
    for (size_t i = 2; i < v.size; ++i) \
        result &= v.elem[i]; \
    return result; \
}

#define VEC_HOR \
template<class T> inline bool hor(const BASE_TYPE<T>& v) { \
    T result = v.elem[0] || v.elem[1]; \
    for (size_t i = 2; i < v.size; ++i) \
        result |= v.elem[i]; \
    return result; \
}

#define VEC_HXOR \
template<class T> inline bool hxor(const BASE_TYPE<T>& v) { \
    T result = v.elem[0] ^ v.elem[1]; \
    for (size_t i = 2; i < v.size; ++i) \
        result ^= v.elem[i]; \
    return result; \
}

#define IMPLEMENT_VEC_FUNCTIONS \
    VEC_DOT_PRODUCT  \
    VEC_LENGTH_SQ \
    VEC_LENGTH \
    VEC_NORMALIZE \
    VEC_DISTANCE \
    VEC_DISTANCE_SQ \
    VEC_UNARY_MINUS \
    VEC_MIN \
    VEC_MAX \
    VEC_FLOOR \
    VEC_CEIL \
    VEC_FIX \
    VEC_ROUND \
    VEC_SIGN \
    VEC_IFLOOR \
    VEC_ICEIL \
    VEC_IFIX \
    VEC_IROUND \
    VEC_ISIGN \
    VEC_MIN_SINGLE \
    VEC_MAX_SINGLE \
    VEC_MIN_SELF \
    VEC_MAX_SELF \
    VEC_CLAMP \
    VEC_CLAMP_SINGLE \
    VEC_REL_OP_EQUAL  \
    VEC_REL_OP_NOT_EQUAL \
    VEC_LESS_THAN \
    VEC_LESS_THAN_EQUAL \
    VEC_GREATER_THAN \
    VEC_GREATER_THAN_EQUAL \
    VEC_EQUAL \
    VEC_NOT_EQUAL \
    VEC_HADD \
    VEC_HSUB \
    VEC_HMUL \
    VEC_HDIV \
    VEC_HMOD \
    VEC_HAND \
    VEC_HOR \
    VEC_HXOR \
    VEC_MIX


/*
    Implementation of Vector2<T> operators
*/

#define BASE_TYPE Vector2
    #define VEC_OP +
    #define VEC_OPEQ +=
        IMPLEMENT_OPERATORS
    #undef  VEC_OP
    #undef  VEC_OPEQ

    #define VEC_OP -
    #define VEC_OPEQ -=
        IMPLEMENT_OPERATORS
    #undef  VEC_OP
    #undef  VEC_OPEQ

    #define VEC_OP *
    #define VEC_OPEQ *=
        IMPLEMENT_OPERATORS
    #undef  VEC_OP
    #undef  VEC_OPEQ

    #define VEC_OP /
    #define VEC_OPEQ /=
        IMPLEMENT_OPERATORS
    #undef  VEC_OP
    #undef  VEC_OPEQ

    #define VEC_OP %
    #define VEC_OPEQ %=
        IMPLEMENT_OPERATORS
    #undef  VEC_OP
    #undef  VEC_OPEQ

    #define VEC_OP &
    #define VEC_OPEQ &=
        IMPLEMENT_OPERATORS
    #undef  VEC_OP
    #undef  VEC_OPEQ

    #define VEC_OP |
    #define VEC_OPEQ |=
        IMPLEMENT_OPERATORS
    #undef  VEC_OP
    #undef  VEC_OPEQ

    #define VEC_OP ^
    #define VEC_OPEQ ^=
        IMPLEMENT_OPERATORS
    #undef  VEC_OP
    #undef  VEC_OPEQ

    IMPLEMENT_VEC_FUNCTIONS
#undef BASE_TYPE

/*
    Implementation of Vector3<T> operators
*/

#define BASE_TYPE Vector3
    #define VEC_OP +
    #define VEC_OPEQ +=
        IMPLEMENT_OPERATORS
    #undef  VEC_OP
    #undef  VEC_OPEQ

    #define VEC_OP -
    #define VEC_OPEQ -=
        IMPLEMENT_OPERATORS
    #undef  VEC_OP
    #undef  VEC_OPEQ

    #define VEC_OP *
    #define VEC_OPEQ *=
        IMPLEMENT_OPERATORS
    #undef  VEC_OP
    #undef  VEC_OPEQ

    #define VEC_OP /
    #define VEC_OPEQ /=
        IMPLEMENT_OPERATORS
    #undef  VEC_OP
    #undef  VEC_OPEQ

    #define VEC_OP %
    #define VEC_OPEQ %=
        IMPLEMENT_OPERATORS
    #undef  VEC_OP
    #undef  VEC_OPEQ

    #define VEC_OP &
    #define VEC_OPEQ &=
        IMPLEMENT_OPERATORS
    #undef  VEC_OP
    #undef  VEC_OPEQ

    #define VEC_OP |
    #define VEC_OPEQ |=
        IMPLEMENT_OPERATORS
    #undef  VEC_OP
    #undef  VEC_OPEQ

    #define VEC_OP ^
    #define VEC_OPEQ ^=
        IMPLEMENT_OPERATORS
    #undef  VEC_OP
    #undef  VEC_OPEQ

    IMPLEMENT_VEC_FUNCTIONS
#undef BASE_TYPE

/*
    Implementation of Vector4<T> operators
*/

#define BASE_TYPE Vector4
    #define VEC_OP +
    #define VEC_OPEQ +=
        IMPLEMENT_OPERATORS
    #undef  VEC_OP
    #undef  VEC_OPEQ

    #define VEC_OP -
    #define VEC_OPEQ -=
        IMPLEMENT_OPERATORS
    #undef  VEC_OP
    #undef  VEC_OPEQ

    #define VEC_OP *
    #define VEC_OPEQ *=
        IMPLEMENT_OPERATORS
    #undef  VEC_OP
    #undef  VEC_OPEQ

    #define VEC_OP /
    #define VEC_OPEQ /=
        IMPLEMENT_OPERATORS
    #undef  VEC_OP
    #undef  VEC_OPEQ

    #define VEC_OP %
    #define VEC_OPEQ %=
        IMPLEMENT_OPERATORS
    #undef  VEC_OP
    #undef  VEC_OPEQ

    #define VEC_OP &
    #define VEC_OPEQ &=
        IMPLEMENT_OPERATORS
    #undef  VEC_OP
    #undef  VEC_OPEQ

    #define VEC_OP |
    #define VEC_OPEQ |=
        IMPLEMENT_OPERATORS
    #undef  VEC_OP
    #undef  VEC_OPEQ

    #define VEC_OP ^
    #define VEC_OPEQ ^=
        IMPLEMENT_OPERATORS
    #undef  VEC_OP
    #undef  VEC_OPEQ

    IMPLEMENT_VEC_FUNCTIONS
#undef BASE_TYPE

/**
    cross product
*/
template<class T>
inline Vector3<T> cross(const Vector3<T>& v1, const Vector3<T>& v2)
{
    Vector3<T> vResult;
    vResult.elem[0] = v1.elem[1] * v2.elem[2] - v1.elem[2] * v2.elem[1];
    vResult.elem[1] = v1.elem[2] * v2.elem[0] - v1.elem[0] * v2.elem[2];
    vResult.elem[2] = v1.elem[0] * v2.elem[1] - v1.elem[1] * v2.elem[0];
    return vResult;
}

/*
    Component-wise absolute value
*/

template<class T>
inline Vector2<T> abs(const Vector2<T>& v) {
    return Vector2<T>(fabs(v.x), fabs(v.y));
}

template<class T>
inline Vector3<T> abs(const Vector3<T>& v) {
    return Vector3<T>(fabs(v.x), fabs(v.y), fabs(v.z));
}

template<class T>
inline Vector4<T> abs(const Vector4<T>& v) {
    return Vector4<T>(fabs(v.x), fabs(v.y), fabs(v.z), fabs(v.z));
}

/*
    specialization for length(VectorX<int>)
*/

inline float length(const Vector2<int>& v) {
    return sqrtf( float(lengthSq(v)) );
}

inline float length(const Vector3<int>& v) {
    return sqrtf( float(lengthSq(v)) );
}

inline float length(const Vector4<int>& v) {
    return sqrtf( float(lengthSq(v)) );
}

/*
     stream operators
*/

/// output Vector2d to stream
template<class T>
std::ostream& operator << (std::ostream& s, const Vector2<T>& v) {
    return (s << "[" << v.elem[0] << " " << v.elem[1] << "]");
}

/// output Vector3d to stream
template<class T>
std::ostream& operator << (std::ostream& s, const Vector3<T>& v) {
    return (s << "[" << v.elem[0] << " " << v.elem[1] << " " << v.elem[2] << "]");
}

/// output Vector4d to stream
template<class T>
std::ostream& operator << (std::ostream& s, const Vector4<T>& v) {
    return (s << "[" << v.elem[0] << " " << v.elem[1] << " " << v.elem[2] << " " << v.elem[3] << "]");
}

/// input Vector2d from stream
template<class T>
std::istream& operator >> (std::istream& s, Vector2<T>& v) {
    char ch;

    s >> ch;
    if (ch == '[') {  // format "[ x y ]"
        s >> v[0] >> v[1] >> ch;
        if (ch != ']') {
            s.clear(std::ios::badbit);
        }
    } else { // format "x y"
        s.putback(ch);
        s >> v[0] >> v[1];
    }

    return s;
}

/// input Vector3d from stream
template<class T>
std::istream& operator >> (std::istream& s, Vector3<T>& v) {
    char ch;

    s >> ch;
    if (ch == '[') {  // format "[ x y z ]"
        s >> v[0] >> v[1] >> v[2] >> ch;
        if (ch != ']') {
            s.clear(std::ios::badbit);
        }
    } else { // format "x y z"
        s.putback(ch);
        s >> v[0] >> v[1] >> v[2];
    }

    return s;
}

/// input Vector4d from stream
template<class T>
std::istream& operator >> (std::istream& s, Vector4<T>& v) {
    char ch;

    s >> ch;
    if (ch =='[') {  // format "[ x y z w ]"
        s >> v[0] >> v[1] >> v[2] >> v[3] >> ch;
        if (ch != ']') {
            s.clear(std::ios::badbit);
        }
    } else { // format "x y z w"
        s.putback(ch);
        s >> v[0] >> v[1] >> v[2] >> v[3];
    }

    return s;
}

} // namespace

#endif // TGT_VECTOR_H
