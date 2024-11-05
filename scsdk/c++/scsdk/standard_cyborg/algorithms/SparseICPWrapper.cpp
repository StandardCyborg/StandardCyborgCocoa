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

#include "standard_cyborg/algorithms/SparseICPWrapper.hpp"

#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/math/Vec3.hpp"
#include "standard_cyborg/math/Mat3x4.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#include <SparseICP.h>
#pragma clang diagnostic pop

using standard_cyborg::math::Mat3x4;
using standard_cyborg::math::Vec3;

using standard_cyborg::sc3d::Geometry;

namespace standard_cyborg {
    
namespace algorithms {


// 3 rows.
typedef Eigen::Matrix<double, 3, Eigen::Dynamic> Vertices;

Vertices toEigen(const std::vector<Vec3> vs)
{
    Vertices vertices;
    
    vertices.resize(Eigen::NoChange, vs.size());
    
    for (int ii = 0; ii < vs.size(); ++ii) {
        vertices(0, ii) = vs[ii].x;
        vertices(1, ii) = vs[ii].y;
        vertices(2, ii) = vs[ii].z;
    }
    
    return vertices;
}

Mat3x4 SparseICPPointToPlane(const Geometry& source, const Geometry& target, const SparseICPParameters& pars)
{
    Vertices sourceVertices = toEigen(source.getPositions());
    Vertices targetVertices = toEigen(target.getPositions());
    Vertices targetNormals = toEigen(target.getNormals());
    
    SICP::Parameters pars_;
    
    pars_.use_penalty = pars.use_penalty;
    pars_.p = pars.p;
    pars_.mu = pars.mu;
    pars_.alpha = pars.alpha;
    pars_.max_mu = pars.max_mu;
    pars_.max_icp = pars.max_icp;
    pars_.max_outer = pars.max_outer;
    pars_.max_inner = pars.max_inner;
    pars_.stop = pars.stop;
    pars_.print_icpn = pars.print_icpn;
    pars_.outlierDeviationsThreshold = pars.outlierDeviationsThreshold;
    
    Eigen::Affine3d t = SICP::point_to_plane(sourceVertices, targetVertices, targetNormals, pars_);
    
    Mat3x4 m{
        (float)t(0, 0), (float)t(0, 1), (float)t(0, 2), (float)t(0, 3),
        (float)t(1, 0), (float)t(1, 1), (float)t(1, 2), (float)t(1, 3),
        (float)t(2, 0), (float)t(2, 1), (float)t(2, 2), (float)t(2, 3)
    };
    
    return m;
}
    
}

}
