//
//  Vec2.hpp
//  StandardCyborgData
//
//  Created by Ricky Reusser on 6/13/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <cmath>
#include <limits>

#include <StandardCyborgData/MathHelpers.hpp>

namespace StandardCyborg {

struct Vec2 {
    float x;
    float y;
    
    Vec2(float x_, float y_) : x(x_), y(y_) {}
    Vec2(float value) : x(value), y(value) {}

    Vec2() : x(0), y(0) {}
    
    /* Vec2 methods */
    
    /* Normalize a vector in-place and return (by reference) the vector itself */
    inline Vec2& normalize();
    
    /* Compute a normalized copy of the vector */
    static inline Vec2 normalize(const Vec2& a);
    
    /* Compute the squared norm of a vector */
    inline float squaredNorm() const;
    
    /* Compute the Euclidean norm (length) of a vector */
    inline float norm() const;
    
    /* Static functions namespaced under Vec2:: */
    
    /* Compute whether two vectors are equal to within floating point epsilon */
    static inline bool almostEqual(
        const Vec2& lhs,
        const Vec2& rhs,
        float relativeTolerance = std::numeric_limits<float>::epsilon(),
        float absoluteTolerance = std::numeric_limits<float>::epsilon()
    );

    /* Compute the dot product of two vectors */
    static inline float dot(const Vec2& lhs, const Vec2& rhs);
    
    /* Compute the out of plane component of the cross product of two 2D vectors */
    static inline float cross(const Vec2& lhs, const Vec2& rhs);
    
    /* Linearly iterpolate between vectors a and b */
    static inline Vec2 lerp(const Vec2& a, const Vec2& b, float interpolant);
    
    /* Compute the per-component maximum of two vectors */
    static inline Vec2 min(const Vec2& a, const Vec2& b);
    
    /* Compute the per-component minimum of two vectors */
    static inline Vec2 max(const Vec2& a, const Vec2& b);
    
    /* Compute the angle between two vectors, in radians */
    static inline float angleBetween(const Vec2& a, const Vec2& b);
    
