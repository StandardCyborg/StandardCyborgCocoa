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

struct Mat3x4;
    
struct Mat4x4
{
    // note: we use row-major order for our matrices.
    float m00, m01, m02, m03,
          m10, m11, m12, m13,
          m20, m21, m22, m23,
          m30, m31, m32, m33;

    Mat4x4();
    
    Mat4x4(const Mat3x4& matrix);

    Mat4x4(float m00_, float m01_, float m02_, float m03_,
           float m10_, float m11_, float m12_, float m13_,
           float m20_, float m21_, float m22_, float m23_,
           float m30_, float m31_, float m32_, float m33_);
    
    static Mat4x4 Identity();
    static Mat4x4 Zeros();
    
    static Mat4x4 fromColumnMajorVector(const std::vector<float>& data);
    static Mat4x4 fromRowMajorVector(const std::vector<float>& data);

    std::vector<float> toColumnMajorVector() const;
    std::vector<float> toRowMajorVector() const;
    
    /** Compute whether two matrices are equal to within floating point epsilon */
    static inline bool almostEqual(
        const Mat4x4& lhs,
        const Mat4x4& rhs,
        float relativeTolerance = std::numeric_limits<float>::epsilon(),
        float absoluteTolerance = std::numeric_limits<float>::epsilon()
    );

    /** Invert the matrix in-place, returning a reference to the inverted matrix */
    Mat4x4& invert();
    
    /** Return an inverted copy of the matrix */
    Mat4x4 inverse() const;
};

inline Mat4x4 operator*(const Mat4x4& lhs, const Mat4x4& rhs)
{
    return Mat4x4{
        lhs.m00 * rhs.m00 + lhs.m01 * rhs.m10 + lhs.m02 * rhs.m20 + lhs.m03 * rhs.m30,
        lhs.m00 * rhs.m01 + lhs.m01 * rhs.m11 + lhs.m02 * rhs.m21 + lhs.m03 * rhs.m31,
        lhs.m00 * rhs.m02 + lhs.m01 * rhs.m12 + lhs.m02 * rhs.m22 + lhs.m03 * rhs.m32,
        lhs.m00 * rhs.m03 + lhs.m01 * rhs.m13 + lhs.m02 * rhs.m23 + lhs.m03 * rhs.m33,

        lhs.m10 * rhs.m00 + lhs.m11 * rhs.m10 + lhs.m12 * rhs.m20 + lhs.m13 * rhs.m30,
        lhs.m10 * rhs.m01 + lhs.m11 * rhs.m11 + lhs.m12 * rhs.m21 + lhs.m13 * rhs.m31,
        lhs.m10 * rhs.m02 + lhs.m11 * rhs.m12 + lhs.m12 * rhs.m22 + lhs.m13 * rhs.m32,
        lhs.m10 * rhs.m03 + lhs.m11 * rhs.m13 + lhs.m12 * rhs.m23 + lhs.m13 * rhs.m33,

        lhs.m20 * rhs.m00 + lhs.m21 * rhs.m10 + lhs.m22 * rhs.m20 + lhs.m23 * rhs.m30,
        lhs.m20 * rhs.m01 + lhs.m21 * rhs.m11 + lhs.m22 * rhs.m21 + lhs.m23 * rhs.m31,
        lhs.m20 * rhs.m02 + lhs.m21 * rhs.m12 + lhs.m22 * rhs.m22 + lhs.m23 * rhs.m32,
        lhs.m20 * rhs.m03 + lhs.m21 * rhs.m13 + lhs.m22 * rhs.m23 + lhs.m23 * rhs.m33,

        lhs.m30 * rhs.m00 + lhs.m31 * rhs.m10 + lhs.m32 * rhs.m20 + lhs.m33 * rhs.m30,
        lhs.m30 * rhs.m01 + lhs.m31 * rhs.m11 + lhs.m32 * rhs.m21 + lhs.m33 * rhs.m31,
        lhs.m30 * rhs.m02 + lhs.m31 * rhs.m12 + lhs.m32 * rhs.m22 + lhs.m33 * rhs.m32,
        lhs.m30 * rhs.m03 + lhs.m31 * rhs.m13 + lhs.m32 * rhs.m23 + lhs.m33 * rhs.m33
    };
}

