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

#include "standard_cyborg/math/Vec4.hpp"

namespace math = standard_cyborg::math;
using math::Vec4;

TEST(Vec4Tests, testConstructionFromParts) {
    using math::Vec2;
    using math::Vec3;
    
    EXPECT_EQ(Vec4(Vec2(1, 2), 3, 4), Vec4(1, 2, 3, 4));
    EXPECT_EQ(Vec4(Vec2(1, 2), Vec2(3, 4)), Vec4(1, 2, 3, 4));
    EXPECT_EQ(Vec4(1, Vec2(2, 3), 4), Vec4(1, 2, 3, 4));
    EXPECT_EQ(Vec4(1, 2, Vec2(3, 4)), Vec4(1, 2, 3, 4));
    EXPECT_EQ(Vec4(1, Vec3(2, 3, 4)), Vec4(1, 2, 3, 4));
    EXPECT_EQ(Vec4(Vec3(1, 2, 3), 4), Vec4(1, 2, 3, 4));
}

TEST(Vec4Tests, testXYZAsVec3) {
    EXPECT_EQ(Vec4(1, 2, 3, 4).xyz(), math::Vec3(1, 2, 3));
}

TEST(Vec4Tests, testDotProduct) {
    EXPECT_EQ(Vec4::dot(Vec4(1, 2, 3, 4), Vec4(4, 5, 6, 7)), 4 + 10 + 18 + 28);
}

TEST(Vec4Tests, testSquaredNorm) {
    EXPECT_EQ(Vec4(1, 2, 3, 4).squaredNorm(), 30);
}

TEST(Vec4Tests, testNorm) {
    EXPECT_NEAR(Vec4(8, 9, 12, 15).norm(), 22.671568f, 1e-8f);
}

TEST(Vec4Tests, testNormalizedByValue) {
    Vec4 x (8, 9, 12, 15);
    Vec4 y = Vec4::normalize(x);
    EXPECT_NEAR(y.x, 8.0f / 22.671568f, 1e-5);
    EXPECT_NEAR(y.y, 9.0f / 22.671568f, 1e-5);
    EXPECT_NEAR(y.z, 12.0f / 22.671568f, 1e-5);
    EXPECT_NEAR(y.w, 15.0f / 22.671568f, 1e-5);
    EXPECT_EQ(x, Vec4(8, 9, 12, 15));
}

TEST(Vec4Tests, testLinearInterpolation) {
    Vec4 a (1.0f, 2.0f, 3.0f, 4.0);
    Vec4 b (2.0f, 4.0f, 6.0f, 8.0);
    EXPECT_EQ(Vec4::lerp(a, b, 0.25), Vec4(1.25f, 2.5f, 3.75f, 5.0f));
}

TEST(Vec4Tests, testPerComponentMin) {
    Vec4 a (1.0f, 2.0f, 3.0f, 4.0f);
    Vec4 b (2.0f, 1.0f, 4.0f, 7.0f);
    EXPECT_EQ(Vec4::min(a, b), Vec4(1.0f, 1.0f, 3.0f, 4.0));
}

TEST(Vec4Tests, testPerComponentMax) {
    Vec4 a (1.0f, 2.0f, 3.0f, 4.0f);
    Vec4 b (2.0f, 1.0f, 4.0f, 7.0f);
    EXPECT_EQ(Vec4::max(a, b), Vec4(2.0f, 2.0f, 4.0f, 7.0f));
}

TEST(Vec4Tests, testNormalize) {
    Vec4 x (8, 9, 12, 15.0);
    Vec4 expected (8.0f / 22.671568f, 9.0 / 22.671568f, 12.0 / 22.671568f, 15.0 / 22.671568f);
    Vec4 returnValue = x.normalize();
    
    // Ensure return value is correct
    EXPECT_NEAR(returnValue.x, expected.x, 1e-7);
    EXPECT_NEAR(returnValue.y, expected.y, 1e-7);
    EXPECT_NEAR(returnValue.z, expected.z, 1e-7);
    EXPECT_NEAR(returnValue.w, expected.w, 1e-7);
    
    // Ensure x is mutated
    EXPECT_NEAR(x.x, expected.x, 1e-7);
    EXPECT_NEAR(x.y, expected.y, 1e-7);
    EXPECT_NEAR(x.z, expected.z, 1e-7);
    EXPECT_NEAR(x.w, expected.w, 1e-7);
}

TEST(Vec4Tests, testNormalizeZeroVector) {
    Vec4 v (Vec4(0.0f, 0.0f, 0.0f, 0.0f).normalize());
    EXPECT_TRUE(std::isnan(v.x));
    EXPECT_TRUE(std::isnan(v.y));
    EXPECT_TRUE(std::isnan(v.z));
    EXPECT_TRUE(std::isnan(v.w));
}

TEST(Vec4Tests, testNormalizedZeroVector) {
    Vec4 v (Vec4::normalize(Vec4(0.0f, 0.0f, 0.0f, 0.0f)));
    EXPECT_TRUE(std::isnan(v.x));
    EXPECT_TRUE(std::isnan(v.y));
    EXPECT_TRUE(std::isnan(v.z));
    EXPECT_TRUE(std::isnan(v.w));
}

