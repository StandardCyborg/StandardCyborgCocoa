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


/*
 #import <XCTest/XCTest.h>
 
 #import "../StandardCyborgAlgorithms/PointToPointAlignment.hpp"
 
 #import <StandardCyborgData/Vec3.hpp>
 #import <StandardCyborgData/Mat3x4.hpp>
 
 namespace math = StandardCyborg::math;
 
 @interface PointToPointAlignmentTests : XCTestCase
 
 @end
 
 @implementation PointToPointAlignmentTests  : XCTestCase
 */

#include <gtest/gtest.h>

#include "standard_cyborg/algorithms/PointToPointAlignment.hpp"

#include "standard_cyborg/math/Vec3.hpp"
#include "standard_cyborg/math/Mat3x4.hpp"



namespace math = standard_cyborg::math;

TEST(PlaneEstimationTests, testPointToPointAlignment) {
    
    std::vector<math::Vec3> positions0 {
        {1.0f, 1.0f, 0.0f},
        {4.0f, 0.0f, 0.0f},
        {1.0f, 4.0f, 0.0f},
    };
    
    std::vector<math::Vec3> positions1 {
        {1.0f, 1.0f, 0.01f},
        {4.0f, 0.0f, 0.01f},
        {1.0f, 4.0f, 0.01f},
    };
    
    math::Mat3x4 m = standard_cyborg::algorithms::PointToPointAlignment(positions0, positions1);
    
    EXPECT_TRUE(math::Mat3x4::almostEqual(m, math::Mat3x4({
        1, 0, 0, +0.0,
        0, 1, 0, +0.0,
        0, 0, 1, +0.01
    }), 1.0e-6, 1.0e-6));
}
