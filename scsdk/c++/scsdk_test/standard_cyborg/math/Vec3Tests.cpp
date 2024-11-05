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

#include "standard_cyborg/math/Vec2.hpp"
#include "standard_cyborg/math/Vec3.hpp"

namespace math = standard_cyborg::math;
using math::Vec3;

TEST(Vec3Tests, testConstructionFromParts) {
    using math::Vec2;
    
    EXPECT_EQ(Vec3(Vec2(1, 2), 3), Vec3(1, 2, 3));
    EXPECT_EQ(Vec3(1, Vec2(2, 3)), Vec3(1, 2, 3));
}

TEST(Vec3Tests, testXYAsVec2) {
    using math::Vec2;
    
    EXPECT_EQ(Vec3(1, 2, 3).xy(), Vec2(1, 2));
}

TEST(Vec3Tests, testDotProduct) {
    EXPECT_EQ(Vec3::dot(Vec3(1, 2, 3), Vec3(4, 5, 6)), 4 + 10 + 18);
}

TEST(Vec3Tests, testCrossProduct) {
    EXPECT_EQ(Vec3::cross(Vec3(1, 2, 3), Vec3(4, 5, 6)), Vec3(-3, 6, -3));
}

TEST(Vec3Tests, testSquaredNorm) {
    EXPECT_EQ(Vec3(1, 2, 3).squaredNorm(), 14);
}

TEST(Vec3Tests, testNorm) {
    EXPECT_NEAR(Vec3(8, 9, 12).norm(), 17.0f, 1e-8f);
}

TEST(Vec3Tests, testNormalizedByValue) {
    Vec3 x (8, 9, 12);
    EXPECT_EQ(Vec3::normalize(x), Vec3(8, 9, 12) / 17.0f);
    EXPECT_EQ(x, Vec3(8, 9, 12));
}

TEST(Vec3Tests, testLinearInterpolation) {
    Vec3 a (1.0f, 2.0f, 3.0f);
    Vec3 b (2.0f, 4.0f, 6.0f);
    EXPECT_EQ(Vec3::lerp(a, b, 0.25), Vec3(1.25f, 2.5f, 3.75f));
}

TEST(Vec3Tests, testPerComponentMin) {
    Vec3 a (1.0f, 2.0f, 3.0f);
    Vec3 b (2.0f, 1.0f, 4.0f);
    EXPECT_EQ(Vec3::min(a, b), Vec3(1.0f, 1.0f, 3.0f));
}

TEST(Vec3Tests, testPerComponentMax) {
    Vec3 a (1.0f, 2.0f, 3.0f);
    Vec3 b (2.0f, 1.0f, 4.0f);
    EXPECT_EQ(Vec3::max(a, b), Vec3(2.0f, 2.0f, 4.0f));
}

TEST(Vec3Tests, testNormalize) {
    Vec3 x (8, 9, 12);
    Vec3 expected (8.0f / 17.0f, 9.0 / 17.0f, 12.0 / 17.0f);
    Vec3 returnValue = x.normalize();
    
    // Ensure return value is correct
    EXPECT_NEAR(returnValue.x, expected.x, 1e-8);
    EXPECT_NEAR(returnValue.y, expected.y, 1e-8);
    EXPECT_NEAR(returnValue.z, expected.z, 1e-8);
    
    // Ensure x is mutated
    EXPECT_NEAR(x.x, expected.x, 1e-8);
    EXPECT_NEAR(x.y, expected.y, 1e-8);
    EXPECT_NEAR(x.z, expected.z, 1e-8);
}

TEST(Vec3Tests, testNormalizeZeroVector) {
    Vec3 v (Vec3(0.0f, 0.0f, 0.0f).normalize());
    EXPECT_TRUE(std::isnan(v.x));
    EXPECT_TRUE(std::isnan(v.y));
    EXPECT_TRUE(std::isnan(v.z));
}

TEST(Vec3Tests, testNormalizedZeroVector) {
    Vec3 v (Vec3::normalize(Vec3(0.0f, 0.0f, 0.0f)));
    EXPECT_TRUE(std::isnan(v.x));
    EXPECT_TRUE(std::isnan(v.y));
    EXPECT_TRUE(std::isnan(v.z));
}

TEST(Vec3Tests, testAngleBetween) {
    EXPECT_EQ(Vec3::angleBetween(Vec3(2, 0, 0), Vec3(3, 0, 0)), 0.0f);
    EXPECT_NEAR(Vec3::angleBetween(Vec3(2, 0, 0), Vec3(0, 2, 0)), M_PI_2, 1e-6);
    EXPECT_NEAR(Vec3::angleBetween(Vec3(1, 1, 1), Vec3(-1, -1, -1)), M_PI, 1e-6);
    
    EXPECT_TRUE(std::isnan(Vec3::angleBetween(Vec3(0, 0, 0), Vec3(1, 0, 0))));
    EXPECT_TRUE(std::isnan(Vec3::angleBetween(Vec3(1, 0, 0), Vec3(0, 0, 0))));
}

TEST(Vec3Tests, testEquality) {
    EXPECT_TRUE(Vec3(1, 2, 3) == Vec3(1, 2, 3));
    EXPECT_FALSE(Vec3(0, 2, 3) == Vec3(1, 2, 3));
    EXPECT_FALSE(Vec3(1, 0, 3) == Vec3(1, 2, 3));
    EXPECT_FALSE(Vec3(1, 2, 0) == Vec3(1, 2, 3));
}

