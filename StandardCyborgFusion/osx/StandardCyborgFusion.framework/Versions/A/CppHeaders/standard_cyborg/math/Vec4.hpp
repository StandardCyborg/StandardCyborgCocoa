/*
Copyright 2020 Standard Cyborg

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

#include <cmath>
#include <limits>

#include "standard_cyborg/math/MathHelpers.hpp"
#include "standard_cyborg/math/Vec2.hpp"
#include "standard_cyborg/math/Vec3.hpp"

namespace standard_cyborg {
namespace math {

struct __attribute__((packed, aligned(16))) Vec4
{
    float x;
    float y;
    float z;
    float w;
    
    Vec4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
    Vec4(float value) : x(value), y(value), z(value), w(value) {}

    Vec4() : x(0), y(0), z(0), w(0) {}
    
    Vec4(Vec2 a, float b, float c) : x(a.x), y(a.y), z(b), w(c) {}

    Vec4(Vec2 a, Vec2 b) : x(a.x), y(a.y), z(b.x), w(b.y) {}

    Vec4(float a, Vec2 b, float c) : x(a), y(b.x), z(b.y), w(c) {}

    Vec4(float a, float b, Vec2 c) : x(a), y(b), z(c.x), w(c.y) {}

    Vec4(float a, Vec3 b) : x(a), y(b.x), z(b.y), w(b.z) {}

    Vec4(Vec3 a, float b) : x(a.x), y(a.y), z(a.z), w(b) {}

    
    /* Vec4 methods */
    
    /* Normalize a vector in-place and return (by reference) the vector itself */
    inline Vec4& normalize();
    
    /* Compute a normalized copy of the vector */
    static inline Vec4 normalize(const Vec4& a);
    
    /* Compute the squared norm of a vector */
    inline float squaredNorm() const;
    
    /* Compute the Euclidean norm (length) of a vector */
    inline float norm() const;
    
    /* Get the first three components (x, y, z) as a Vec3 */
    inline Vec3 xyz() const;
    
    /* Static functions namespaced under Vec4:: */
    
    /* Compute whether two vectors are equal to within floating point epsilon */
    static inline bool almostEqual(
        const Vec4& lhs,
        const Vec4& rhs,
        float relativeTolerance = std::numeric_limits<float>::epsilon(),
        float absoluteTolerance = std::numeric_limits<float>::epsilon()
    );

    /* Compute the dot product of two vectors */
    static inline float dot(const Vec4& lhs, const Vec4& rhs);
    
    /* Linearly iterpolate between vectors a and b */
    static inline Vec4 lerp(const Vec4& a, const Vec4& b, float interpolant);
    
    /* Compute the per-component maximum of two vectors */
    static inline Vec4 min(const Vec4& a, const Vec4& b);
    
    /* Compute the per-component minimum of two vectors */
    static inline Vec4 max(const Vec4& a, const Vec4& b);
    
    /* Raise the vector to a component-wise power */
    static inline Vec4 pow(const Vec4& a, float exponent);
};

static_assert(offsetof(Vec4, x) == 0, "offset of Vec4.x is 0 bytes");
static_assert(offsetof(Vec4, y) == 4, "offset of Vec4.y is 4 bytes");
static_assert(offsetof(Vec4, z) == 8, "offset of Vec4.z is 8 bytes");
static_assert(offsetof(Vec4, w) == 12, "offset of Vec4.w is 12 bytes");
static_assert(sizeof(Vec4) == 16, "size of Vec4 is 16 bytes");

/* Equality operators */
inline bool operator==(const Vec4& lhs, const Vec4& rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
}

inline bool operator!=(const Vec4& lhs, const Vec4& rhs)
{
    return !(lhs == rhs);
}

/* Unary minus */
inline Vec4 operator-(const Vec4& a)
{
    return Vec4(-a.x, -a.y, -a.z, -a.w);
}

/* Assignment arithmetic operators with floats */
inline Vec4& operator/=(Vec4& lhs, const float rhs)
{
    lhs.x /= rhs;
    lhs.y /= rhs;
    lhs.z /= rhs;
    lhs.w /= rhs;
    return lhs;
}

inline Vec4& operator*=(Vec4& lhs, const float rhs)
{
    lhs.x *= rhs;
    lhs.y *= rhs;
    lhs.z *= rhs;
    lhs.w *= rhs;
    return lhs;
}

inline Vec4& operator+=(Vec4& lhs, const float rhs)
{
    lhs.x += rhs;
    lhs.y += rhs;
    lhs.z += rhs;
    lhs.w += rhs;
    return lhs;
}

inline Vec4& operator-=(Vec4& lhs, const float rhs)
{
    lhs.x -= rhs;
    lhs.y -= rhs;
    lhs.z -= rhs;
    lhs.w -= rhs;
    return lhs;
}

/* Assignment arithmetic operators with Vec4 */
inline Vec4& operator/=(Vec4& lhs, const Vec4& rhs)
{
    lhs.x /= rhs.x;
    lhs.y /= rhs.y;
    lhs.z /= rhs.z;
    lhs.w /= rhs.w;
    return lhs;
}

