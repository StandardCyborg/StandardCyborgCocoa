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

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>

/*
 #import <XCTest/XCTest.h>
 
 #import <StandardCyborgData/DataUtils.hpp>
 #import <StandardCyborgData/DebugHelpers.hpp>
 #import <StandardCyborgData/Vec3.hpp>
 
 #import "../StandardCyborgAlgorithms/DBScan.hpp"
 
 namespace math = StandardCyborg::math;
 using math::Vec3;
 
 @interface DBScanTests : XCTestCase
 
 @end
 
 
 @implementation DBScanTests
 */

#include <gtest/gtest.h>

#include "standard_cyborg/algorithms/DBScan.hpp"
#include "standard_cyborg/math/Vec3.hpp"

//#include "standard_cyborg/sc3d/VertexSelection.hpp"

//#include "standard_cyborg/util/DebugHelpers.hpp"

namespace math = standard_cyborg::math;

using math::Vec3;

float randFloat() {
    return (float)rand() / (float)RAND_MAX;
}

float randFloat(float fmin, float fmax) {
    return fmin + (fmax - fmin) * randFloat();
}

TEST(DBScanTests, testDBscan) {
    // Create two well-separated clusters
    Vec3 point1 {1.0f, 2.0f, 4.0f};
    Vec3 point2 {-7.0f, -3.0f, 1.0};
    
    srand(0);
    
    const int n = 50;
    const float a = 0.1f;
    std::vector<Vec3> positions {};
    
    // create one cluster of points.
    for (int i = 0; i < n; i++) {
        positions.push_back(point1 +
                            Vec3({ a * randFloat(-1.0, +1.0), a * randFloat(-1.0, +1.0), a * randFloat(-1.0, +1.0) })
                            );
    }
    
    // create another cluster of points.
    for (int i = 0; i < n; i++) {
        positions.push_back(point2 +
                            Vec3({ a * randFloat(-1.0, +1.0), a * randFloat(-1.0, +1.0), a * randFloat(-1.0, +1.0) })
                            );
    }
    
    // some noise points.
    positions.push_back(Vec3({  100.0f,  100.0f,  100.0f, }));
    positions.push_back(Vec3({ -100.0f, -100.0f, -100.0f, }));
    positions.push_back(Vec3({ -300.0f, -300.0f, -300.0f, }));
    
    /*
     10 is minPts.
     0.2 is epsilon.
     see:
     https://en.wikipedia.org/wiki/DBSCAN
     for explanation of parameters.
     */
    auto result = standard_cyborg::algorithms::DBScan::compute(positions, 10, 0.2);
    
    EXPECT_EQ(result.size(), positions.size());
    
    // first 50 are cluster #0
    for (int ii = 0; ii < 50; ++ii) {
        EXPECT_EQ(result[ii], 0);
    }
    
    // second 50 are cluster #1
    for (int ii = 50; ii < 100; ++ii) {
        EXPECT_EQ(result[ii], 1);
    }
    
    // finally, some noise points.
    for (int ii = 100; ii < result.size(); ++ii) {
        EXPECT_EQ(result[ii], -1);
    }
}
