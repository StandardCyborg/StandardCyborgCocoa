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

#include <limits>
#include <vector>

#include "standard_cyborg/math/Vec3.hpp"
#include "standard_cyborg/util/AssertHelper.hpp"

namespace standard_cyborg {
namespace math {

struct Mat3x3;
struct Mat4x4;
struct Quaternion;
struct Transform;

struct Mat3x4 {
    // note: we use row-major order for our matrices.
    float m00, m01, m02, m03,
          m10, m11, m12, m13,
          m20, m21, m22, m23;
    
    Mat3x4();
    
    Mat3x4(const Mat4x4& matrix);
    
    Mat3x4(float m00_, float m01_, float m02_, float m03_,
           float m10_, float m11_, float m12_, float m13_,
           float m20_, float m21_, float m22_, float m23_);

    Mat3x4(Mat3x3 transform, Vec3 translation);
    
    static Mat3x4 Identity();
    static Mat3x4 Zeros();
    
    static Mat3x4 fromColumnMajorVector(const std::vector<float>& data);
    static Mat3x4 fromRowMajorVector(const std::vector<float>& data);
    
    /** Construct a Mat3x4 by truncating the last row from a Mat4x4—which has been presumably been used
     * to store a 3D affine transform. This performs no check that the last row of the Mat4x4 is the
     * identity, though it ought to be if truncation is meaningful.
     */
    static Mat3x4 fromMat4x4(const Mat4x4 transform);
    
    std::vector<float> toColumnMajorVector() const;
    std::vector<float> toRowMajorVector() const;
    
    /** Compute whether two matrices are equal to within floating point epsilon */
    static inline bool almostEqual(
        const Mat3x4& lhs,
        const Mat3x4& rhs,
        float relativeTolerance = std::numeric_limits<float>::epsilon(),
        float absoluteTolerance = std::numeric_limits<float>::epsilon()
    );
    
    /** Invert in-place */
    Mat3x4& invert();
    
    /** Return an inverted copy */
    Mat3x4 inverse() const;

    /** Rotate the matrix about the x-axis by an angle in radians */
    static Mat3x4 fromRotationX(float radians);
    
    /** Rotate the matrix about the y-axis by an angle in radians */
    static Mat3x4 fromRotationY(float radians);
    
    /** Rotate the matrix about the z-axis by an angle in radians */
    static Mat3x4 fromRotationZ(float radians);
    
    /** Rotate the matrix about the z-axis by an angle in radians */
    static Mat3x4 fromTranslation(Vec3 xyz);
    
    /** Rotate the matrix about the z-axis by an angle in radians */
    static Mat3x4 fromScale(Vec3 xyz);
    
    /** Expand a Mat3x3 to a Mat3x4 with zero translation */
    static Mat3x4 fromMat3x3(const Mat3x3& matrix);
    
