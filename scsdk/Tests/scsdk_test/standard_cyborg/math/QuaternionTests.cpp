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


#include <doctest/doctest.h>
#include <cmath>

#ifndef CHECK_NEAR
#define CHECK_NEAR(a, b, tol) CHECK(std::abs(static_cast<double>(a) - static_cast<double>(b)) <= static_cast<double>(tol))
#endif

#include <standard_cyborg/math/Quaternion.hpp>
#include <standard_cyborg/math/Mat3x3.hpp>


namespace math = standard_cyborg::math;
using math::Quaternion;

TEST_CASE("QuaternionTests.testConstruction") {
    Quaternion q;
    CHECK_EQ(q.x, 0.0f);
    CHECK_EQ(q.y, 0.0f);
    CHECK_EQ(q.z, 0.0f);
    CHECK_EQ(q.w, 1.0f);
}

TEST_CASE("QuaternionTests.testIdentity") {
    Quaternion q (Quaternion::Identity());
    CHECK_EQ(q.x, 0.0f);
    CHECK_EQ(q.y, 0.0f);
    CHECK_EQ(q.z, 0.0f);
    CHECK_EQ(q.w, 1.0f);
}

TEST_CASE("QuaternionTests.testConstructionFromParts") {
    using math::Vec2;
    using math::Vec3;
    
    CHECK_EQ(Quaternion(Vec2(1, 2), 3, 4), Quaternion(1, 2, 3, 4));
    CHECK_EQ(Quaternion(Vec2(1, 2), Vec2(3, 4)), Quaternion(1, 2, 3, 4));
    CHECK_EQ(Quaternion(1, Vec2(2, 3), 4), Quaternion(1, 2, 3, 4));
    CHECK_EQ(Quaternion(1, 2, Vec2(3, 4)), Quaternion(1, 2, 3, 4));
    CHECK_EQ(Quaternion(1, Vec3(2, 3, 4)), Quaternion(1, 2, 3, 4));
    CHECK_EQ(Quaternion(Vec3(1, 2, 3), 4), Quaternion(1, 2, 3, 4));
}

TEST_CASE("QuaternionTests.testXYZAsVec3") {
    CHECK_EQ(Quaternion(1, 2, 3, 4).xyz(), math::Vec3(1, 2, 3));
}

TEST_CASE("QuaternionTests.testDotProduct") {
    CHECK_EQ(Quaternion::dot(Quaternion(1, 2, 3, 4), Quaternion(4, 5, 6, 7)), 4 + 10 + 18 + 28);
}

TEST_CASE("QuaternionTests.testSquaredNorm") {
    CHECK_EQ(Quaternion(1, 2, 3, 4).squaredNorm(), 30);
}

TEST_CASE("QuaternionTests.testNorm") {
    CHECK_NEAR(Quaternion(8, 9, 12, 15).norm(), 22.671568f, 1e-8f);
}

TEST_CASE("QuaternionTests.testTransformingVec3") {
    math::Vec3 x = Quaternion::Identity().rotateX(M_PI_2) * math::Vec3(1, 2, 3);
    CHECK(math::Vec3::almostEqual(x, math::Vec3(1, -3, 2)));
}

TEST_CASE("QuaternionTests.testNormalizedByValue") {
    Quaternion x (8, 9, 12, 15);
    Quaternion y = Quaternion::normalize(x);
    CHECK_NEAR(y.x, 8.0f / 22.671568f, 1e-5);
    CHECK_NEAR(y.y, 9.0f / 22.671568f, 1e-5);
    CHECK_NEAR(y.z, 12.0f / 22.671568f, 1e-5);
    CHECK_NEAR(y.w, 15.0f / 22.671568f, 1e-5);
    CHECK_EQ(x, Quaternion(8, 9, 12, 15));
}

TEST_CASE("QuaternionTests.testLinearInterpolation") {
    Quaternion a (1.0f, 2.0f, 3.0f, 4.0);
    Quaternion b (2.0f, 4.0f, 6.0f, 8.0);
    CHECK_EQ(Quaternion::lerp(a, b, 0.25), Quaternion(1.25f, 2.5f, 3.75f, 5.0f));
}

