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
#include "standard_cyborg/algorithms/PointToPointAlignment.hpp"

#include "standard_cyborg/util/IncludeEigen.hpp"

#include "standard_cyborg/math/Vec3.hpp"
#include "standard_cyborg/math/Mat3x4.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <Eigen/Jacobi>
#pragma clang diagnostic pop

namespace standard_cyborg {

namespace algorithms {

standard_cyborg::math::Mat3x4 PointToPointAlignment(const std::vector<standard_cyborg::math::Vec3>& sourcePositions, const std::vector<standard_cyborg::math::Vec3>& targetPositions)
{
    int N = (int)sourcePositions.size();
    
    Eigen::Matrix3Xf p = Eigen::Matrix3Xf(3, N);
    Eigen::Matrix3Xf q = Eigen::Matrix3Xf(3, N);
    Eigen::MatrixXf avgP, avgQ, pNorm, qNorm, C, R, t;
    
    for (int iv = 0; iv < N; ++iv) {
        p.col(iv) = Eigen::Vector3f(sourcePositions[iv].x, sourcePositions[iv].y, sourcePositions[iv].z);
        q.col(iv) = Eigen::Vector3f(targetPositions[iv].x, targetPositions[iv].y, targetPositions[iv].z);
    }
    
    avgP = p.rowwise().mean();
    avgQ = q.rowwise().mean();
    
    pNorm = p - avgP.replicate(1, p.cols());
    qNorm = q - avgQ.replicate(1, q.cols());
    
    C = pNorm * qNorm.transpose();
    Eigen::JacobiSVD<Eigen::MatrixXf> svd(C, Eigen::ComputeThinU | Eigen::ComputeThinV);
    
    // rotation
    R = svd.matrixV() * svd.matrixU().transpose();
    
    // translation
    t = avgQ - R * avgP;
    
    standard_cyborg::math::Mat3x4 r = {
        R(0, 0), R(0, 1), R(0, 2), t(0),
        R(1, 0), R(1, 1), R(1, 2), t(1),
        R(2, 0), R(2, 1), R(2, 2), t(2),
    };
    
    return r;
}

}

}
