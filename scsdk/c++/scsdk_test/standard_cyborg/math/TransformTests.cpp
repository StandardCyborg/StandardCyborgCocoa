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

#include "standard_cyborg/math/Transform.hpp"
#include "standard_cyborg/math/Mat3x4.hpp"
#include "standard_cyborg/math/Mat3x3.hpp"

using standard_cyborg::math::Transform;
namespace math = standard_cyborg::math;
using math::Vec3;

TEST(TransformTests, testInitialization) {
    Transform t;
    EXPECT_TRUE(Vec3::almostEqual(t.translation, {0, 0, 0}));
    EXPECT_TRUE(Vec3::almostEqual(t.scale, {1, 1, 1}));
    EXPECT_TRUE(Vec3::almostEqual(t.shear, {0, 0, 0}));
    EXPECT_TRUE(math::Quaternion::almostEqual(t.rotation, math::Quaternion::Identity()));
}

TEST(TransformTests, testDecomposition) {
    Transform t (Transform::fromMat3x4({
        0.0f,  -3.0f,   0.0f,  20.0f,
        2.0f,   1.0f,   0.0f,  30.0f,
        0.0f,   0.0f,   4.0f,  40.0f
    }));
    
    EXPECT_TRUE(Vec3::almostEqual(t.translation, {20, 30, 40}));
    EXPECT_TRUE(Vec3::almostEqual(t.scale, {2, 3, 4}));
    EXPECT_TRUE(Vec3::almostEqual(t.shear, {0.5, 0, 0}));
    EXPECT_TRUE(math::Quaternion::almostEqual(t.rotation, {0, 0, 0.70710678118f, 0.70710678118f}));
    
    // This is a redundant assertion, but it's much easier to reason through and makes agreement
    // with the original python implementation more clear
    EXPECT_TRUE(math::Mat3x3::almostEqual(math::Mat3x3::fromQuaternion(t.rotation), {
        0.0f, -1.0f,  0.0f,
        1.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  1.0f
    }));
}

TEST(TransformTests, testGetMat3x4) {
    Transform t;
    t.translation = {20, 30, 40};
    t.scale = {2, 3, 4};
    t.shear = {0.5, 0, 0};
    t.rotation = (math::Quaternion{0, 0, 1, 1}).normalize();
    
    math::Mat3x4 matrix (math::Mat3x4::fromTransform(t));
    
    EXPECT_TRUE(math::Mat3x4::almostEqual(matrix, {
        0, -3, 0, 20,
        2, 1, 0, 30,
        0, 0, 4, 40
    }));
}
