//
//  Mat3x4.hpp
//  StandardCyborgGeometry
//
//  Created by eric on 2019-04-05.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <limits>
#include <vector>

#include <StandardCyborgData/Vec3.hpp>

namespace StandardCyborg {

struct Mat3x4 {
    // note: we use row-major order for our matrices.
    float m00, m01, m02, m03,
          m10, m11, m12, m13,
          m20, m21, m22, m23;
    
    Mat3x4();
    
    Mat3x4(float m00_, float m01_, float m02_, float m03_,
           float m10_, float m11_, float m12_, float m13_,
           float m20_, float m21_, float m22_, float m23_);
    
    static Mat3x4 Identity();
    static Mat3x4 Zeros();
    
    static Mat3x4 fromColumnMajorVector(const std::vector<float>& data);
    static Mat3x4 fromRowMajorVector(const std::vector<float>& data);
    
    std::vector<float> toColumnMajorVector() const;
    std::vector<float> toRowMajorVector() const;
    
    /* Compute whether two matrices are equal to within floating point epsilon */
    static inline bool almostEqual(
        const Mat3x4& lhs,
        const Mat3x4& rhs,
        float relativeTolerance = std::numeric_limits<float>::epsilon(),
        float absoluteTolerance = std::numeric_limits<float>::epsilon()
    );
    
    /* Invert in-place */
    Mat3x4& invert();
    
    /* Return an inverted copy */
    Mat3x4 inverse() const;
    
    /* Sets the translation components to the specified coordinates */
    Mat3x4& scale(Vec3 xyz);
    
    /* Sets the translation components to the specified coordinates */
    Mat3x4& translate(Vec3 xyz);
    
    /* Rotate the matrix about the x-axis by an angle in radians */
    Mat3x4& rotateX(float radians);
    
    /* Rotate the matrix about the y-axis by an angle in radians */
    Mat3x4& rotateY(float radians);
    
    /* Rotate the matrix about the z-axis by an angle in radians */
    Mat3x4& rotateZ(float radians);
    
    /* Rotate the matrix about the x-axis by an angle in radians */
    static Mat3x4 fromRotationX(float radians);
    
    /* Rotate the matrix about the y-axis by an angle in radians */
    static Mat3x4 fromRotationY(float radians);
    
    /* Rotate the matrix about the z-axis by an angle in radians */
    static Mat3x4 fromRotationZ(float radians);
    
    /* Rotate the matrix about the z-axis by an angle in radians */
    static Mat3x4 fromTranslation(Vec3 xyz);
    
    /* Rotate the matrix about the z-axis by an angle in radians */
    static Mat3x4 fromScale(Vec3 xyz);
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
    return StandardCyborg::almostEqual(lhs.m00, rhs.m00, relativeTolerance, absoluteTolerance) &&
           StandardCyborg::almostEqual(lhs.m01, rhs.m01, relativeTolerance, absoluteTolerance) &&
           StandardCyborg::almostEqual(lhs.m02, rhs.m02, relativeTolerance, absoluteTolerance) &&
           StandardCyborg::almostEqual(lhs.m03, rhs.m03, relativeTolerance, absoluteTolerance) &&
    
           StandardCyborg::almostEqual(lhs.m10, rhs.m10, relativeTolerance, absoluteTolerance) &&
           StandardCyborg::almostEqual(lhs.m11, rhs.m11, relativeTolerance, absoluteTolerance) &&
           StandardCyborg::almostEqual(lhs.m12, rhs.m12, relativeTolerance, absoluteTolerance) &&
           StandardCyborg::almostEqual(lhs.m13, rhs.m13, relativeTolerance, absoluteTolerance) &&
    
           StandardCyborg::almostEqual(lhs.m20, rhs.m20, relativeTolerance, absoluteTolerance) &&
           StandardCyborg::almostEqual(lhs.m21, rhs.m21, relativeTolerance, absoluteTolerance) &&
           StandardCyborg::almostEqual(lhs.m22, rhs.m22, relativeTolerance, absoluteTolerance) &&
           StandardCyborg::almostEqual(lhs.m23, rhs.m23, relativeTolerance, absoluteTolerance);
}

inline Mat3x4 Mat3x4::fromRowMajorVector(const std::vector<float>& data)
{
    return Mat3x4{
        data[0], data[1], data[2], data[3],
        data[4], data[5], data[6], data[7],
        data[8], data[9], data[10], data[11]
    };
}

inline Mat3x4 Mat3x4::fromColumnMajorVector(const std::vector<float>& data)
{
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

} // namespace StandardCyborg
