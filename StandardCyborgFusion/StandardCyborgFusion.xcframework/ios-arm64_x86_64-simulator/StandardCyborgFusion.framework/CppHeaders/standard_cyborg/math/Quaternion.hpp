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

struct Mat3x3;

struct __attribute__((packed, aligned(16))) Quaternion
{
    float x;
    float y;
    float z;
    float w;
    
    Quaternion(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
    Quaternion(float value) : x(value), y(value), z(value), w(value) {}

    /** Default constructor initializes to the identity, (0, 0, 0, 1) */
    Quaternion() : x(0), y(0), z(0), w(1) {}
    
    Quaternion(Vec2 a, float b, float c) : x(a.x), y(a.y), z(b), w(c) {}
    Quaternion(Vec2 a, Vec2 b) : x(a.x), y(a.y), z(b.x), w(b.y) {}
    Quaternion(float a, Vec2 b, float c) : x(a), y(b.x), z(b.y), w(c) {}
    Quaternion(float a, float b, Vec2 c) : x(a), y(b), z(c.x), w(c.y) {}
    Quaternion(float a, Vec3 b) : x(a), y(b.x), z(b.y), w(b.z) {}
    Quaternion(Vec3 a, float b) : x(a.x), y(a.y), z(a.z), w(b) {}

    /* Quaternion methods */
    
    /** Normalize a vector in-place and return (by reference) the vector itself */
    inline Quaternion& normalize();

    /** Compute the squared norm of a vector */
    inline float squaredNorm() const;
    
    /** Compute the Euclidean norm (length) of a vector */
    inline float norm() const;
    
    /** Get the first three components (x, y, z) as a Vec3 */
    inline Vec3 xyz() const;
    
    /** Invert the quaternion in-place */
    inline Quaternion& invert();
    
    /** Return an inverted copy of the quaternion */
    inline Quaternion inverse() const;
    
    /** Conjugate this quaternion in-place */
    inline Quaternion& conjugate();
    
    /** Get a conjugated copy of this quaternion */
    inline Quaternion conjugated() const;
    
    /** Perform in-place rotation about the x-axis */
    Quaternion& rotateX(float radians);
    
    /** Perform in-place rotation about the y-axis */
    Quaternion& rotateY(float radians);
    
    /** Perform in-place rotation about the z-axis */
    Quaternion& rotateZ(float radians);
    
    /* Static functions namespaced under Quaternion:: */
    
    /** Get the identity (which is also how quaternions are initialized, but to be very explicit that it is the identity */
    static inline Quaternion Identity();
    
    /** Retrieve a quaternion from a Mat3x3 rotation matrix */
    static Quaternion fromMat3x3(const Mat3x3 matrix);
    
    /** Compute a normalized copy of the vector */
    static inline Quaternion normalize(const Quaternion& a);
    
    /** Compute whether two vectors are equal to within floating point epsilon */
    static inline bool almostEqual(
        const Quaternion& lhs,
        const Quaternion& rhs,
        float relativeTolerance = std::numeric_limits<float>::epsilon(),
        float absoluteTolerance = std::numeric_limits<float>::epsilon()
    );

    /** Compute the dot product of two vectors */
    static inline float dot(const Quaternion& lhs, const Quaternion& rhs);
    
    /** Linearly iterpolate between vectors a and b */
    static inline Quaternion lerp(Quaternion a, Quaternion b, float interpolant);
    
    /** Perform spherical linear interpolation between two quaternions */
    static Quaternion slerp(Quaternion a, Quaternion b, float interpolant);
    
    /** Create a new quaternion representing a rotation around the x axis */
    static Quaternion fromRotationX(float radians);
    
    /** Create a new quaternion representing a rotation around the y axis */
    static Quaternion fromRotationY(float radians);
    
    /** Create a new quaternion representing a rotation around the z axis */
    static Quaternion fromRotationZ(float radians);
};

static_assert(offsetof(Quaternion, x) == 0, "offset of Quaternion.x is 0 bytes");
static_assert(offsetof(Quaternion, y) == 4, "offset of Quaternion.y is 4 bytes");
static_assert(offsetof(Quaternion, z) == 8, "offset of Quaternion.z is 8 bytes");
static_assert(offsetof(Quaternion, w) == 12, "offset of Quaternion.w is 12 bytes");
static_assert(sizeof(Quaternion) == 16, "size of Quaternion is 16 bytes");

/* Transformation by a quaternion */
Vec3 operator*(Quaternion q, Vec3 v);

inline Quaternion Quaternion::Identity() {
    return Quaternion {0, 0, 0, 1};
}

/* Equality operators */
inline bool operator==(const Quaternion& lhs, const Quaternion& rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
}

inline bool operator!=(const Quaternion& lhs, const Quaternion& rhs)
{
    return !(lhs == rhs);
}

/* Unary minus */
inline Quaternion operator-(const Quaternion& a)
{
    return Quaternion(-a.x, -a.y, -a.z, -a.w);
}

inline Quaternion& operator*=(Quaternion& lhs, const float rhs)
{
    lhs.x *= rhs;
    lhs.y *= rhs;
    lhs.z *= rhs;
    lhs.w *= rhs;
    return lhs;
}

inline Quaternion& operator+=(Quaternion& lhs, const float rhs)
{
    lhs.x += rhs;
    lhs.y += rhs;
    lhs.z += rhs;
    lhs.w += rhs;
    return lhs;
}

/* Assignment arithmetic operators with Quaternion */
inline Quaternion& operator*=(Quaternion& lhs, const Quaternion& rhs)
{
    float ax = lhs.x;
    float ay = lhs.y;
    float az = lhs.z;
    float aw = lhs.w;
    lhs.x = ax * rhs.w + aw * rhs.x + ay * rhs.z - az * rhs.y;
    lhs.y = ay * rhs.w + aw * rhs.y + az * rhs.x - ax * rhs.z;
    lhs.z = az * rhs.w + aw * rhs.z + ax * rhs.y - ay * rhs.x;
    lhs.w = aw * rhs.w - ax * rhs.x - ay * rhs.y - az * rhs.z;
    return lhs;
}

inline Quaternion& operator+=(Quaternion& lhs, const Quaternion& rhs)
{
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    lhs.z += rhs.z;
    lhs.w += rhs.w;
    return lhs;
}

/* Arithmetic operators for Quaternion <-> Quaternion */
inline Quaternion operator+(const Quaternion& lhs, const Quaternion& rhs)
{
    return Quaternion(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);
}

inline Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs)
{
    return Quaternion{
        lhs.x * rhs.w + lhs.w * rhs.x + lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.y * rhs.w + lhs.w * rhs.y + lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.z * rhs.w + lhs.w * rhs.z + lhs.x * rhs.y - lhs.y * rhs.x,
        lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z
    };
}

