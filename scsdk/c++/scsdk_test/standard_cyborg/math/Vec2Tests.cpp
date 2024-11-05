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
#include "standard_cyborg/sc3d/Size2D.hpp"

namespace math = standard_cyborg::math;
using math::Vec2;

TEST(Vec2Tests, testDotProduct) {
    EXPECT_EQ(Vec2::dot(Vec2(1, 2), Vec2(4, 5)), 4 + 10);
}

TEST(Vec2Tests, testCrossProduct) {
    EXPECT_EQ(Vec2::cross(Vec2(1, 2), Vec2(4, 5)), -3);
}

TEST(Vec2Tests, testSquaredNorm) {
    EXPECT_EQ(Vec2(1, 2).squaredNorm(), 5);
}

TEST(Vec2Tests, testNorm) {
    EXPECT_NEAR(Vec2(3.0, 4.0).norm(), 5.0f, 1e-8f);
}

TEST(Vec2Tests, testNormalizedByValue) {
    Vec2 x (3, 4);
    EXPECT_EQ(Vec2::normalize(x), Vec2(3, 4) / 5.0);
    EXPECT_EQ(x, Vec2(3, 4));
}

TEST(Vec2Tests, testLinearInterpolation) {
    Vec2 a (1.0f, 2.0f);
    Vec2 b (2.0f, 4.0f);
    EXPECT_EQ(Vec2::lerp(a, b, 0.25), Vec2(1.25f, 2.5f));
}

TEST(Vec2Tests, testPerComponentMin) {
    Vec2 a (1.0f, 2.0f);
    Vec2 b (2.0f, 1.0f);
    EXPECT_EQ(Vec2::min(a, b), Vec2(1.0f, 1.0f));
}

TEST(Vec2Tests, testPerComponentMax) {
    Vec2 a (1.0f, 2.0f);
    Vec2 b (2.0f, 1.0f);
    EXPECT_EQ(Vec2::max(a, b), Vec2(2.0f, 2.0f));
}

TEST(Vec2Tests, testNormalize) {
    Vec2 x (3, 4);
    Vec2 expected (3.0f / 5.0f, 4.0f / 5.0f);
    Vec2 returnValue = x.normalize();
    
    // Ensure return value is correct
    EXPECT_NEAR(returnValue.x, expected.x, 1e-8);
    EXPECT_NEAR(returnValue.y, expected.y, 1e-8);
    
    // Ensure x is mutated
    EXPECT_NEAR(x.x, expected.x, 1e-8);
    EXPECT_NEAR(x.y, expected.y, 1e-8);
}

TEST(Vec2Tests, testNormalizeZeroVector) {
    Vec2 v (Vec2(0.0f, 0.0f).normalize());
    EXPECT_TRUE(std::isnan(v.x));
    EXPECT_TRUE(std::isnan(v.y));
}

TEST(Vec2Tests, testNormalizedZeroVector) {
    Vec2 v (Vec2::normalize(Vec2(0.0f, 0.0f)));
    EXPECT_TRUE(std::isnan(v.x));
    EXPECT_TRUE(std::isnan(v.y));
}

TEST(Vec2Tests, testAngleBetween) {
    EXPECT_EQ(Vec2::angleBetween(Vec2(2, 0), Vec2(3, 0)), 0.0f);
    EXPECT_NEAR(Vec2::angleBetween(Vec2(2, 0), Vec2(0, 2)), M_PI_2, 1e-6);
    EXPECT_NEAR(Vec2::angleBetween(Vec2(1, 1), Vec2(-1, -1)), M_PI, 1e-6);
    
    EXPECT_TRUE(std::isnan(Vec2::angleBetween(Vec2(0, 0), Vec2(1, 0))));
    EXPECT_TRUE(std::isnan(Vec2::angleBetween(Vec2(1, 0), Vec2(0, 0))));
}

TEST(Vec2Tests, testEquality) {
    EXPECT_TRUE(Vec2(1, 2) == Vec2(1, 2));
    EXPECT_FALSE(Vec2(0, 2) == Vec2(1, 2));
    EXPECT_FALSE(Vec2(1, 0) == Vec2(1, 2));
}

