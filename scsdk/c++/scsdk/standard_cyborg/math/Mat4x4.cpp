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

#include "standard_cyborg/math/Mat4x4.hpp"
#include "standard_cyborg/math/Mat3x4.hpp"

#include <cmath>

namespace standard_cyborg {
namespace math {

// clang-format off

Mat4x4::Mat4x4(float m00_, float m01_, float m02_, float m03_,
               float m10_, float m11_, float m12_, float m13_,
               float m20_, float m21_, float m22_, float m23_,
               float m30_, float m31_, float m32_, float m33_) :
    m00(m00_), m01(m01_), m02(m02_), m03(m03_),
    m10(m10_), m11(m11_), m12(m12_), m13(m13_),
    m20(m20_), m21(m21_), m22(m22_), m23(m23_),
    m30(m30_), m31(m31_), m32(m32_), m33(m33_)
{}

Mat4x4::Mat4x4() :
    m00(1), m01(0), m02(0), m03(0),
    m10(0), m11(1), m12(0), m13(0),
    m20(0), m21(0), m22(1), m23(0),
    m30(0), m31(0), m32(0), m33(1)
{}

Mat4x4::Mat4x4(const Mat3x4& matrix) :
    m00(matrix.m00), m01(matrix.m01), m02(matrix.m02), m03(matrix.m03),
    m10(matrix.m10), m11(matrix.m11), m12(matrix.m12), m13(matrix.m03),
    m20(matrix.m20), m21(matrix.m21), m22(matrix.m22), m23(matrix.m23),
    m30(0), m31(0), m32(0), m33(1)
{}

// clang-format on

Mat4x4 Mat4x4::Identity()
{
    return Mat4x4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );
}

Mat4x4 Mat4x4::Zeros()
{
    return Mat4x4(
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
    );
}

    
Mat4x4 Mat4x4::inverse() const
{
    float b00 = m00 * m11 - m01 * m10;
    float b01 = m00 * m12 - m02 * m10;
    float b02 = m00 * m13 - m03 * m10;
    float b03 = m01 * m12 - m02 * m11;
    float b04 = m01 * m13 - m03 * m11;
    float b05 = m02 * m13 - m03 * m12;
    float b06 = m20 * m31 - m21 * m30;
    float b07 = m20 * m32 - m22 * m30;
    float b08 = m20 * m33 - m23 * m30;
    float b09 = m21 * m32 - m22 * m31;
    float b10 = m21 * m33 - m23 * m31;
    float b11 = m22 * m33 - m23 * m32;

    // Calculate the determinant
    float det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;

    if (det == 0.0f) {
        return Mat4x4({NAN, NAN, NAN, NAN,
                       NAN, NAN, NAN, NAN,
                       NAN, NAN, NAN, NAN,
                       NAN, NAN, NAN, NAN});
    } else {
        float invDet = 1.0f / det;

        return Mat4x4{
            (m11 * b11 - m12 * b10 + m13 * b09) * invDet,
            (m02 * b10 - m01 * b11 - m03 * b09) * invDet,
            (m31 * b05 - m32 * b04 + m33 * b03) * invDet,
            (m22 * b04 - m21 * b05 - m23 * b03) * invDet,
            (m12 * b08 - m10 * b11 - m13 * b07) * invDet,
            (m00 * b11 - m02 * b08 + m03 * b07) * invDet,
            (m32 * b02 - m30 * b05 - m33 * b01) * invDet,
            (m20 * b05 - m22 * b02 + m23 * b01) * invDet,
            (m10 * b10 - m11 * b08 + m13 * b06) * invDet,
            (m01 * b08 - m00 * b10 - m03 * b06) * invDet,
            (m30 * b04 - m31 * b02 + m33 * b00) * invDet,
            (m21 * b02 - m20 * b04 - m23 * b00) * invDet,
            (m11 * b07 - m10 * b09 - m12 * b06) * invDet,
            (m00 * b09 - m01 * b07 + m02 * b06) * invDet,
            (m31 * b01 - m30 * b03 - m32 * b00) * invDet,
            (m20 * b03 - m21 * b01 + m22 * b00) * invDet};
    }
}

Mat4x4& Mat4x4::invert()
{
    float a00 = m00;
    float a01 = m01;
    float a02 = m02;
    float a03 = m03;

    float a10 = m10;
    float a11 = m11;
    float a12 = m12;
    float a13 = m13;
    
    float a20 = m20;
    float a21 = m21;
    float a22 = m22;
    float a23 = m23;
    
    float a30 = m30;
    float a31 = m31;
    float a32 = m32;
    float a33 = m33;
    
    float b00 = a00 * a11 - a01 * a10;
    float b01 = a00 * a12 - a02 * a10;
    float b02 = a00 * a13 - a03 * a10;
    float b03 = a01 * a12 - a02 * a11;
    float b04 = a01 * a13 - a03 * a11;
    float b05 = a02 * a13 - a03 * a12;
    float b06 = a20 * a31 - a21 * a30;
    float b07 = a20 * a32 - a22 * a30;
    float b08 = a20 * a33 - a23 * a30;
    float b09 = a21 * a32 - a22 * a31;
    float b10 = a21 * a33 - a23 * a31;
    float b11 = a22 * a33 - a23 * a32;

    // Calculate the determinant
    float det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;

    if (det == 0.0f) {
        m00 = NAN;
        m01 = NAN;
        m02 = NAN;
        m03 = NAN;
        m10 = NAN;
        m11 = NAN;
        m12 = NAN;
        m13 = NAN;
        m20 = NAN;
        m21 = NAN;
        m22 = NAN;
        m23 = NAN;
        m30 = NAN;
        m31 = NAN;
        m32 = NAN;
        m33 = NAN;
    } else {
        float invDet = 1.0f / det;

        m00 = (a11 * b11 - a12 * b10 + a13 * b09) * invDet;
        m01 = (a02 * b10 - a01 * b11 - a03 * b09) * invDet;
        m02 = (a31 * b05 - a32 * b04 + a33 * b03) * invDet;
        m03 = (a22 * b04 - a21 * b05 - a23 * b03) * invDet;
        m10 = (a12 * b08 - a10 * b11 - a13 * b07) * invDet;
        m11 = (a00 * b11 - a02 * b08 + a03 * b07) * invDet;
        m12 = (a32 * b02 - a30 * b05 - a33 * b01) * invDet;
        m13 = (a20 * b05 - a22 * b02 + a23 * b01) * invDet;
        m20 = (a10 * b10 - a11 * b08 + a13 * b06) * invDet;
        m21 = (a01 * b08 - a00 * b10 - a03 * b06) * invDet;
        m22 = (a30 * b04 - a31 * b02 + a33 * b00) * invDet;
        m23 = (a21 * b02 - a20 * b04 - a23 * b00) * invDet;
        m30 = (a11 * b07 - a10 * b09 - a12 * b06) * invDet;
        m31 = (a00 * b09 - a01 * b07 + a02 * b06) * invDet;
        m32 = (a31 * b01 - a30 * b03 - a32 * b00) * invDet;
        m33 = (a20 * b03 - a21 * b01 + a22 * b00) * invDet;
    }
    
    return *this;
}

} // namespace math
} // namespace standard_cyborg