inline Vec3 operator*(const Mat4x4& lhs, const Vec3& rhs)
{
    float wInv =  1.0 / (lhs.m30 * rhs.x + lhs.m31 * rhs.y + lhs.m32 * rhs.z + lhs.m33);
    
    return Vec3{
        (lhs.m00 * rhs.x + lhs.m01 * rhs.y + lhs.m02 * rhs.z + lhs.m03 * 1) * wInv,
        (lhs.m10 * rhs.x + lhs.m11 * rhs.y + lhs.m12 * rhs.z + lhs.m13 * 1) * wInv,
        (lhs.m20 * rhs.x + lhs.m21 * rhs.y + lhs.m22 * rhs.z + lhs.m23 * 1) * wInv
    };
}

inline bool operator==(const Mat4x4& lhs, const Mat4x4& rhs)
{
    return lhs.m00 == rhs.m00 && lhs.m01 == rhs.m01 && lhs.m02 == rhs.m02 && lhs.m03 == rhs.m03 &&
           lhs.m10 == rhs.m10 && lhs.m11 == rhs.m11 && lhs.m12 == rhs.m12 && lhs.m13 == rhs.m13 &&
           lhs.m20 == rhs.m20 && lhs.m21 == rhs.m21 && lhs.m22 == rhs.m22 && lhs.m23 == rhs.m23 &&
           lhs.m30 == rhs.m30 && lhs.m31 == rhs.m31 && lhs.m32 == rhs.m32 && lhs.m33 == rhs.m33;
}

inline bool operator!=(const Mat4x4& lhs, const Mat4x4& rhs)
{
    return !(lhs == rhs);
}

inline bool Mat4x4::almostEqual(const Mat4x4& lhs, const Mat4x4& rhs, float relativeTolerance, float absoluteTolerance)
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
           AlmostEqual(lhs.m23, rhs.m23, relativeTolerance, absoluteTolerance) &&
    
           AlmostEqual(lhs.m30, rhs.m30, relativeTolerance, absoluteTolerance) &&
           AlmostEqual(lhs.m31, rhs.m31, relativeTolerance, absoluteTolerance) &&
           AlmostEqual(lhs.m32, rhs.m32, relativeTolerance, absoluteTolerance) &&
           AlmostEqual(lhs.m33, rhs.m33, relativeTolerance, absoluteTolerance);
}

inline Mat4x4 Mat4x4::fromRowMajorVector(const std::vector<float>& data)
{
    SCASSERT(data.size() == 16, "Expected length of data to be 16");
    return Mat4x4{
        data[0], data[1], data[2], data[3],
        data[4], data[5], data[6], data[7],
        data[8], data[9], data[10], data[11],
        data[12], data[13], data[14], data[15]
    };
}

inline Mat4x4 Mat4x4::fromColumnMajorVector(const std::vector<float>& data)
{
    SCASSERT(data.size() == 16, "Expected length of data to be 16");
    return Mat4x4{
        data[0], data[4], data[8], data[12],
        data[1], data[5], data[9], data[13],
        data[2], data[6], data[10], data[14],
        data[3], data[7], data[11], data[15]
    };
}

inline std::vector<float> Mat4x4::toColumnMajorVector() const
{
    return std::vector<float>{m00, m10, m20, m30, m01, m11, m21, m31, m02, m12, m22, m32, m03, m13, m23, m33};
}

inline std::vector<float> Mat4x4::toRowMajorVector() const
{
    return std::vector<float>{m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33};
}

} // namespace math
} // namespace standard_cyborg
