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

#include "standard_cyborg/math/Mat3x3.hpp"

#include "standard_cyborg/math/Mat3x4.hpp"
#include "standard_cyborg/math/Quaternion.hpp"

// normalMatrix, inverse, and fromQuaternion have been adapted from https://github.com/stackgl/gl-mat3
//
// normalMatrix: https://github.com/stackgl/gl-mat3/blob/df6df371ab119fb0c9f2b428d3a4f8d05cd2119e/normalFromMat4.js
// inverse: https://github.com/stackgl/gl-mat3/blob/df6df371ab119fb0c9f2b428d3a4f8d05cd2119e/invert.js
// fromQuat: https://github.com/stackgl/gl-mat3/blob/master/fromQuat.js
//
// Copyright (c) 2013 Brandon Jones, Colin MacKenzie IV, Hugh Kennedy
//
// This software is provided 'as-is', without any express or implied warranty. In no event will
// the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including commercial
// applications, and to alter it and redistribute it freely, subject to the following restrictions:
//
// The origin of this software must not be misrepresented you must not claim that you wrote the
// original software. If you use this software in a product, an acknowledgment in the product
// documentation would be appreciated but is not required.
//
// Altered source versions must be plainly marked as such, and must not be misrepresented as being
// the original software.
//
// This notice may not be removed or altered from any source distribution.

namespace standard_cyborg {
namespace math {

// clang-format off
Mat3x3::Mat3x3(float m00_, float m01_, float m02_,
               float m10_, float m11_, float m12_,
               float m20_, float m21_, float m22_) :
    m00(m00_), m01(m01_), m02(m02_),
    m10(m10_), m11(m11_), m12(m12_),
    m20(m20_), m21(m21_), m22(m22_)
{}


Mat3x3::Mat3x3() :
    m00(1), m01(0), m02(0),
    m10(0), m11(1), m12(0),
    m20(0), m21(0), m22(1)
{}

Mat3x3 Mat3x3::Identity()
{
    return Mat3x3(
        1, 0, 0,
        0, 1, 0,
        0, 0, 1
    );
}

Mat3x3 Mat3x3::Zeros()
{
    return Mat3x3(
        0, 0, 0,
        0, 0, 0,
        0, 0, 0
    );
}

Mat3x3 Mat3x3::normalMatrix(const Mat3x4& mat)
{
    float b00 = mat.m00 * mat.m11 - mat.m01 * mat.m10;
    float b01 = mat.m00 * mat.m12 - mat.m02 * mat.m10;
    float b03 = mat.m01 * mat.m12 - mat.m02 * mat.m11;

    // Calculate the reciprocal of the determinant
    float detRecip = 1.0f / (b00 * mat.m22 - b01 * mat.m21 + b03 * mat.m20);

    return Mat3x3({
        (mat.m11 * mat.m22 - mat.m12 * mat.m21) * detRecip,
        (mat.m12 * mat.m20 - mat.m10 * mat.m22) * detRecip,
        (mat.m10 * mat.m21 - mat.m11 * mat.m20) * detRecip,

        (mat.m02 * mat.m21 - mat.m01 * mat.m22) * detRecip,
        (mat.m00 * mat.m22 - mat.m02 * mat.m20) * detRecip,
        (mat.m01 * mat.m20 - mat.m00 * mat.m21) * detRecip,

        b03 * detRecip,
        -b01 * detRecip,
        b00 * detRecip
    });
}
// clang-format on

Mat3x3 Mat3x3::inverse() const
{
    float a0 = m00 * m11 - m01 * m10;
    float a1 = m00 * m12 - m02 * m10;
    float a2 = m01 * m12 - m02 * m11;

    // Calculate the determinant
    float det = a0 * m22 - a1 * m21 + a2 * m20;

    if (det == 0.0f) {
        return Mat3x3({NAN, NAN, NAN,
                       NAN, NAN, NAN,
                       NAN, NAN, NAN});
    }
    
    det = 1.0f / det;

    return Mat3x3({
        (m11 * m22 - m12 * m21) * det,
        (m02 * m21 - m01 * m22) * det,
        a2 * det,

        (m12 * m20 - m10 * m22) * det,
        (m00 * m22 - m02 * m20) * det,
        -a1 * det,

        (m10 * m21 - m11 * m20) * det,
        (m01 * m20 - m00 * m21) * det,
        a0 * det,
    });
}

Mat3x3& Mat3x3::invert()
{
    float a00 = m00;
    float a01 = m01;
    float a02 = m02;
    float a10 = m10;
    float a11 = m11;
    float a12 = m12;
    float a20 = m20;
    float a21 = m21;
    float a22 = m22;
    
    float b0 = a00 * a11 - a01 * a10;
    float b1 = a00 * a12 - a02 * a10;
    float b2 = a01 * a12 - a02 * a11;

    // Calculate the determinant
    float det = b0 * a22 - b1 * a21 + b2 * a20;

    if (det == 0.0f) {
        m00 = NAN;
        m01 = NAN;
        m02 = NAN;
        
        m10 = NAN;
        m11 = NAN;
        m12 = NAN;

        m20 = NAN;
        m21 = NAN;
        m22 = NAN;
    } else {
        det = 1.0f / det;

        m00 = (a11 * a22 - a12 * a21) * det;
        m01 = (a02 * a21 - a01 * a22) * det;
        m02 = b2 * det;

        m10 = (a12 * a20 - a10 * a22) * det;
        m11 = (a00 * a22 - a02 * a20) * det;
        m12 = -b1 * det;

        m20 = (a10 * a21 - a11 * a20) * det;
        m21 = (a01 * a20 - a00 * a21) * det;
        m22 = b0 * det;
    }
    
    return *this;
}

Mat3x3 Mat3x3::fromQuaternion(Quaternion q) {
    float x = q.x;
    float y = q.y;
    float z = q.z;
    float w = q.w;
    
    float x2 = x + x;
    float y2 = y + y;
    float z2 = z + z;
    
    float xx = x * x2;
    float yx = y * x2;
    float yy = y * y2;
    float zx = z * x2;
    float zy = z * y2;
    float zz = z * z2;
    float wx = w * x2;
    float wy = w * y2;
    float wz = w * z2;

    return Mat3x3 {
        1.0f - yy - zz,
        yx - wz,
        zx + wy,
        
        yx + wz,
        1.0f - xx - zz,
        zy - wx,
        
        zx - wy,
        zy + wx,
        1.0f - xx - yy
    };
}

} // namespace math
} // namespace standard_cyborg
