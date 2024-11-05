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


#include <gtest/gtest.h>


#include "standard_cyborg/algorithms/SparseICPWrapper.hpp"


#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/math/Mat3x4.hpp"
#include "standard_cyborg/math/Vec3.hpp"

using standard_cyborg::math::Mat3x4;
using standard_cyborg::math::Vec3;
using standard_cyborg::sc3d::Geometry;

TEST(SparseICP, testSparseICP) {
    
    std::vector<Vec3> positions0 {
        {1.0f, 1.0f, 0.0f},
        {4.0f, 0.0f, 0.0f},
        {1.0f, 4.0f, 0.0f},
    };
    
    std::vector<Vec3> normals {
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
    };
    
    std::vector<Vec3> positions1 {
        {1.0f, 1.0f, 0.01f},
        {4.0f, 0.0f, 0.01f},
        {1.0f, 4.0f, 0.01f},
    };
    
    Geometry geo0 (positions0, normals);
    Geometry geo1 (positions1, normals);
    
    standard_cyborg::algorithms::SparseICPParameters pars;
    
    pars.p = 0.2;
    pars.max_icp = 15;
    pars.stop = 2e-3; // 2e-3
    pars.max_outer = 100; // 50
    pars.max_inner = 1;
    pars.print_icpn = true;
    
    Mat3x4 m = standard_cyborg::algorithms::SparseICPPointToPlane(geo0, geo1, pars);
    
    EXPECT_TRUE(Mat3x4::almostEqual(m, Mat3x4({
        1, 0, 0, +0.0,
        0, 1, 0, +0.0,
        0, 0, 1, +0.01
    }), 1.0e-6, 1.0e-6));
}
