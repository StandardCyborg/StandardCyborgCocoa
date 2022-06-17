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

namespace standard_cyborg {
namespace math {

struct __attribute__((packed, aligned(16))) Vec3 {
    float x;
    float y;
    float z;

// A dummy to pad Vec3 and align it for easy texture interop
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
private:
    float _padding = 0.0f;
#pragma clang diagnostic pop

public:
    
    // clang-format off
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    
    Vec3(float value) : x(value), y(value), z(value) {}

    Vec3() : x(0), y(0), z(0) {}
    
    Vec3(Vec2 a, float b) : x(a.x), y(a.y), z(b) {}

    Vec3(float a, Vec2 b) : x(a), y(b.x), z(b.y) {}
    // clang-format on

    /* Vec3 methods */
    
    /** Normalize a vector in-place and return (by reference) the vector itself */
    inline Vec3& normalize();
    
    /** Compute a normalized copy of the vector */
    static inline Vec3 normalize(const Vec3& a);
    
    /** Compute the squared norm of a vector */
    inline float squaredNorm() const;
    
    /** Compute the Euclidean norm (length) of a vector */
    inline float norm() const;
    
    /** Get the first two components (x, y) as a Vec2 */
    inline Vec2 xy() const;
    
    /* Static functions namespaced under Vec3:: */
    
    /** Compute whether two vectors are equal to within floating point epsilon */
    static inline bool almostEqual(
        const Vec3& lhs,
        const Vec3& rhs,
        float relativeTolerance = std::numeric_limits<float>::epsilon(),
        float absoluteTolerance = std::numeric_limits<float>::epsilon()
    );
    
    /** Compute the dot product of two vectors */
    static inline float dot(const Vec3& lhs, const Vec3& rhs);
    
    /** Compute the cross product of two vectors */
    static inline Vec3 cross(const Vec3& lhs, const Vec3& rhs);
    
    /** Linearly iterpolate between vectors a and b */
    static inline Vec3 lerp(const Vec3& a, const Vec3& b, float interpolant);
    
    /** Compute the per-component maximum of two vectors */
    static inline Vec3 min(const Vec3& a, const Vec3& b);
    
    /** Compute the per-component minimum of two vectors */
    static inline Vec3 max(const Vec3& a, const Vec3& b);
    
    /** Compute the angle between two vectors, in radians */
    static inline float angleBetween(const Vec3& a, const Vec3& b);
    
    /** Compute the cartesian distance between two vectors */
    static inline float distanceBetween(const Vec3& lhs, const Vec3& rhs);
    
    /** Compute the square of the cartesian distance between two vectors */
    static inline float squaredDistanceBetween(const Vec3& lhs, const Vec3& rhs);
    
    /** Raise the vector to a component-wise power */
    static inline Vec3 pow(const Vec3& a, float exponent);
    