    /** Compute a Transform to Mat3x4 */
    static Mat3x4 fromTransform(const Transform& transform);
};

inline Mat3x4 operator*(const Mat3x4& lhs, const Mat3x4& rhs)
{
    return Mat3x4{
        lhs.m00 * rhs.m00 + lhs.m01 * rhs.m10 + lhs.m02 * rhs.m20 + lhs.m03 * 0,
        lhs.m00 * rhs.m01 + lhs.m01 * rhs.m11 + lhs.m02 * rhs.m21 + lhs.m03 * 0,
        lhs.m00 * rhs.m02 + lhs.m01 * rhs.m12 + lhs.m02 * rhs.m22 + lhs.m03 * 0,
        lhs.m00 * rhs.m03 + lhs.m01 * rhs.m13 + lhs.m02 * rhs.m23 + lhs.m03 * 1,

        lhs.m10 * rhs.m00 + lhs.m11 * rhs.m10 + lhs.m12 * rhs.m20 + lhs.m13 * 0,
        lhs.m10 * rhs.m01 + lhs.m11 * rhs.m11 + lhs.m12 * rhs.m21 + lhs.m13 * 0,
        lhs.m10 * rhs.m02 + lhs.m11 * rhs.m12 + lhs.m12 * rhs.m22 + lhs.m13 * 0,
        lhs.m10 * rhs.m03 + lhs.m11 * rhs.m13 + lhs.m12 * rhs.m23 + lhs.m13 * 1,

        lhs.m20 * rhs.m00 + lhs.m21 * rhs.m10 + lhs.m22 * rhs.m20 + lhs.m23 * 0,
        lhs.m20 * rhs.m01 + lhs.m21 * rhs.m11 + lhs.m22 * rhs.m21 + lhs.m23 * 0,
        lhs.m20 * rhs.m02 + lhs.m21 * rhs.m12 + lhs.m22 * rhs.m22 + lhs.m23 * 0,
        lhs.m20 * rhs.m03 + lhs.m21 * rhs.m13 + lhs.m22 * rhs.m23 + lhs.m23 * 1,
    };
}

inline Vec3 operator*(const Mat3x4& lhs, const Vec3& rhs)
{
    return Vec3{
        lhs.m00 * rhs.x + lhs.m01 * rhs.y + lhs.m02 * rhs.z + lhs.m03 * 1,
        lhs.m10 * rhs.x + lhs.m11 * rhs.y + lhs.m12 * rhs.z + lhs.m13 * 1,
        lhs.m20 * rhs.x + lhs.m21 * rhs.y + lhs.m22 * rhs.z + lhs.m23 * 1,
    };
}

inline bool operator==(const Mat3x4& lhs, const Mat3x4& rhs)
{
    return lhs.m00 == rhs.m00 && lhs.m01 == rhs.m01 && lhs.m02 == rhs.m02 && lhs.m03 == rhs.m03 &&
           lhs.m10 == rhs.m10 && lhs.m11 == rhs.m11 && lhs.m12 == rhs.m12 && lhs.m13 == rhs.m13 &&
           lhs.m20 == rhs.m20 && lhs.m21 == rhs.m21 && lhs.m22 == rhs.m22 && lhs.m23 == rhs.m23;
}

inline bool operator!=(const Mat3x4& lhs, const Mat3x4& rhs)
{
    return !(lhs == rhs);
}

inline bool Mat3x4::almostEqual(const Mat3x4& lhs, const Mat3x4& rhs, float relativeTolerance, float absoluteTolerance)
{
    return AlmostEqual(lhs.m00, rhs.m00, relativeTolerance, absoluteTolerance) &&
           AlmostEqual(lhs.m01, rhs.m01, relativeTolerance, absoluteTolerance) &&
           AlmostEqual(lhs.m02, rhs.m02, relativeTolerance, absoluteTolerance) &&
           AlmostEqual(lhs.m03, rhs.m03, relativeTolerance, absoluteTolerance) &&
    
           AlmostEqual(lhs.m10, rhs.m10, relativeTolerance, absoluteTolerance) &&
           AlmostEqual(lhs.m11, rhs.m11, relativeTolerance, absoluteTolerance) &&
           AlmostEqual(lhs.m12, rhs.m12, relativeTolerance, absoluteTolerance) &&
           AlmostEqual(lhs.m13, rhs.m13, relativeTolerance, absoluteTolerance) &&
    
           AlmostEqual(lhs.m20, rhs.m20, relativeTolerance, absoluteTolerance) &&
           AlmostEqual(lhs.m21, rhs.m21, relativeTolerance, absoluteTolerance) &&
           AlmostEqual(lhs.m22, rhs.m22, relativeTolerance, absoluteTolerance) &&
           AlmostEqual(lhs.m23, rhs.m23, relativeTolerance, absoluteTolerance);
}

inline Mat3x4 Mat3x4::fromRowMajorVector(const std::vector<float>& data)
{
    SCASSERT(data.size() == 12, "Expected length of data to be 12");
    return Mat3x4{
        data[0], data[1], data[2], data[3],
        data[4], data[5], data[6], data[7],
        data[8], data[9], data[10], data[11]
    };
}

inline Mat3x4 Mat3x4::fromColumnMajorVector(const std::vector<float>& data)
{
    SCASSERT(data.size() == 12, "Expected length of data to be 12");
    return Mat3x4{
        data[0], data[3], data[6], data[9],
        data[1], data[4], data[7], data[10],
        data[2], data[5], data[8], data[11]
    };
}

inline std::vector<float> Mat3x4::toColumnMajorVector() const
{
    return std::vector<float>{m00, m10, m20, m01, m11, m21, m02, m12, m22, m03, m13, m23};
}

inline std::vector<float> Mat3x4::toRowMajorVector() const
{
    return std::vector<float>{m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23};
}

} // namespace math
} // namespace standard_cyborg
