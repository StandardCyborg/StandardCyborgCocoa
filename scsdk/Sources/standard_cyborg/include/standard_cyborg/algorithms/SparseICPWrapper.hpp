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

namespace standard_cyborg {

namespace sc3d {
class Geometry;
}

namespace math {
struct Mat3x4;
}

namespace algorithms {
    
struct SparseICPParameters {
    bool use_penalty = false; /// if use_penalty then penalty method else ADMM or ALM (see max_inner)
    double p = 1.0;           /// p norm
    double mu = 10.0;         /// penalty weight
    double alpha = 1.2;       /// penalty increase factor
    double max_mu = 1e5;      /// max penalty
    int max_icp = 100;        /// max ICP iteration
    int max_outer = 100;      /// max outer iteration
    int max_inner = 1;        /// max inner iteration. If max_inner=1 then ADMM else ALM
    double stop = 1e-5;       /// stopping criteria
    bool print_icpn = false;  /// (debug) print ICP iteration
    double outlierDeviationsThreshold = 1.0f; // The number of standard deviations outside of which a depth value being aligned is coinsidered an outlier
};
    
standard_cyborg::math::Mat3x4 SparseICPPointToPlane(const standard_cyborg::sc3d::Geometry& source, const standard_cyborg::sc3d::Geometry& target, const SparseICPParameters& pars);
    
}
}