TEST_CASE("QuaternionTests.testNormalize") {
    Quaternion x (8, 9, 12, 15.0);
    Quaternion expected (8.0f / 22.671568f, 9.0 / 22.671568f, 12.0 / 22.671568f, 15.0 / 22.671568f);
    Quaternion returnValue = x.normalize();
    
    // Ensure return value is correct
    CHECK_NEAR(returnValue.x, expected.x, 1e-7);
    CHECK_NEAR(returnValue.y, expected.y, 1e-7);
    CHECK_NEAR(returnValue.z, expected.z, 1e-7);
    CHECK_NEAR(returnValue.w, expected.w, 1e-7);
    
    // Ensure x is mutated
    CHECK_NEAR(x.x, expected.x, 1e-7);
    CHECK_NEAR(x.y, expected.y, 1e-7);
    CHECK_NEAR(x.z, expected.z, 1e-7);
    CHECK_NEAR(x.w, expected.w, 1e-7);
}

TEST_CASE("QuaternionTests.testNormalizeZeroVector") {
    Quaternion v (Quaternion(0.0f, 0.0f, 0.0f, 0.0f).normalize());
    CHECK(std::isnan(v.x));
    CHECK(std::isnan(v.y));
    CHECK(std::isnan(v.z));
    CHECK(std::isnan(v.w));
}

TEST_CASE("QuaternionTests.testNormalizedZeroVector") {
    Quaternion v (Quaternion::normalize(Quaternion(0.0f, 0.0f, 0.0f, 0.0f)));
    CHECK(std::isnan(v.x));
    CHECK(std::isnan(v.y));
    CHECK(std::isnan(v.z));
    CHECK(std::isnan(v.w));
}

TEST_CASE("QuaternionTests.testEquality") {
    CHECK(Quaternion(1, 2, 3, 4) == Quaternion(1, 2, 3, 4));
    CHECK_FALSE(Quaternion(0, 2, 3, 4) == Quaternion(1, 2, 3, 4));
    CHECK_FALSE(Quaternion(1, 0, 3, 4) == Quaternion(1, 2, 3, 4));
    CHECK_FALSE(Quaternion(1, 2, 0, 4) == Quaternion(1, 2, 3, 4));
    CHECK_FALSE(Quaternion(1, 2, 3, 0) == Quaternion(1, 2, 3, 4));
}

TEST_CASE("QuaternionTests.testInequality") {
    CHECK_FALSE(Quaternion(1, 2, 3, 4) != Quaternion(1, 2, 3, 4));
    CHECK(Quaternion(0, 2, 3, 4) != Quaternion(1, 2, 3, 4));
    CHECK(Quaternion(1, 0, 3, 4) != Quaternion(1, 2, 3, 4));
    CHECK(Quaternion(1, 2, 0, 4) != Quaternion(1, 2, 3, 4));
    CHECK(Quaternion(1, 2, 3, 0) != Quaternion(1, 2, 3, 4));
}

TEST_CASE("QuaternionTests.testUnaryMinus") {
    CHECK_EQ(-Quaternion(1, 2, 3, 4), Quaternion(-1, -2, -3, -4));
}

TEST_CASE("QuaternionTests.testArithmetic") {
    CHECK_EQ(Quaternion(1, 2, 3, 4) + Quaternion(4, 5, 6, 7), Quaternion(5, 7, 9, 11));
    CHECK_EQ(Quaternion(1, 2, 3, 4) * Quaternion(4, 5, 6, 7), Quaternion(20, 40, 42, -4));
}

TEST_CASE("QuaternionTests.testQuaternionFloatArithmetic") {
    CHECK_EQ(Quaternion(1, 2, 3, 4) + 3, Quaternion(4, 5, 6, 7));
    CHECK_EQ(Quaternion(1, 2, 3, 4) * 3, Quaternion(3, 6, 9, 12));
}

TEST_CASE("QuaternionTests.testFloatQuaternionArithmetic") {
    CHECK_EQ(3 + Quaternion(1, 2, 3, 4), Quaternion(4, 5, 6, 7));
    CHECK_EQ(3 * Quaternion(1, 2, 3, 4), Quaternion(3, 6, 9, 12));
}

TEST_CASE("QuaternionTests.testQuaternionQuaternionArithmeticAssignmentOperators") {
    Quaternion x1 (1, 2, 3, 4);
    CHECK_EQ((x1 += Quaternion(4, 5, 6, 7)), Quaternion(5, 7, 9, 11));
    CHECK_EQ(x1, Quaternion(5, 7, 9, 11));
    
    Quaternion x3 (1, 2, 3, 4);
    CHECK_EQ((x3 *= Quaternion(4, 5, 6, 7)), Quaternion(20, 40, 42, -4));
    CHECK_EQ(x3, Quaternion(20, 40, 42, -4));
}