TEST(Vec2Tests, testInequality) {
    EXPECT_FALSE(Vec2(1, 2) != Vec2(1, 2));
    EXPECT_TRUE(Vec2(0, 2) != Vec2(1, 2));
    EXPECT_TRUE(Vec2(1, 0) != Vec2(1, 2));
}

TEST(Vec2Tests, testUnaryMinus) {
    EXPECT_EQ(-Vec2(1, 2), Vec2(-1, -2));
}

TEST(Vec2Tests, testArithmetic) {
    EXPECT_EQ(Vec2(1, 2) + Vec2(4, 5), Vec2(5, 7));
    EXPECT_EQ(Vec2(1, 2) - Vec2(4, 5), Vec2(-3, -3));
    EXPECT_EQ(Vec2(1, 2) * Vec2(4, 5), Vec2(4, 10));
    EXPECT_EQ(Vec2(2, 4) / Vec2(1, 2), Vec2(2, 2));
}

TEST(Vec2Tests, testVec2FloatArithmetic) {
    EXPECT_EQ(Vec2(1, 2) + 3, Vec2(4, 5));
    EXPECT_EQ(Vec2(1, 2) - 3, Vec2(-2, -1));
    EXPECT_EQ(Vec2(1, 2) * 3, Vec2(3, 6));
    EXPECT_EQ(Vec2(2, 4) / 2, Vec2(1, 2));
}

TEST(Vec2Tests, testFloatVec2Arithmetic) {
    EXPECT_EQ(3 + Vec2(1, 2), Vec2(4, 5));
    EXPECT_EQ(3 - Vec2(1, 2), Vec2(2, 1));
    EXPECT_EQ(3 * Vec2(1, 2), Vec2(3, 6));
    EXPECT_EQ(16 / Vec2(2, 4), Vec2(8, 4));
}

TEST(Vec2Tests, testVec2Vec2ArithmeticAssignmentOperators) {
    Vec2 x1 (1, 2);
    EXPECT_EQ((x1 += Vec2(4, 5)), Vec2(5, 7));
    EXPECT_EQ(x1, Vec2(5, 7));
    
    Vec2 x2 (1, 2);
    EXPECT_EQ((x2 -= Vec2(4, 5)), Vec2(-3, -3));
    EXPECT_EQ(x2, Vec2(-3, -3));
    
    Vec2 x3 (1, 2);
    EXPECT_EQ((x3 *= Vec2(4, 5)), Vec2(4, 10));
    EXPECT_EQ(x3, Vec2(4, 10));
    
    Vec2 x4 (4, 8);
    EXPECT_EQ((x4 /= Vec2(2, 4)), Vec2(2, 2));
    EXPECT_EQ(x4, Vec2(2, 2));
}

TEST(Vec2Tests, testVec2FloatArithmeticAssignmentOperators) {
    Vec2 x1 (1, 2);
    EXPECT_EQ((x1 += 1), Vec2(2, 3));
    EXPECT_EQ(x1, Vec2(2, 3));
    
    Vec2 x2 (1, 2);
    EXPECT_EQ((x2 -= 1), Vec2(0, 1));
    EXPECT_EQ(x2, Vec2(0, 1));
    
    Vec2 x3 (1, 2);
    EXPECT_EQ((x3 *= 3), Vec2(3, 6));
    EXPECT_EQ(x3, Vec2(3, 6));
    
    Vec2 x4 (4, 8);
    EXPECT_EQ((x4 /= 2), Vec2(2, 4));
    EXPECT_EQ(x4, Vec2(2, 4));
}

TEST(Vec2Tests, testVec2Pow) {
    Vec2 x1 (1, 2);
    EXPECT_EQ(Vec2::pow(x1, 2.0), Vec2(1, 4));
    EXPECT_EQ(x1, Vec2(1, 2));
}

TEST(Vec2Tests, testVec2Constructor) {
    EXPECT_EQ(Vec2(1, 2), Vec2(1, 2));
    EXPECT_EQ(Vec2(10), Vec2(10));
    EXPECT_EQ(Vec2(standard_cyborg::sc3d::Size2D{1, 2}), Vec2(1.0f, 2.0f));
}