TEST(Vec4Tests, testEquality) {
    EXPECT_TRUE(Vec4(1, 2, 3, 4) == Vec4(1, 2, 3, 4));
    EXPECT_FALSE(Vec4(0, 2, 3, 4) == Vec4(1, 2, 3, 4));
    EXPECT_FALSE(Vec4(1, 0, 3, 4) == Vec4(1, 2, 3, 4));
    EXPECT_FALSE(Vec4(1, 2, 0, 4) == Vec4(1, 2, 3, 4));
    EXPECT_FALSE(Vec4(1, 2, 3, 0) == Vec4(1, 2, 3, 4));
}

TEST(Vec4Tests, testInequality) {
    EXPECT_FALSE(Vec4(1, 2, 3, 4) != Vec4(1, 2, 3, 4));
    EXPECT_TRUE(Vec4(0, 2, 3, 4) != Vec4(1, 2, 3, 4));
    EXPECT_TRUE(Vec4(1, 0, 3, 4) != Vec4(1, 2, 3, 4));
    EXPECT_TRUE(Vec4(1, 2, 0, 4) != Vec4(1, 2, 3, 4));
    EXPECT_TRUE(Vec4(1, 2, 3, 0) != Vec4(1, 2, 3, 4));
}

TEST(Vec4Tests, testUnaryMinus) {
    EXPECT_EQ(-Vec4(1, 2, 3, 4), Vec4(-1, -2, -3, -4));
}

TEST(Vec4Tests, testArithmetic) {
    EXPECT_EQ(Vec4(1, 2, 3, 4) + Vec4(4, 5, 6, 7), Vec4(5, 7, 9, 11));
    EXPECT_EQ(Vec4(1, 2, 3, 4) - Vec4(4, 5, 6, 7), Vec4(-3, -3, -3, -3));
    EXPECT_EQ(Vec4(1, 2, 3, 4) * Vec4(4, 5, 6, 7), Vec4(4, 10, 18, 28));
    EXPECT_EQ(Vec4(2, 4, 8, 16) / Vec4(1, 2, 4, 8), Vec4(2, 2, 2, 2));
}

TEST(Vec4Tests, testVec4FloatArithmetic) {
    EXPECT_EQ(Vec4(1, 2, 3, 4) + 3, Vec4(4, 5, 6, 7));
    EXPECT_EQ(Vec4(1, 2, 3, 4) - 3, Vec4(-2, -1, 0, 1));
    EXPECT_EQ(Vec4(1, 2, 3, 4) * 3, Vec4(3, 6, 9, 12));
    EXPECT_EQ(Vec4(2, 4, 8, 16) / 2, Vec4(1, 2, 4, 8));
}

TEST(Vec4Tests, testFloatVec4Arithmetic) {
    EXPECT_EQ(3 + Vec4(1, 2, 3, 4), Vec4(4, 5, 6, 7));
    EXPECT_EQ(3 - Vec4(1, 2, 3, 4), Vec4(2, 1, 0, -1));
    EXPECT_EQ(3 * Vec4(1, 2, 3, 4), Vec4(3, 6, 9, 12));
    EXPECT_EQ(16 / Vec4(2, 4, 8, 16), Vec4(8, 4, 2, 1));
}

TEST(Vec4Tests, testVec4Vec4ArithmeticAssignmentOperators) {
    Vec4 x1 (1, 2, 3, 4);
    EXPECT_EQ((x1 += Vec4(4, 5, 6, 7)), Vec4(5, 7, 9, 11));
    EXPECT_EQ(x1, Vec4(5, 7, 9, 11));
    
    Vec4 x2 (1, 2, 3, 4);
    EXPECT_EQ((x2 -= Vec4(4, 5, 6, 7)), Vec4(-3, -3, -3, -3));
    EXPECT_EQ(x2, Vec4(-3, -3, -3, -3));
    
    Vec4 x3 (1, 2, 3, 4);
    EXPECT_EQ((x3 *= Vec4(4, 5, 6, 7)), Vec4(4, 10, 18, 28));
    EXPECT_EQ(x3, Vec4(4, 10, 18, 28));
    
    Vec4 x4 (4, 8, 16, 32);
    EXPECT_EQ((x4 /= Vec4(2, 4, 8, 16)), Vec4(2, 2, 2, 2));
    EXPECT_EQ(x4, Vec4(2, 2, 2, 2));
}

TEST(Vec4Tests, testVec4FloatArithmeticAssignmentOperators) {
    Vec4 x1 (1, 2, 3, 4);
    EXPECT_EQ((x1 += 1), Vec4(2, 3, 4, 5));
    EXPECT_EQ(x1, Vec4(2, 3, 4, 5));
    
    Vec4 x2 (1, 2, 3, 4);
    EXPECT_EQ((x2 -= 1), Vec4(0, 1, 2, 3));
    EXPECT_EQ(x2, Vec4(0, 1, 2, 3));
    
    Vec4 x3 (1, 2, 3, 4);
    EXPECT_EQ((x3 *= 3), Vec4(3, 6, 9, 12));
    EXPECT_EQ(x3, Vec4(3, 6, 9, 12));
    
    Vec4 x4 (4, 8, 16, 32);
    EXPECT_EQ((x4 /= 2), Vec4(2, 4, 8, 16));
    EXPECT_EQ(x4, Vec4(2, 4, 8, 16));
}

TEST(Vec4Tests, testVec4Pow) {
    Vec4 x1 (1, 2, 3, 4);
    EXPECT_EQ(Vec4::pow(x1, 2.0), Vec4(1, 4, 9, 16));
    EXPECT_EQ(x1, Vec4(1, 2, 3, 4));
}
