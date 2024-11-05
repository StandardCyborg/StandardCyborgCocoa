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
#include "standard_cyborg/math/Transform.hpp"

#include "standard_cyborg/math/Mat3x4.hpp"
#include "standard_cyborg/math/Mat3x3.hpp"
#include "standard_cyborg/util/DataUtils.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#ifndef DEBUG
#define EIGEN_NO_DEBUG  1
#endif
#include <Eigen/Core>
#include <Eigen/Cholesky>
#pragma clang diagnostic pop

namespace standard_cyborg {
namespace math {

// This function is derived from the transforms3d python package and specialized for 3x3 matrices.
// You may find the original source of this function at:
//     https://github.com/matthew-brett/transforms3d/blob/8f81b063686f9b892bdbd4775d02615dce028105/transforms3d/affines.py#L156-L246
//
// **********************
// Copyright and Licenses
// **********************
//
// Retrieved from https://github.com/matthew-brett/transforms3d/blob/master/LICENSE on Dec. 2, 2019
//
// Transforms3d
// ============
//
// The transforms3d package, including all examples, code snippets and attached
// documentation is covered by the 2-clause BSD license.
//
//    Copyright (c) 2009-2017, Matthew Brett and Christoph Gohlke
//    All rights reserved.
//
//    Redistribution and use in source and binary forms, with or without
//    modification, are permitted provided that the following conditions are
//    met:
//
//    1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//
//    2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
//    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//    PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
//    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// 3rd party code and data
// =======================
//
// Some code distributed within the transforms3d sources was developed by other
// projects. This code is distributed under its respective licenses that are
// listed below.
//
// Sphinx autosummary extension
// ----------------------------
//
// This extension has been copied from NumPy (Jul 16, 2010) as the one shipped with
// Sphinx 0.6 doesn't work properly.
//
// ::
//
//  Copyright (c) 2007-2009 Stefan van der Walt and Sphinx team
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//    a. Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimer.
//    b. Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//    c. Neither the name of the Enthought nor the names of its contributors
//       may be used to endorse or promote products derived from this software
//       without specific prior written permission.
//
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
//  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
//  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
//  DAMAGE.

typedef Eigen::Matrix<float, 3, 3> EMat3;
typedef Eigen::Vector3f EVec3;

Transform Transform::fromMat3x4(const math::Mat3x4& A) {
    Transform transform;

    // Start by extracting the translation. This part is very easy.
    transform.translation = {A.m03, A.m13, A.m23};
    
    // R = rotation
    // Z = zoom (scale)
    // S = shear
    EMat3 RZS;
    RZS << A.m00, A.m01, A.m02,
           A.m10, A.m11, A.m12,
           A.m20, A.m21, A.m22;

    // Use the Cholesky decomposition to perform the separation
    // This part very closely mirrors the original implementation, only specialized for 3x3 matrices
    EMat3 RZSt_RZS (RZS.transpose() * RZS);
    Eigen::LLT<EMat3> cholesky;
    cholesky.compute(RZSt_RZS);
    EMat3 ZS (cholesky.matrixL().transpose());
    EVec3 Z (ZS.diagonal());
    EMat3 R (RZS * ZS.inverse());
    
    // Detect and fix if the rotation flips parity
    if (R.determinant() < 0.0f) {
        Z(0) = -Z(0);
        ZS.row(0) = -ZS.row(0);
        R = RZS * ZS.inverse();
    }

    // Extract the result into StandardCyborg data types
    transform.scale = toVec3(Z);
    
    transform.shear = math::Vec3{
        ZS(0, 1) / Z(0),
        ZS(0, 2) / Z(0),
        ZS(1, 2) / Z(1)
    };
    
    transform.rotation = math::Quaternion::fromMat3x3(toMat3x3(R));
    
    return transform;
}

Transform Transform::fromMat3x4(const math::Mat3x4& A, std::string srcFrame, std::string destFrame) {
  Transform t (Transform::fromMat3x4(A));
  t.srcFrame = srcFrame;
  t.destFrame = destFrame;
  return t;
}

Transform Transform::inverse() const {
  Mat3x4 m (Mat3x4::fromTransform(*this));
  return Transform::fromMat3x4(m.invert(), destFrame, srcFrame);
}


} // namespace math
} // namespace standard_cyborg
