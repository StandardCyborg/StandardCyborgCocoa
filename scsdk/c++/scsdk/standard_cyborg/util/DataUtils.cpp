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

#include "standard_cyborg/util/DataUtils.hpp"

#include "standard_cyborg/math/Mat3x3.hpp"
#include "standard_cyborg/math/Mat3x4.hpp"
#include "standard_cyborg/math/Mat4x4.hpp"
#include "standard_cyborg/math/Vec3.hpp"
#include "standard_cyborg/sc3d/Face3.hpp"

namespace standard_cyborg {

// MARK: non-const Vec3 adaptors

using math::Vec3;
using math::Mat3x4;
using math::Mat4x4;
using math::Mat3x3;
using sc3d::Face3;

Eigen::Ref<Eigen::Matrix<float, 3, Eigen::Dynamic>> toMatrix3Xf(std::vector<Vec3>& data)
{
    return Eigen::Map<Eigen::Matrix<float, 3, Eigen::Dynamic>, Eigen::Aligned, Eigen::Stride<sizeof(Vec3) / sizeof(float), 1>>(
        (float*)data.data(), 3, data.size()
    );
}

Eigen::Ref<Eigen::Matrix<float, Eigen::Dynamic, 3, Eigen::RowMajor>> toMatrixX3f(std::vector<Vec3>& data)
{
    return Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, 3, Eigen::RowMajor>, Eigen::Aligned, Eigen::Stride<sizeof(Vec3) / sizeof(float), 1>>(
        (float*)data.data(), data.size(), 3
    );
}

// MARK: const Vec3 adaptors

const Eigen::Ref<const Eigen::Matrix<float, 3, Eigen::Dynamic>> toMatrix3Xf(const std::vector<Vec3>& data)
{
    return Eigen::Map<const Eigen::Matrix<float, 3, Eigen::Dynamic>, Eigen::Aligned, Eigen::Stride<sizeof(Vec3) / sizeof(float), 1>>(
        (float*)data.data(), 3, data.size()
    );
}

const Eigen::Ref<const Eigen::Matrix<float, Eigen::Dynamic, 3, Eigen::RowMajor>> toMatrixX3f(const std::vector<Vec3>& data)
{
    return Eigen::Map<const Eigen::Matrix<float, Eigen::Dynamic, 3, Eigen::RowMajor>, Eigen::Aligned, Eigen::Stride<sizeof(Vec3) / sizeof(float), 1>>(
        (float*)data.data(), data.size(), 3
    );
}

// MARK: non-const Face3 adaptors

Eigen::Ref<Eigen::Matrix<int, 3, Eigen::Dynamic>> toMatrix3Xi(std::vector<Face3>& data)
{
    return Eigen::Map<Eigen::Matrix<int, 3, Eigen::Dynamic>, Eigen::Aligned, Eigen::Stride<sizeof(Face3) / sizeof(int), 1>>(
        (int*)data.data(), 3, data.size()
    );
}

Eigen::Ref<Eigen::Matrix<int, Eigen::Dynamic, 3, Eigen::RowMajor>> toMatrixX3i(std::vector<Face3>& data)
{
    return Eigen::Map<Eigen::Matrix<int, Eigen::Dynamic, 3, Eigen::RowMajor>, Eigen::Aligned, Eigen::Stride<sizeof(Face3) / sizeof(int), 1>>(
        (int*)data.data(), data.size(), 3
    );
}

// MARK: const Face3 adaptors

const Eigen::Ref<const Eigen::Matrix<int, 3, Eigen::Dynamic>> toMatrix3Xi(const std::vector<Face3>& data)
{
    return Eigen::Map<const Eigen::Matrix<int, 3, Eigen::Dynamic>, Eigen::Aligned, Eigen::Stride<sizeof(Face3) / sizeof(int), 1>>(
        (int*)data.data(), 3, data.size()
    );
}

const Eigen::Ref<const Eigen::Matrix<int, Eigen::Dynamic, 3, Eigen::RowMajor>> toMatrixX3i(const std::vector<Face3>& data)
{
    return Eigen::Map<const Eigen::Matrix<int, Eigen::Dynamic, 3, Eigen::RowMajor>, Eigen::Aligned, Eigen::Stride<sizeof(Face3) / sizeof(int), 1>>(
        (int*)data.data(), data.size(), 3
    );
}

Eigen::Vector3f toVector3f(Vec3 vec3)
{
    Eigen::Vector3f v;
    v[0] = vec3.x;
    v[1] = vec3.y;
    v[2] = vec3.z;
    return v;
}

Eigen::Matrix3f toMatrix3f(Mat3x3 matrix)
{
    Eigen::Matrix3f m;
    m << matrix.m00, matrix.m01, matrix.m02,
         matrix.m10, matrix.m11, matrix.m12,
         matrix.m20, matrix.m21, matrix.m22;
    return m;
}

Eigen::Matrix4f toMatrix4f(Mat3x4 matrix)
{
    Eigen::Matrix4f m;
    m << matrix.m00, matrix.m01, matrix.m02, matrix.m03,
         matrix.m10, matrix.m11, matrix.m12, matrix.m13,
         matrix.m20, matrix.m21, matrix.m22, matrix.m23,
         0.0, 0.0, 0.0, 1.0;
    return m;
}

Eigen::Matrix4f toMatrix4f(Mat4x4 matrix)
{
    Eigen::Matrix4f m;
    m << matrix.m00, matrix.m01, matrix.m02, matrix.m03,
         matrix.m10, matrix.m11, matrix.m12, matrix.m13,
         matrix.m20, matrix.m21, matrix.m22, matrix.m23,
         matrix.m30, matrix.m31, matrix.m32, matrix.m33;
    return m;
}

Vec3 toVec3(Eigen::Vector3f vector) {
    return Vec3 {vector[0], vector[1], vector[2]};
}

Mat3x3 toMat3x3(Eigen::Matrix3f matrix) {
    return Mat3x3 {
        matrix(0, 0), matrix(0, 1), matrix(0, 2),
        matrix(1, 0), matrix(1, 1), matrix(1, 2),
        matrix(2, 0), matrix(2, 1), matrix(2, 2)
    };
}

Mat3x4 toMat3x4(Eigen::Matrix4f matrix) {
    return Mat3x4 {
        matrix(0, 0), matrix(0, 1), matrix(0, 2), matrix(0, 3),
        matrix(1, 0), matrix(1, 1), matrix(1, 2), matrix(1, 3),
        matrix(2, 0), matrix(2, 1), matrix(2, 2), matrix(2, 3)
    };
}

Mat4x4 toMat4x4(Eigen::Matrix4f matrix) {
    return Mat4x4 {
        matrix(0, 0), matrix(0, 1), matrix(0, 2), matrix(0, 3),
        matrix(1, 0), matrix(1, 1), matrix(1, 2), matrix(1, 3),
        matrix(2, 0), matrix(2, 1), matrix(2, 2), matrix(2, 3),
        matrix(3, 0), matrix(3, 1), matrix(3, 2), matrix(3, 3)
    };
}

Eigen::Matrix4f columnMajorToMatrix4f(const std::vector<float>& v) {
    Eigen::Matrix4f m;
    m << v[0], v[4], v[8], v[12],
         v[1], v[5], v[9], v[13],
         v[2], v[6], v[10], v[14],
         v[3], v[7], v[11], v[15];
    return m;
}

Eigen::Matrix3f columnMajorToMatrix3f(const std::vector<float>& v) {
    Eigen::Matrix3f m;
    m << v[0], v[3], v[6],
         v[1], v[4], v[7],
         v[2], v[5], v[8];
    return m;
}

} // namespace standard_cyborg
