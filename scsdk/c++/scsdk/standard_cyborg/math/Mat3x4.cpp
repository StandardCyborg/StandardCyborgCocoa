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

#include "standard_cyborg/math/Mat3x4.hpp"

#include "standard_cyborg/math/Mat3x3.hpp"
#include "standard_cyborg/math/Mat4x4.hpp"
#include "standard_cyborg/math/Transform.hpp"

#include <cmath>

namespace standard_cyborg {
namespace math {

// clang-format off
Mat3x4::Mat3x4(float m00_, float m01_, float m02_, float m03_,
               float m10_, float m11_, float m12_, float m13_,
               float m20_, float m21_, float m22_, float m23_) :
    m00(m00_), m01(m01_), m02(m02_), m03(m03_),
    m10(m10_), m11(m11_), m12(m12_), m13(m13_),
    m20(m20_), m21(m21_), m22(m22_), m23(m23_)
{}

Mat3x4::Mat3x4(Mat3x3 transform, Vec3 translation) :
m00(transform.m00), m01(transform.m01), m02(transform.m02), m03(translation.x),
m10(transform.m10), m11(transform.m11), m12(transform.m12), m13(translation.y),
m20(transform.m20), m21(transform.m21), m22(transform.m22), m23(translation.z)
{}

Mat3x4::Mat3x4(const Mat4x4& matrix) :
    m00(matrix.m00), m01(matrix.m01), m02(matrix.m02), m03(matrix.m03),
    m10(matrix.m10), m11(matrix.m11), m12(matrix.m12), m13(matrix.m03),
    m20(matrix.m20), m21(matrix.m21), m22(matrix.m22), m23(matrix.m23)
{}

Mat3x4::Mat3x4() :
    m00(1), m01(0), m02(0), m03(0),
    m10(0), m11(1), m12(0), m13(0),
    m20(0), m21(0), m22(1), m23(0)
{}
// clang-format on

Mat3x4 Mat3x4::Identity()
{
    return Mat3x4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0
    );
}

Mat3x4 Mat3x4::Zeros()
{
    return Mat3x4(
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
    );
}

Mat3x4 Mat3x4::fromMat4x4(const Mat4x4 transform) {
    return Mat3x4(
        transform.m00, transform.m01, transform.m02, transform.m03,
        transform.m10, transform.m11, transform.m12, transform.m13,
        transform.m20, transform.m21, transform.m22, transform.m23
    );
}

Mat3x4 Mat3x4::inverse() const
{
    float a0 = m00 * m11 - m01 * m10;
    float a1 = m00 * m12 - m02 * m10;
    float a2 = m00 * m13 - m03 * m10;
    float a3 = m01 * m12 - m02 * m11;
    float a4 = m01 * m13 - m03 * m11;
    float a5 = m02 * m13 - m03 * m12;

    // Calculate the determinant
    float det = a0 * m22 - a1 * m21 + a3 * m20;

    if (det == 0.0f) {
        Mat3x4({NAN, NAN, NAN, NAN,
                NAN, NAN, NAN, NAN,
                NAN, NAN, NAN, NAN});
    }
    
    det = 1.0f / det;

    // clang-format off
    return Mat3x4({
        (m11 * m22 - m12 * m21) * det,
        (m02 * m21 - m01 * m22) * det,
        (a3) * det,
        (m22 * a4 - m21 * a5 - m23 * a3) * det,
        
        (m12 * m20 - m10 * m22) * det,
        (m00 * m22 - m02 * m20) * det,
        (-a1) * det,
        (m20 * a5 - m22 * a2 + m23 * a1) * det,
        
        (m10 * m21 - m11 * m20) * det,
        (m01 * m20 - m00 * m21) * det,
        (a0) * det,
        (m21 * a2 - m20 * a4 - m23 * a0) * det
    });
    // clang-format on
}

Mat3x4& Mat3x4::invert()
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
    
    float b0 = a00 * a11 - a01 * a10;
    float b1 = a00 * a12 - a02 * a10;
    float b2 = a00 * a13 - a03 * a10;
    float b3 = a01 * a12 - a02 * a11;
    float b4 = a01 * a13 - a03 * a11;
    float b5 = a02 * a13 - a03 * a12;

    // Calculate the determinant
    float det = b0 * a22 - b1 * a21 + b3 * a20;

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
    } else {
        det = 1.0f / det;

        m00 = (a11 * a22 - a12 * a21) * det;
        m01 = (a02 * a21 - a01 * a22) * det;
        m02 = b3 * det;
        m03 = (a22 * b4 - a21 * b5 - a23 * b3) * det;
        
        m10 = (a12 * a20 - a10 * a22) * det;
        m11 = (a00 * a22 - a02 * a20) * det;
        m12 = (-b1) * det;
        m13 = (a20 * b5 - a22 * b2 + a23 * b1) * det;
        
        m20 = (a10 * a21 - a11 * a20) * det;
        m21 = (a01 * a20 - a00 * a21) * det;
        m22 = b0 * det;
        m23 = (a21 * b2 - a20 * b4 - a23 * b0) * det;
    }
    
    return *this;
}

Mat3x4 Mat3x4::fromRotationZ(float radians)
{
    float s = std::sin(radians);
    float c = std::cos(radians);
    
    return Mat3x4 {
           c,    s, 0.0f, 0.0f,
          -s,    c, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f
    };
}

Mat3x4 Mat3x4::fromRotationX(float radians) {
    float s = std::sin(radians);
    float c = std::cos(radians);
    
    return Mat3x4 {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f,    c,    s, 0.0f,
        0.0f,   -s,    c, 0.0f
    };
}

Mat3x4 Mat3x4::fromRotationY(float radians) {
    float s = std::sin(radians);
    float c = std::cos(radians);

    return Mat3x4 {
           c, 0.0f,   -s, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
           s, 0.0f,    c, 0.0f
    };
}

Mat3x4 Mat3x4::fromTranslation(Vec3 xyz) {
    return Mat3x4 {
        1.0f, 0.0f, 0.0f, xyz.x,
        0.0f, 1.0f, 0.0f, xyz.y,
        0.0f, 0.0f, 1.0f, xyz.z
    };
}

Mat3x4 Mat3x4::fromScale(Vec3 xyz) {
    return Mat3x4 {
        xyz.x, 0.0f, 0.0f, 0.0f,
        0.0f, xyz.y, 0.0f, 0.0f,
        0.0f, 0.0f, xyz.z, 0.0f
    };
}

Mat3x4 Mat3x4::fromMat3x3(const Mat3x3& matrix) {
    return Mat3x4 {
        matrix.m00, matrix.m01, matrix.m02, 0.0f,
        matrix.m10, matrix.m11, matrix.m12, 0.0f,
        matrix.m20, matrix.m21, matrix.m22, 0.0f
    };
}

Mat3x4 Mat3x4::fromTransform(const Transform& t) {
    Mat3x3 R (Mat3x3::fromQuaternion(t.rotation));
    Mat3x3 S {
        1.0f, t.shear[0], t.shear[1],
        0.0f, 1.0f, t.shear[2],
        0.0f, 0.0f, 1.0f
    };
    Mat3x3 Z (Mat3x3::fromDiagonal(t.scale));

    return Mat3x4(R * Z * S, t.translation);
}

} // namespace math
} // namespace standard_cyborg