    /* Raise the vector to a component-wise power */
    static inline Vec2 pow(const Vec2& a, float exponent);
};

static_assert(offsetof(Vec2, x) == 0, "offset of Vec2.x is 0 bytes");
static_assert(offsetof(Vec2, y) == 4, "offset of Vec2.y is 4 bytes");
static_assert(sizeof(Vec2) == 8, "size of Vec2 is 8 bytes");

/* Equality operators */
inline bool operator==(const Vec2& lhs, const Vec2& rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline bool operator!=(const Vec2& lhs, const Vec2& rhs)
{
    return !(lhs == rhs);
}

/* Unary minus */
inline Vec2 operator-(const Vec2& a)
{
    return Vec2(-a.x, -a.y);
}

/* Assignment arithmetic operators with floats */
inline Vec2& operator/=(Vec2& lhs, const float rhs)
{
    lhs.x /= rhs;
    lhs.y /= rhs;
    return lhs;
}

inline Vec2& operator*=(Vec2& lhs, const float rhs)
{
    lhs.x *= rhs;
    lhs.y *= rhs;
    return lhs;
}

inline Vec2& operator+=(Vec2& lhs, const float rhs)
{
    lhs.x += rhs;
    lhs.y += rhs;
    return lhs;
}

inline Vec2& operator-=(Vec2& lhs, const float rhs)
{
    lhs.x -= rhs;
    lhs.y -= rhs;
    return lhs;
}

/* Assignment arithmetic operators with Vec2 */
inline Vec2& operator/=(Vec2& lhs, const Vec2& rhs)
{
    lhs.x /= rhs.x;
    lhs.y /= rhs.y;
    return lhs;
}

inline Vec2& operator*=(Vec2& lhs, const Vec2& rhs)
{
    lhs.x *= rhs.x;
    lhs.y *= rhs.y;
    return lhs;
}

inline Vec2& operator+=(Vec2& lhs, const Vec2& rhs)
{
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    return lhs;
}

inline Vec2& operator-=(Vec2& lhs, const Vec2& rhs)
{
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    return lhs;
}

/* Arithmetic operators for Vec2 <-> Vec2 */
inline Vec2 operator+(const Vec2& lhs, const Vec2& rhs)
{
    return Vec2(lhs.x + rhs.x, lhs.y + rhs.y);
}

inline Vec2 operator-(const Vec2& lhs, const Vec2& rhs)
{
    return Vec2(lhs.x - rhs.x, lhs.y - rhs.y);
}

inline Vec2 operator*(const Vec2& lhs, const Vec2& rhs)
{
    return Vec2(lhs.x * rhs.x, lhs.y * rhs.y);
}

inline Vec2 operator/(const Vec2& lhs, const Vec2& rhs)
{
    return Vec2(lhs.x / rhs.x, lhs.y / rhs.y);
}

/* Arithmetic operators for Vec2 <-> float */
inline Vec2 operator+(const Vec2& lhs, const float rhs)
{
    return Vec2(lhs.x + rhs, lhs.y + rhs);
}

inline Vec2 operator-(const Vec2& lhs, const float rhs)
{
    return Vec2(lhs.x - rhs, lhs.y - rhs);
}

inline Vec2 operator*(const Vec2& lhs, const float rhs)
{
    return Vec2(lhs.x * rhs, lhs.y * rhs);
}

inline Vec2 operator/(const Vec2& lhs, const float rhs)
{
    return Vec2(lhs.x / rhs, lhs.y / rhs);
}

/* Arithmetic operators for float <-> Vec2 */
inline Vec2 operator+(const float lhs, const Vec2& rhs)
{
    return Vec2(lhs + rhs.x, lhs + rhs.y);
}

inline Vec2 operator-(const float lhs, const Vec2& rhs)
{
    return Vec2(lhs - rhs.x, lhs - rhs.y);
}

inline Vec2 operator*(const float lhs, const Vec2& rhs)
{
    return Vec2(lhs * rhs.x, lhs * rhs.y);
}

inline Vec2 operator/(const float lhs, const Vec2& rhs)
{
    return Vec2(lhs / rhs.x, lhs / rhs.y);
}

inline Vec2& Vec2::normalize()
{
    float invLength = 1.0 / std::sqrt(x * x + y * y);
    x *= invLength;
    y *= invLength;
    return *this;
}

inline Vec2 Vec2::normalize(const Vec2& v)
{
    float invLength = 1.0 / std::sqrt(v.x * v.x + v.y * v.y);
    return Vec2(v.x * invLength, v.y * invLength);
}

inline float Vec2::squaredNorm() const
{
    return x * x + y * y;
}

inline float Vec2::norm() const
{
    return std::sqrtf(x * x + y * y);
}

inline float Vec2::dot(const Vec2& lhs, const Vec2& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

inline bool Vec2::almostEqual(const Vec2& lhs, const Vec2& rhs, float relativeTolerance, float absoluteTolerance)
{
    return StandardCyborg::almostEqual(lhs.x, rhs.x, relativeTolerance, absoluteTolerance) &&
           StandardCyborg::almostEqual(lhs.y, rhs.y, relativeTolerance, absoluteTolerance);
}

inline float Vec2::cross(const Vec2& lhs, const Vec2& rhs)
{
    return lhs.x * rhs.y - lhs.y * rhs.x;
}

inline Vec2 Vec2::lerp(const Vec2& a, const Vec2& b, float interpolant)
{
    float complement = 1.0f - interpolant;
    
    return Vec2(
        complement * a.x + interpolant * b.x,
        complement * a.y + interpolant * b.y
    );
}

inline Vec2 Vec2::min(const Vec2& a, const Vec2& b)
{
    return Vec2(
        std::min(a.x, b.x),
        std::min(a.y, b.y)
    );
}

inline Vec2 Vec2::max(const Vec2& a, const Vec2& b)
{
    return Vec2(
        std::max(a.x, b.x),
        std::max(a.y, b.y)
    );
}

inline float Vec2::angleBetween(const Vec2& a, const Vec2& b)
{
    float lengthProduct = (a.x * a.x + a.y * a.y) * (b.x * b.x + b.y * b.y);
    float arg = (a.x * b.x + a.y * b.y) / std::sqrt(lengthProduct);
    
    if (arg > 1.0f) arg = 1.0;
    if (arg < -1.0f) arg = -1.0;
    
    return std::acosf(arg);
}

inline Vec2 Vec2::pow(const Vec2& a, float exponent)
{
    return Vec2(
        std::pow(a.x, exponent),
        std::pow(a.y, exponent)
    );
}

} // namespace StandardCyborg
