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

#include "standard_cyborg/math/Quaternion.hpp"
#include "standard_cyborg/math/Mat3x3.hpp"


namespace math = standard_cyborg::math;
using math::Quaternion;

TEST(QuaternionTests, testConstruction) {
    Quaternion q;
    EXPECT_EQ(q.x, 0.0f);
    EXPECT_EQ(q.y, 0.0f);
    EXPECT_EQ(q.z, 0.0f);
    EXPECT_EQ(q.w, 1.0f);
}

TEST(QuaternionTests, testIdentity) {
    Quaternion q (Quaternion::Identity());
    EXPECT_EQ(q.x, 0.0f);
    EXPECT_EQ(q.y, 0.0f);
    EXPECT_EQ(q.z, 0.0f);
    EXPECT_EQ(q.w, 1.0f);
}

TEST(QuaternionTests, testConstructionFromParts) {
    using math::Vec2;
    using math::Vec3;
    
    EXPECT_EQ(Quaternion(Vec2(1, 2), 3, 4), Quaternion(1, 2, 3, 4));
    EXPECT_EQ(Quaternion(Vec2(1, 2), Vec2(3, 4)), Quaternion(1, 2, 3, 4));
    EXPECT_EQ(Quaternion(1, Vec2(2, 3), 4), Quaternion(1, 2, 3, 4));
    EXPECT_EQ(Quaternion(1, 2, Vec2(3, 4)), Quaternion(1, 2, 3, 4));
    EXPECT_EQ(Quaternion(1, Vec3(2, 3, 4)), Quaternion(1, 2, 3, 4));
    EXPECT_EQ(Quaternion(Vec3(1, 2, 3), 4), Quaternion(1, 2, 3, 4));
}

TEST(QuaternionTests, testXYZAsVec3) {
    EXPECT_EQ(Quaternion(1, 2, 3, 4).xyz(), math::Vec3(1, 2, 3));
}

TEST(QuaternionTests, testDotProduct) {
    EXPECT_EQ(Quaternion::dot(Quaternion(1, 2, 3, 4), Quaternion(4, 5, 6, 7)), 4 + 10 + 18 + 28);
}

TEST(QuaternionTests, testSquaredNorm) {
    EXPECT_EQ(Quaternion(1, 2, 3, 4).squaredNorm(), 30);
}

TEST(QuaternionTests, testNorm) {
    EXPECT_NEAR(Quaternion(8, 9, 12, 15).norm(), 22.671568f, 1e-8f);
}

TEST(QuaternionTests, testTransformingVec3) {
    math::Vec3 x = Quaternion::Identity().rotateX(M_PI_2) * math::Vec3(1, 2, 3);
    EXPECT_TRUE(math::Vec3::almostEqual(x, math::Vec3(1, -3, 2)));
}

TEST(QuaternionTests, testNormalizedByValue) {
    Quaternion x (8, 9, 12, 15);
    Quaternion y = Quaternion::normalize(x);
    EXPECT_NEAR(y.x, 8.0f / 22.671568f, 1e-5);
    EXPECT_NEAR(y.y, 9.0f / 22.671568f, 1e-5);
    EXPECT_NEAR(y.z, 12.0f / 22.671568f, 1e-5);
    EXPECT_NEAR(y.w, 15.0f / 22.671568f, 1e-5);
    EXPECT_EQ(x, Quaternion(8, 9, 12, 15));
}

TEST(QuaternionTests, testLinearInterpolation) {
    Quaternion a (1.0f, 2.0f, 3.0f, 4.0);
    Quaternion b (2.0f, 4.0f, 6.0f, 8.0);
    EXPECT_EQ(Quaternion::lerp(a, b, 0.25), Quaternion(1.25f, 2.5f, 3.75f, 5.0f));
}