inline Vec4& operator*=(Vec4& lhs, const Vec4& rhs)
{
    lhs.x *= rhs.x;
    lhs.y *= rhs.y;
    lhs.z *= rhs.z;
    lhs.w *= rhs.w;
    return lhs;
}

inline Vec4& operator+=(Vec4& lhs, const Vec4& rhs)
{
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    lhs.z += rhs.z;
    lhs.w += rhs.w;
    return lhs;
}

inline Vec4& operator-=(Vec4& lhs, const Vec4& rhs)
{
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    lhs.z -= rhs.z;
    lhs.w -= rhs.w;
    return lhs;
}

/* Arithmetic operators for Vec4 <-> Vec4 */
inline Vec4 operator+(const Vec4& lhs, const Vec4& rhs)
{
    return Vec4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);
}

inline Vec4 operator-(const Vec4& lhs, const Vec4& rhs)
{
    return Vec4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);
}

inline Vec4 operator*(const Vec4& lhs, const Vec4& rhs)
{
    return Vec4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w);
}

inline Vec4 operator/(const Vec4& lhs, const Vec4& rhs)
{
    return Vec4(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w);
}

/* Arithmetic operators for Vec4 <-> float */
inline Vec4 operator+(const Vec4& lhs, const float rhs)
{
    return Vec4(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs, lhs.w + rhs);
}

inline Vec4 operator-(const Vec4& lhs, const float rhs)
{
    return Vec4(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs, lhs.w - rhs);
}

inline Vec4 operator*(const Vec4& lhs, const float rhs)
{
    return Vec4(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs);
}

inline Vec4 operator/(const Vec4& lhs, const float rhs)
{
    return Vec4(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs);
}

/* Arithmetic operators for float <-> Vec4 */
inline Vec4 operator+(const float lhs, const Vec4& rhs)
{
    return Vec4(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z, lhs + rhs.w);
}

inline Vec4 operator-(const float lhs, const Vec4& rhs)
{
    return Vec4(lhs - rhs.x, lhs - rhs.y, lhs - rhs.z, lhs - rhs.w);
}

inline Vec4 operator*(const float lhs, const Vec4& rhs)
{
    return Vec4(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w);
}

inline Vec4 operator/(const float lhs, const Vec4& rhs)
{
    return Vec4(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z, lhs / rhs.w);
}

inline Vec4& Vec4::normalize()
{
    float invLength = 1.0 / std::sqrt(x * x + y * y + z * z + w * w);
    x *= invLength;
    y *= invLength;
    z *= invLength;
    w *= invLength;
    return *this;
}

inline Vec4 Vec4::normalize(const Vec4& v)
{
    float invLength = 1.0 / std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
    return Vec4(v.x * invLength, v.y * invLength, v.z * invLength, v.w * invLength);
}

inline float Vec4::squaredNorm() const
{
    return x * x + y * y + z * z + w * w;
}

inline float Vec4::norm() const
{
    return std::sqrt(x * x + y * y + z * z + w * w);
}

inline float Vec4::dot(const Vec4& lhs, const Vec4& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

inline bool Vec4::almostEqual(const Vec4& lhs, const Vec4& rhs, float relativeTolerance, float absoluteTolerance)
{
    return AlmostEqual(lhs.x, rhs.x, relativeTolerance, absoluteTolerance) &&
           AlmostEqual(lhs.y, rhs.y, relativeTolerance, absoluteTolerance) &&
           AlmostEqual(lhs.z, rhs.z, relativeTolerance, absoluteTolerance) &&
           AlmostEqual(lhs.w, rhs.w, relativeTolerance, absoluteTolerance);
}

inline Vec4 Vec4::lerp(const Vec4& a, const Vec4& b, float interpolant)
{
    float complement = 1.0f - interpolant;
    return Vec4(
        complement * a.x + interpolant * b.x,
        complement * a.y + interpolant * b.y,
        complement * a.z + interpolant * b.z,
        complement * a.w + interpolant * b.w
    );
}

inline Vec4 Vec4::min(const Vec4& a, const Vec4& b)
{
    return Vec4(
        std::min(a.x, b.x),
        std::min(a.y, b.y),
        std::min(a.z, b.z),
        std::min(a.w, b.w)
    );
}

inline Vec4 Vec4::max(const Vec4& a, const Vec4& b)
{
    return Vec4(
        std::max(a.x, b.x),
        std::max(a.y, b.y),
        std::max(a.z, b.z),
        std::max(a.w, b.w)
    );
}

inline Vec4 Vec4::pow(const Vec4& a, float exponent)
{
    return Vec4(
        std::pow(a.x, exponent),
        std::pow(a.y, exponent),
        std::pow(a.z, exponent),
        std::pow(a.w, exponent)
    );
}

inline Vec3 Vec4::xyz() const
{
    return Vec3(x, y, z);
}

} // namespace math
} // namespace standard_cyborg