/* Arithmetic operators for Quaternion <-> float */
inline Quaternion operator+(const Quaternion& lhs, const float rhs)
{
    return Quaternion(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs, lhs.w + rhs);
}

inline Quaternion operator*(const Quaternion& lhs, const float rhs)
{
    return Quaternion(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs);
}

/* Arithmetic operators for float <-> Quaternion */
inline Quaternion operator+(const float lhs, const Quaternion& rhs)
{
    return Quaternion(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z, lhs + rhs.w);
}

inline Quaternion operator*(const float lhs, const Quaternion& rhs)
{
    return Quaternion(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w);
}

inline Quaternion& Quaternion::normalize()
{
    float invLength = 1.0 / std::sqrt(x * x + y * y + z * z + w * w);
    x *= invLength;
    y *= invLength;
    z *= invLength;
    w *= invLength;
    return *this;
}

inline Quaternion Quaternion::normalize(const Quaternion& v)
{
    float invLength = 1.0 / std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
    return Quaternion(v.x * invLength, v.y * invLength, v.z * invLength, v.w * invLength);
}

inline float Quaternion::squaredNorm() const
{
    return x * x + y * y + z * z + w * w;
}

inline float Quaternion::norm() const
{
    return std::sqrt(x * x + y * y + z * z + w * w);
}

inline float Quaternion::dot(const Quaternion& lhs, const Quaternion& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

inline bool Quaternion::almostEqual(const Quaternion& lhs, const Quaternion& rhs, float relativeTolerance, float absoluteTolerance)
{
    return AlmostEqual(lhs.x, rhs.x, relativeTolerance, absoluteTolerance) &&
           AlmostEqual(lhs.y, rhs.y, relativeTolerance, absoluteTolerance) &&
           AlmostEqual(lhs.z, rhs.z, relativeTolerance, absoluteTolerance) &&
           AlmostEqual(lhs.w, rhs.w, relativeTolerance, absoluteTolerance);
}

inline Quaternion Quaternion::lerp(Quaternion a, Quaternion b, float interpolant)
{
    float complement = 1.0f - interpolant;
    return Quaternion(
        complement * a.x + interpolant * b.x,
        complement * a.y + interpolant * b.y,
        complement * a.z + interpolant * b.z,
        complement * a.w + interpolant * b.w
    );
}

inline Quaternion& Quaternion::invert() {
    float a0 = x;
    float a1 = y;
    float a2 = z;
    float a3 = w;
    float dot = a0 * a0 + a1 * a1 + a2 * a2 + a3 * a3;
    if (dot == 0.0f) {
        x = y = z = w = 0.0f;
        return *this;
    }
    float invDot = 1.0 / dot;
    
    x = -a0 * invDot;
    y = -a1 * invDot;
    z = -a2 * invDot;
    w = a3 * invDot;
    
    return *this;
}

inline Quaternion Quaternion::inverse() const {
    Quaternion q {x, y, z, w};
    q.invert();
    return q;
}

inline Vec3 Quaternion::xyz() const
{
    return Vec3(x, y, z);
}

Quaternion& Quaternion::conjugate() {
    x = -x;
    y = -y;
    z = -z;
    return *this;
}

Quaternion Quaternion::conjugated() const {
    return Quaternion { -x, -y, -z, w };
}


} // namespace math
} // namespace standard_cyborg