    inline float operator[](int i) const;
};

static_assert(offsetof(Vec3, x) == 0, "offset of Vec3.x is 0 bytes");
static_assert(offsetof(Vec3, y) == 4, "offset of Vec3.y is 4 bytes");
static_assert(offsetof(Vec3, z) == 8, "offset of Vec3.z is 8 bytes");
static_assert(sizeof(Vec3) == 16, "size of Vec3 is 16 bytes");

inline float Vec3::operator[](int i) const
{
    void* thisVoid = (void*)(this);
    return ((float*)thisVoid)[i];
}

/* Equality operators */
inline bool operator==(const Vec3& lhs, const Vec3& rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

inline bool operator!=(const Vec3& lhs, const Vec3& rhs)
{
    return !(lhs == rhs);
}

/* Unary minus */
inline Vec3 operator-(const Vec3& a)
{
    return Vec3(-a.x, -a.y, -a.z);
}

/* Assignment arithmetic operators with floats */
inline Vec3& operator/=(Vec3& lhs, const float rhs)
{
    lhs.x /= rhs;
    lhs.y /= rhs;
    lhs.z /= rhs;
    return lhs;
}

inline Vec3& operator*=(Vec3& lhs, const float rhs)
{
    lhs.x *= rhs;
    lhs.y *= rhs;
    lhs.z *= rhs;
    return lhs;
}

inline Vec3& operator+=(Vec3& lhs, const float rhs)
{
    lhs.x += rhs;
    lhs.y += rhs;
    lhs.z += rhs;
    return lhs;
}

inline Vec3& operator-=(Vec3& lhs, const float rhs)
{
    lhs.x -= rhs;
    lhs.y -= rhs;
    lhs.z -= rhs;
    return lhs;
}

/* Assignment arithmetic operators with Vec3 */
inline Vec3& operator/=(Vec3& lhs, const Vec3& rhs)
{
    lhs.x /= rhs.x;
    lhs.y /= rhs.y;
    lhs.z /= rhs.z;
    return lhs;
}

inline Vec3& operator*=(Vec3& lhs, const Vec3& rhs)
{
    lhs.x *= rhs.x;
    lhs.y *= rhs.y;
    lhs.z *= rhs.z;
    return lhs;
}

inline Vec3& operator+=(Vec3& lhs, const Vec3& rhs)
{
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    lhs.z += rhs.z;
    return lhs;
}

inline Vec3& operator-=(Vec3& lhs, const Vec3& rhs)
{
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    lhs.z -= rhs.z;
    return lhs;
}

/* Arithmetic operators for Vec3 <-> Vec3 */
inline Vec3 operator+(const Vec3& lhs, const Vec3& rhs)
{
    return Vec3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

inline Vec3 operator-(const Vec3& lhs, const Vec3& rhs)
{
    return Vec3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

inline Vec3 operator*(const Vec3& lhs, const Vec3& rhs)
{
    return Vec3(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
}

inline Vec3 operator/(const Vec3& lhs, const Vec3& rhs)
{
    return Vec3(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z);
}

/* Arithmetic operators for Vec3 <-> float */
inline Vec3 operator+(const Vec3& lhs, const float rhs)
{
    return Vec3(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs);
}

inline Vec3 operator-(const Vec3& lhs, const float rhs)
{
    return Vec3(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs);
}

inline Vec3 operator*(const Vec3& lhs, const float rhs)
{
    return Vec3(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs);
}

inline Vec3 operator/(const Vec3& lhs, const float rhs)
{
    return Vec3(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs);
}

/* Arithmetic operators for float <-> Vec3 */
inline Vec3 operator+(const float lhs, const Vec3& rhs)
{
    return Vec3(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z);
}

inline Vec3 operator-(const float lhs, const Vec3& rhs)
{
    return Vec3(lhs - rhs.x, lhs - rhs.y, lhs - rhs.z);
}

inline Vec3 operator*(const float lhs, const Vec3& rhs)
{
    return Vec3(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
}

inline Vec3 operator/(const float lhs, const Vec3& rhs)
{
    return Vec3(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z);
}

inline Vec3& Vec3::normalize()
{
    float invLength = 1.0 / std::sqrt(x * x + y * y + z * z);
    x *= invLength;
    y *= invLength;
    z *= invLength;
    return *this;
}

inline Vec3 Vec3::normalize(const Vec3& v)
{
    float invLength = 1.0 / std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return Vec3(v.x * invLength, v.y * invLength, v.z * invLength);
}

inline float Vec3::squaredNorm() const
{
    return x * x + y * y + z * z;
}

inline float Vec3::norm() const
{
    return std::sqrt(x * x + y * y + z * z);
}

inline float Vec3::dot(const Vec3& lhs, const Vec3& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

inline bool Vec3::almostEqual(const Vec3& lhs, const Vec3& rhs, float relativeTolerance, float absoluteTolerance)
{
    return AlmostEqual(lhs.x, rhs.x, relativeTolerance, absoluteTolerance) &&
           AlmostEqual(lhs.y, rhs.y, relativeTolerance, absoluteTolerance) &&
           AlmostEqual(lhs.z, rhs.z, relativeTolerance, absoluteTolerance);
}

inline Vec3 Vec3::cross(const Vec3& lhs, const Vec3& rhs)
{
    return Vec3(
        lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.x * rhs.y - lhs.y * rhs.x
    );
}

inline Vec3 Vec3::lerp(const Vec3& a, const Vec3& b, float interpolant)
{
    float complement = 1.0f - interpolant;
    return Vec3(
        complement * a.x + interpolant * b.x,
        complement * a.y + interpolant * b.y,
        complement * a.z + interpolant * b.z
    );
}

inline Vec3 Vec3::min(const Vec3& a, const Vec3& b)
{
    return Vec3(
        std::min(a.x, b.x),
        std::min(a.y, b.y),
        std::min(a.z, b.z)
    );
}

inline Vec3 Vec3::max(const Vec3& a, const Vec3& b)
{
    return Vec3(
        std::max(a.x, b.x),
        std::max(a.y, b.y),
        std::max(a.z, b.z)
    );
}

inline float Vec3::angleBetween(const Vec3& a, const Vec3& b)
{
    float lengthProduct = (a.x * a.x + a.y * a.y + a.z * a.z) * (b.x * b.x + b.y * b.y + b.z * b.z);
    float arg = (a.x * b.x + a.y * b.y + a.z * b.z) / std::sqrt(lengthProduct);
    if (arg > 1.0f) arg = 1.0;
    if (arg < -1.0f) arg = -1.0;
    return std::acos(arg);
}

inline float Vec3::distanceBetween(const Vec3& lhs, const Vec3& rhs)
{
    return (lhs - rhs).norm();
}

/** Compute the square of the cartesian distance between two vectors */
inline float Vec3::squaredDistanceBetween(const Vec3& lhs, const Vec3& rhs)
{
    return (lhs - rhs).squaredNorm();
}

inline Vec3 Vec3::pow(const Vec3& a, float exponent)
{
    return Vec3(
        std::pow(a.x, exponent),
        std::pow(a.y, exponent),
        std::pow(a.z, exponent)
    );
}

inline Vec2 Vec3::xy() const
{
    return Vec2(x, y);
}

} // namespace math
} // namespace standard_cyborg