TEST_CASE("QuaternionTests.testQuaternionFloatArithmeticAssignmentOperators") {
    Quaternion x1 (1, 2, 3, 4);
    CHECK_EQ((x1 += 1), Quaternion(2, 3, 4, 5));
    CHECK_EQ(x1, Quaternion(2, 3, 4, 5));
    
    Quaternion x3 (1, 2, 3, 4);
    CHECK_EQ((x3 *= 3), Quaternion(3, 6, 9, 12));
    CHECK_EQ(x3, Quaternion(3, 6, 9, 12));
}

TEST_CASE("QuaternionTests.testFromMat3x3") {
    math::Mat3x3 m {
        -0.6f, 0.0f, 0.8f,
        -0.8f, 0.0f, -0.6f,
        0.0f, -1.0f, 0.0f
    };
    
    Quaternion q (Quaternion::fromMat3x3(m));
    q *= (1.0 / q.x);
    
    CHECK_EQ(q, Quaternion({1, -2, 2, -1}));
}

TEST_CASE("QuaternionTests.testConjugated") {
    Quaternion q {1, 2, 3, 4};
    CHECK_EQ(q.conjugated(), Quaternion(-1, -2, -3, 4));
    CHECK_EQ(q, Quaternion(1, 2, 3, 4));
}

TEST_CASE("QuaternionTests.testConjugate") {
    Quaternion q {1, 2, 3, 4};
    CHECK_EQ(q.conjugate(), Quaternion(-1, -2, -3, 4));
    CHECK_EQ(q, Quaternion(-1, -2, -3, 4));
}

TEST_CASE("QuaternionTests.testRotateX") {
    Quaternion q {1, 2, 3, 4};
    
    // Check returns a rotated reference
    CHECK(Quaternion::almostEqual(q.rotateX(M_PI), {4, 3, -2, -1}));
    
    // Check mutated the original
    CHECK(Quaternion::almostEqual(q, {4, 3, -2, -1}));
}

TEST_CASE("QuaternionTests.testRotateY") {
    Quaternion q {1, 2, 3, 4};
    
    // Check returns a rotated reference
    CHECK(Quaternion::almostEqual(q.rotateY(M_PI), {-3, 4, 1, -2}));
    
    // Check mutated the original
    CHECK(Quaternion::almostEqual(q, {-3, 4, 1, -2}));
}

TEST_CASE("QuaternionTests.testRotateZ") {
    Quaternion q {1, 2, 3, 4};
    
    // Check returns a rotated reference
    CHECK(Quaternion::almostEqual(q.rotateZ(M_PI), {2, -1, 4, -3}));
    
    // Check mutated the original
    CHECK(Quaternion::almostEqual(q, {2, -1, 4, -3}));
}

TEST_CASE("QuaternionTests.testSlerp") {
    Quaternion a (Quaternion::normalize({1, 2, 3, 4}));
    Quaternion b (Quaternion::normalize({2, 3, -4, 5}));
    
    CHECK(Quaternion::almostEqual(Quaternion::slerp(a, b, 0.0f), a));
    CHECK(Quaternion::almostEqual(Quaternion::slerp(a, b, 0.5f), {0.272f, 0.462602f, 0.00202861f, 0.843808f}, 1.0e-5, 1.0e-5));
    CHECK(Quaternion::almostEqual(Quaternion::slerp(a, b, 1.0f), b));
}

TEST_CASE("QuaternionTests.testInvert") {
    Quaternion q {1, 2, -1, -2};
    CHECK(Quaternion::almostEqual(q.invert(), {-0.1f, -0.2f, 0.1f, -0.2f}));
    CHECK(Quaternion::almostEqual(q, {-0.1f, -0.2f, 0.1f, -0.2f}));
}

TEST_CASE("QuaternionTests.testInverse") {
    Quaternion q {1, 2, -1, -2};
    CHECK(Quaternion::almostEqual(q.inverse(), {-0.1f, -0.2f, 0.1f, -0.2f}));
    CHECK(Quaternion::almostEqual(q, {1, 2, -1, -2}));
}

TEST_CASE("QuaternionTests.testFromRotationX") {
    Quaternion q(Quaternion::fromRotationX(M_PI_2));
    CHECK(Quaternion::almostEqual(q, Quaternion::normalize({1, 0, 0, 1})));
}

TEST_CASE("QuaternionTests.testFromRotationY") {
    Quaternion q(Quaternion::fromRotationY(M_PI_2));
    CHECK(Quaternion::almostEqual(q, Quaternion::normalize({0, 1, 0, 1})));
}

TEST_CASE("QuaternionTests.testFromRotationZ") {
    Quaternion q(Quaternion::fromRotationZ(M_PI_2));
    CHECK(Quaternion::almostEqual(q, Quaternion::normalize({0, 0, 1, 1})));
}
