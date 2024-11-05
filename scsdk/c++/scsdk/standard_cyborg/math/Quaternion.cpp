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

#include "standard_cyborg/math/Quaternion.hpp"
#include "standard_cyborg/math/Mat3x3.hpp"
#include <cmath>
#include <array>

namespace standard_cyborg {
namespace math {

// The  below have been adapted from https://github.com/stackgl/gl-quat
//
// fromMat3x3: https://github.com/stackgl/gl-quat/blob/6d3b1574dd51b78a24dd504f2ab920d3945b0b4b/fromMat3.js
// slerp: https://github.com/stackgl/gl-quat/blob/master/slerp.js
//
// Copyright (c) 2013 Brandon Jones, Colin MacKenzie IV
//
// This software is provided 'as-is', without any express or implied warranty. In no event
// will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including commercial
// applications, and to alter it and redistribute it freely, subject to the following restrictions:
//
// The origin of this software must not be misrepresented you must not claim that you wrote the
// original software. If you use this software in a product, an acknowledgment in the product
// documentation would be appreciated but is not required.
//
// Altered source versions must be plainly marked as such, and must not be misrepresented as
// being the original software.
//
// This notice may not be removed or altered from any source distribution.

Quaternion Quaternion::fromMat3x3(const Mat3x3 matrix) {
    // Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
    // article "Quaternion Calculus and Fast Animation".
    float mTrace (matrix.trace());
    float fRoot;

    // The non-positive-trace branch of this implementation does a moderate amount of index
    // arithmetic. We could get fancier with casting or unrolling, but we'll copy the input
    // output so that we can simply use array indexing before prmaturely optimizing.
    std::array<float, 4> out;
    
    // Despite their best efforts to claim indifference, OpenGL matrices use *column* major
    // ordering if you left-multiply transformations. The implementation below is designed
    // based on opengl conventions, so we pack the mat3x3 into
    std::array<float, 9> m {
        matrix.m00, matrix.m10, matrix.m20,
        matrix.m01, matrix.m11, matrix.m21,
        matrix.m02, matrix.m12, matrix.m22
    };

    if (mTrace > 0.0f) {
        // |w| > 1/2, may as well choose w > 1/2
        fRoot = std::sqrt(mTrace + 1.0f);  // 2w
        out[3] = 0.5 * fRoot;
        fRoot = 0.5 / fRoot;  // 1/(4w)
        out[0] = (m[5] - m[7]) * fRoot;
        out[1] = (m[6] - m[2]) * fRoot;
        out[2] = (m[1] - m[3]) * fRoot;
    } else {
        // |w| <= 1/2
        int i = 0;
        if (m[4] > m[0]) {
            i = 1;
        }
        if (m[8] > m[i * 3 + i]) {
            i = 2;
        }
        int j = (i + 1) % 3;
        int k = (i + 2) % 3;
        
        fRoot = std::sqrt(m[i * 3 + i] - m[j * 3 + j] - m[k * 3 + k] + 1.0f);
        out[i] = 0.5f * fRoot;
        fRoot = 0.5f / fRoot;
        out[3] = (m[j * 3 + k] - m[k * 3 + j]) * fRoot;
        out[j] = (m[j * 3 + i] + m[i * 3 + j]) * fRoot;
        out[k] = (m[k * 3 + i] + m[i * 3 + k]) * fRoot;
    }
    
    return Quaternion { out[0], out[1], out[2], out[3] };
}

Quaternion Quaternion::slerp(Quaternion a, Quaternion b, float t) {
    // benchmarks:
    //    http://jsperf.com/quaternion-slerp-implementations
    
    float ax = a.x;
    float ay = a.y;
    float az = a.z;
    float aw = a.w;
    
    float bx = b.x;
    float by = b.y;
    float bz = b.z;
    float bw = b.w;
    
    float omega, cosom, sinom, scale0, scale1;
    
    // calc cosine
    cosom = ax * bx + ay * by + az * bz + aw * bw;
    
    // adjust signs (if necessary)
    if (cosom < 0.0f) {
        cosom = -cosom;
        bx = -bx;
        by = -by;
        bz = -bz;
        bw = -bw;
    }
    
    // calculate coefficients
    if ((1.0f - cosom) > 0.000001f) {
        // standard case (slerp)
        omega = std::acos(cosom);
        sinom = std::sin(omega);
        scale0 = std::sin((1.0f - t) * omega) / sinom;
        scale1 = std::sin(t * omega) / sinom;
    } else {
        // "from" and "to" quaternions are very close
        //  ... so we can do a linear interpolation
        scale0 = 1.0f - t;
        scale1 = t;
    }
    // calculate final values
    return Quaternion {
        scale0 * ax + scale1 * bx,
        scale0 * ay + scale1 * by,
        scale0 * az + scale1 * bz,
        scale0 * aw + scale1 * bw
    };
}

Vec3 operator*(Quaternion q, Vec3 v) {
    // calculate quat * vec
    float ix = q.w * v.x + q.y * v.z - q.z * v.y;
    float iy = q.w * v.y + q.z * v.x - q.x * v.z;
    float iz = q.w * v.z + q.x * v.y - q.y * v.x;
    float iw = -q.x * v.x - q.y * v.y - q.z * v.z;

    // calculate result * inverse quat
    return {
        ix * q.w + iw * -q.x + iy * -q.z - iz * -q.y,
        iy * q.w + iw * -q.y + iz * -q.x - ix * -q.z,
        iz * q.w + iw * -q.z + ix * -q.y - iy * -q.x
    };
}

Quaternion& Quaternion::rotateX(float radians) {
    radians *= 0.5f;

    float ax = x;
    float ay = y;
    float az = z;
    float aw = w;
    float bx = std::sin(radians);
    float bw = std::cos(radians);

    x = ax * bw + aw * bx;
    y = ay * bw + az * bx;
    z = az * bw - ay * bx;
    w = aw * bw - ax * bx;
    
    return *this;
}

Quaternion& Quaternion::rotateY(float radians) {
    radians *= 0.5f;
    
    float ax = x;
    float ay = y;
    float az = z;
    float aw = w;
    float by = std::sin(radians);
    float bw = std::cos(radians);
    
    x = ax * bw - az * by;
    y = ay * bw + aw * by;
    z = az * bw + ax * by;
    w = aw * bw - ay * by;
    
    return *this;
}

Quaternion& Quaternion::rotateZ(float radians) {
    radians *= 0.5;

    float ax = x;
    float ay = y;
    float az = z;
    float aw = w;
    float bz = std::sin(radians);
    float bw = std::cos(radians);

    x = ax * bw + ay * bz;
    y = ay * bw - ax * bz;
    z = az * bw + aw * bz;
    w = aw * bw - az * bz;
    
    return *this;
}

Quaternion Quaternion::fromRotationX(float radians) {
    radians *= 0.5f;
    return Quaternion {
         std::sin(radians),
         0.0f,
         0.0f,
         std::cos(radians)
    };
}

Quaternion Quaternion::fromRotationY(float radians) {
    radians *= 0.5f;
    return Quaternion {
        0.0f,
        std::sin(radians),
        0.0f,
        std::cos(radians)
    };
}

Quaternion Quaternion::fromRotationZ(float radians) {
    radians *= 0.5;
    return Quaternion {
        0.0f,
        0.0f,
        std::sin(radians),
        std::cos(radians)
    };
}

} // namespace math
} // namespace standard_cyborg