TEST(QuaternionTests, testNormalize) {
    Quaternion x (8, 9, 12, 15.0);
    Quaternion expected (8.0f / 22.671568f, 9.0 / 22.671568f, 12.0 / 22.671568f, 15.0 / 22.671568f);
    Quaternion returnValue = x.normalize();
    
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

TEST(QuaternionTests, testNormalizeZeroVector) {
    Quaternion v (Quaternion(0.0f, 0.0f, 0.0f, 0.0f).normalize());
    EXPECT_TRUE(std::isnan(v.x));
    EXPECT_TRUE(std::isnan(v.y));
    EXPECT_TRUE(std::isnan(v.z));
    EXPECT_TRUE(std::isnan(v.w));
}

TEST(QuaternionTests, testNormalizedZeroVector) {
    Quaternion v (Quaternion::normalize(Quaternion(0.0f, 0.0f, 0.0f, 0.0f)));
    EXPECT_TRUE(std::isnan(v.x));
    EXPECT_TRUE(std::isnan(v.y));
    EXPECT_TRUE(std::isnan(v.z));
    EXPECT_TRUE(std::isnan(v.w));
}

TEST(QuaternionTests, testEquality) {
    EXPECT_TRUE(Quaternion(1, 2, 3, 4) == Quaternion(1, 2, 3, 4));
    EXPECT_FALSE(Quaternion(0, 2, 3, 4) == Quaternion(1, 2, 3, 4));
    EXPECT_FALSE(Quaternion(1, 0, 3, 4) == Quaternion(1, 2, 3, 4));
    EXPECT_FALSE(Quaternion(1, 2, 0, 4) == Quaternion(1, 2, 3, 4));
    EXPECT_FALSE(Quaternion(1, 2, 3, 0) == Quaternion(1, 2, 3, 4));
}

TEST(QuaternionTests, testInequality) {
    EXPECT_FALSE(Quaternion(1, 2, 3, 4) != Quaternion(1, 2, 3, 4));
    EXPECT_TRUE(Quaternion(0, 2, 3, 4) != Quaternion(1, 2, 3, 4));
    EXPECT_TRUE(Quaternion(1, 0, 3, 4) != Quaternion(1, 2, 3, 4));
    EXPECT_TRUE(Quaternion(1, 2, 0, 4) != Quaternion(1, 2, 3, 4));
    EXPECT_TRUE(Quaternion(1, 2, 3, 0) != Quaternion(1, 2, 3, 4));
}

TEST(QuaternionTests, testUnaryMinus) {
    EXPECT_EQ(-Quaternion(1, 2, 3, 4), Quaternion(-1, -2, -3, -4));
}

TEST(QuaternionTests, testArithmetic) {
    EXPECT_EQ(Quaternion(1, 2, 3, 4) + Quaternion(4, 5, 6, 7), Quaternion(5, 7, 9, 11));
    EXPECT_EQ(Quaternion(1, 2, 3, 4) * Quaternion(4, 5, 6, 7), Quaternion(20, 40, 42, -4));
}

TEST(QuaternionTests, testQuaternionFloatArithmetic) {
    EXPECT_EQ(Quaternion(1, 2, 3, 4) + 3, Quaternion(4, 5, 6, 7));
    EXPECT_EQ(Quaternion(1, 2, 3, 4) * 3, Quaternion(3, 6, 9, 12));
}

TEST(QuaternionTests, testFloatQuaternionArithmetic) {
    EXPECT_EQ(3 + Quaternion(1, 2, 3, 4), Quaternion(4, 5, 6, 7));
    EXPECT_EQ(3 * Quaternion(1, 2, 3, 4), Quaternion(3, 6, 9, 12));
}

TEST(QuaternionTests, testQuaternionQuaternionArithmeticAssignmentOperators) {
    Quaternion x1 (1, 2, 3, 4);
    EXPECT_EQ((x1 += Quaternion(4, 5, 6, 7)), Quaternion(5, 7, 9, 11));
    EXPECT_EQ(x1, Quaternion(5, 7, 9, 11));
    
    Quaternion x3 (1, 2, 3, 4);
    EXPECT_EQ((x3 *= Quaternion(4, 5, 6, 7)), Quaternion(20, 40, 42, -4));
    EXPECT_EQ(x3, Quaternion(20, 40, 42, -4));
}

TEST(QuaternionTests, testQuaternionFloatArithmeticAssignmentOperators) {
    Quaternion x1 (1, 2, 3, 4);
    EXPECT_EQ((x1 += 1), Quaternion(2, 3, 4, 5));
    EXPECT_EQ(x1, Quaternion(2, 3, 4, 5));
    
    Quaternion x3 (1, 2, 3, 4);
    EXPECT_EQ((x3 *= 3), Quaternion(3, 6, 9, 12));
    EXPECT_EQ(x3, Quaternion(3, 6, 9, 12));
}

TEST(QuaternionTests, testFromMat3x3) {
    math::Mat3x3 m {
        -0.6f, 0.0f, 0.8f,
        -0.8f, 0.0f, -0.6f,
        0.0f, -1.0f, 0.0f
    };
    
    Quaternion q (Quaternion::fromMat3x3(m));
    q *= (1.0 / q.x);
    
    EXPECT_EQ(q, Quaternion({1, -2, 2, -1}));
}

TEST(QuaternionTests, testConjugated) {
    Quaternion q {1, 2, 3, 4};
    EXPECT_EQ(q.conjugated(), Quaternion(-1, -2, -3, 4));
    EXPECT_EQ(q, Quaternion(1, 2, 3, 4));
}

TEST(QuaternionTests, testConjugate) {
    Quaternion q {1, 2, 3, 4};
    EXPECT_EQ(q.conjugate(), Quaternion(-1, -2, -3, 4));
    EXPECT_EQ(q, Quaternion(-1, -2, -3, 4));
}

TEST(QuaternionTests, testRotateX) {
    Quaternion q {1, 2, 3, 4};
    
    // Check returns a rotated reference
    EXPECT_TRUE(Quaternion::almostEqual(q.rotateX(M_PI), {4, 3, -2, -1}));
    
    // Check mutated the original
    EXPECT_TRUE(Quaternion::almostEqual(q, {4, 3, -2, -1}));
}

TEST(QuaternionTests, testRotateY) {
    Quaternion q {1, 2, 3, 4};
    
    // Check returns a rotated reference
    EXPECT_TRUE(Quaternion::almostEqual(q.rotateY(M_PI), {-3, 4, 1, -2}));
    
    // Check mutated the original
    EXPECT_TRUE(Quaternion::almostEqual(q, {-3, 4, 1, -2}));
}

TEST(QuaternionTests, testRotateZ) {
    Quaternion q {1, 2, 3, 4};
    
    // Check returns a rotated reference
    EXPECT_TRUE(Quaternion::almostEqual(q.rotateZ(M_PI), {2, -1, 4, -3}));
    
    // Check mutated the original
    EXPECT_TRUE(Quaternion::almostEqual(q, {2, -1, 4, -3}));
}

TEST(QuaternionTests, testSlerp) {
    Quaternion a (Quaternion::normalize({1, 2, 3, 4}));
    Quaternion b (Quaternion::normalize({2, 3, -4, 5}));
    
    EXPECT_TRUE(Quaternion::almostEqual(Quaternion::slerp(a, b, 0.0f), a));
    EXPECT_TRUE(Quaternion::almostEqual(Quaternion::slerp(a, b, 0.5f), {0.272f, 0.462602f, 0.00202861f, 0.843808f}, 1.0e-5, 1.0e-5));
    EXPECT_TRUE(Quaternion::almostEqual(Quaternion::slerp(a, b, 1.0f), b));
}

TEST(QuaternionTests, testInvert) {
    Quaternion q {1, 2, -1, -2};
    EXPECT_TRUE(Quaternion::almostEqual(q.invert(), {-0.1f, -0.2f, 0.1f, -0.2f}));
    EXPECT_TRUE(Quaternion::almostEqual(q, {-0.1f, -0.2f, 0.1f, -0.2f}));
}

TEST(QuaternionTests, testInverse) {
    Quaternion q {1, 2, -1, -2};
    EXPECT_TRUE(Quaternion::almostEqual(q.inverse(), {-0.1f, -0.2f, 0.1f, -0.2f}));
    EXPECT_TRUE(Quaternion::almostEqual(q, {1, 2, -1, -2}));
}

TEST(QuaternionTests, testFromRotationX) {
    Quaternion q(Quaternion::fromRotationX(M_PI_2));
    EXPECT_TRUE(Quaternion::almostEqual(q, Quaternion::normalize({1, 0, 0, 1})));
}

TEST(QuaternionTests, testFromRotationY) {
    Quaternion q(Quaternion::fromRotationY(M_PI_2));
    EXPECT_TRUE(Quaternion::almostEqual(q, Quaternion::normalize({0, 1, 0, 1})));
}

TEST(QuaternionTests, testFromRotationZ) {
    Quaternion q(Quaternion::fromRotationZ(M_PI_2));
    EXPECT_TRUE(Quaternion::almostEqual(q, Quaternion::normalize({0, 0, 1, 1})));
}
