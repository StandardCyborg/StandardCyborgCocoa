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
 
 #include <StandardCyborgData/Vec3.hpp>
 #include <StandardCyborgData/Mat3x3.hpp>
 
 #import "../StandardCyborgAlgorithms/CreateVectorRotationMatrix.hpp"
 
 namespace math = StandardCyborg::math;
 using math::Vec3;
 
 
 @interface CreateVectorRotationMatrixTests : XCTestCase
 
 @end
 
 @implementation CreateVectorRotationMatrixTests : XCTestCase
 */

#include <gtest/gtest.h>

#include "standard_cyborg/algorithms/CreateVectorRotationMatrix.hpp"
#include "standard_cyborg/math/Vec3.hpp"
#include "standard_cyborg/math/Mat3x3.hpp"

namespace math = standard_cyborg::math;
using math::Vec3;


constexpr float EPS = 1E-6;

TEST(CreateVectorRotationMatrixTests, testCreateVectorRotationMatrix) {
    Vec3 v0(-0.215666, 0.107833, -0.970495);
    Vec3 v1(0.224105, -0.0194874, -0.97437);
    
    math::Mat3x3 mat = standard_cyborg::algorithms::createVectorRotationMatrix(v0, v1);
    
    Vec3 result = mat * v0;
    
    EXPECT_NEAR(result.x, v1.x, EPS);
    EXPECT_NEAR(result.y, v1.y, EPS);
    EXPECT_NEAR(result.z, v1.z, EPS);
}

TEST(CreateVectorRotationMatrixTests, testCreateVectorRotationMatrixParallel) {
    Vec3 v0(-0.215666, 0.107833, -0.970495);
    Vec3 v1 = v0;
    
    math::Mat3x3 mat = standard_cyborg::algorithms::createVectorRotationMatrix(v0, v1);
    
    Vec3 result = mat * v0;
    
    EXPECT_NEAR(result.x, v1.x, EPS);
    EXPECT_NEAR(result.y, v1.y, EPS);
    EXPECT_NEAR(result.z, v1.z, EPS);
}

TEST(CreateVectorRotationMatrixTests, testCreateVectorRotationMatrixClose) {
    Vec3 v0 = Vec3::normalize(Vec3(12424002.0f, 3.0f, 100.0f));
    Vec3 v1 = Vec3::normalize(Vec3(12424003.0f, 3.0f, 100.0f));
    
    math::Mat3x3 mat = standard_cyborg::algorithms::createVectorRotationMatrix(v0, v1);
    
    Vec3 result = mat * v0;
    
    EXPECT_NEAR(result.x, v1.x, EPS);
    EXPECT_NEAR(result.y, v1.y, EPS);
    EXPECT_NEAR(result.z, v1.z, EPS);
}