TEST(Vec3Tests, testInequality) {
    EXPECT_FALSE(Vec3(1, 2, 3) != Vec3(1, 2, 3));
    EXPECT_TRUE(Vec3(0, 2, 3) != Vec3(1, 2, 3));
    EXPECT_TRUE(Vec3(1, 0, 3) != Vec3(1, 2, 3));
    EXPECT_TRUE(Vec3(1, 2, 0) != Vec3(1, 2, 3));
}

TEST(Vec3Tests, testUnaryMinus) {
    EXPECT_EQ(-Vec3(1, 2, 3), Vec3(-1, -2, -3));
}

TEST(Vec3Tests, testArithmetic) {
    EXPECT_EQ(Vec3(1, 2, 3) + Vec3(4, 5, 6), Vec3(5, 7, 9));
    EXPECT_EQ(Vec3(1, 2, 3) - Vec3(4, 5, 6), Vec3(-3, -3, -3));
    EXPECT_EQ(Vec3(1, 2, 3) * Vec3(4, 5, 6), Vec3(4, 10, 18));
    EXPECT_EQ(Vec3(2, 4, 8) / Vec3(1, 2, 4), Vec3(2, 2, 2));
}

TEST(Vec3Tests, testVec3FloatArithmetic) {
    EXPECT_EQ(Vec3(1, 2, 3) + 3, Vec3(4, 5, 6));
    EXPECT_EQ(Vec3(1, 2, 3) - 3, Vec3(-2, -1, 0));
    EXPECT_EQ(Vec3(1, 2, 3) * 3, Vec3(3, 6, 9));
    EXPECT_EQ(Vec3(2, 4, 8) / 2, Vec3(1, 2, 4));
}

TEST(Vec3Tests, testFloatVec3Arithmetic) {
    EXPECT_EQ(3 + Vec3(1, 2, 3), Vec3(4, 5, 6));
    EXPECT_EQ(3 - Vec3(1, 2, 3), Vec3(2, 1, 0));
    EXPECT_EQ(3 * Vec3(1, 2, 3), Vec3(3, 6, 9));
    EXPECT_EQ(16 / Vec3(2, 4, 8), Vec3(8, 4, 2));
}

TEST(Vec3Tests, testVec3Vec3ArithmeticAssignmentOperators) {
    Vec3 x1 (1, 2, 3);
    EXPECT_EQ((x1 += Vec3(4, 5, 6)), Vec3(5, 7, 9));
    EXPECT_EQ(x1, Vec3(5, 7, 9));
    
    Vec3 x2 (1, 2, 3);
    EXPECT_EQ((x2 -= Vec3(4, 5, 6)), Vec3(-3, -3, -3));
    EXPECT_EQ(x2, Vec3(-3, -3, -3));
    
    Vec3 x3 (1, 2, 3);
    EXPECT_EQ((x3 *= Vec3(4, 5, 6)), Vec3(4, 10, 18));
    EXPECT_EQ(x3, Vec3(4, 10, 18));
    
    Vec3 x4 (4, 8, 16);
    EXPECT_EQ((x4 /= Vec3(2, 4, 8)), Vec3(2, 2, 2));
    EXPECT_EQ(x4, Vec3(2, 2, 2));
}

TEST(Vec3Tests, testVec3FloatArithmeticAssignmentOperators) {
    Vec3 x1 (1, 2, 3);
    EXPECT_EQ((x1 += 1), Vec3(2, 3, 4));
    EXPECT_EQ(x1, Vec3(2, 3, 4));
    
    Vec3 x2 (1, 2, 3);
    EXPECT_EQ((x2 -= 1), Vec3(0, 1, 2));
    EXPECT_EQ(x2, Vec3(0, 1, 2));
    
    Vec3 x3 (1, 2, 3);
    EXPECT_EQ((x3 *= 3), Vec3(3, 6, 9));
    EXPECT_EQ(x3, Vec3(3, 6, 9));
    
    Vec3 x4 (4, 8, 16);
    EXPECT_EQ((x4 /= 2), Vec3(2, 4, 8));
    EXPECT_EQ(x4, Vec3(2, 4, 8));
}

TEST(Vec3Tests, testVec3Pow) {
    Vec3 x1 (1, 2, 3);
    EXPECT_EQ(Vec3::pow(x1, 2.0), Vec3(1, 4, 9));
    EXPECT_EQ(x1, Vec3(1, 2, 3));
}

TEST(Vec3Tests, testVec3IndexOperator) {
    Vec3 x1 (1, 2, 3);
    EXPECT_EQ(x1[0], 1);
    EXPECT_EQ(x1[1], 2);
    EXPECT_EQ(x1[2], 3);
}

TEST(Vec3Tests, testSquaredDistanceBetween) {
    EXPECT_EQ(Vec3::squaredDistanceBetween(Vec3{1.0f, 2.0f, 3.0f}, Vec3{10.0f, 30.0f, 5.0f}), 9*9 + 28*28 + 2*2);
}


TEST(Vec3Tests, testDistanceBetween) {
    EXPECT_EQ(Vec3::distanceBetween(Vec3{9.0f, 2.0f, 3.0f}, Vec3{10.0f, 4.0f, 5.0f}), 3.0f);
}
