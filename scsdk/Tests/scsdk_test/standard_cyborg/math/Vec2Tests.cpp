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

#include <standard_cyborg/math/Vec2.hpp>
#include <standard_cyborg/sc3d/Size2D.hpp>

namespace math = standard_cyborg::math;
using math::Vec2;

TEST_CASE("Vec2Tests.testDotProduct") {
    CHECK_EQ(Vec2::dot(Vec2(1, 2), Vec2(4, 5)), 4 + 10);
}

TEST_CASE("Vec2Tests.testCrossProduct") {
    CHECK_EQ(Vec2::cross(Vec2(1, 2), Vec2(4, 5)), -3);
}

TEST_CASE("Vec2Tests.testSquaredNorm") {
    CHECK_EQ(Vec2(1, 2).squaredNorm(), 5);
}

TEST_CASE("Vec2Tests.testNorm") {
    CHECK_NEAR(Vec2(3.0, 4.0).norm(), 5.0f, 1e-8f);
}

TEST_CASE("Vec2Tests.testNormalizedByValue") {
    Vec2 x (3, 4);
    CHECK_EQ(Vec2::normalize(x), Vec2(3, 4) / 5.0);
    CHECK_EQ(x, Vec2(3, 4));
}

TEST_CASE("Vec2Tests.testLinearInterpolation") {
    Vec2 a (1.0f, 2.0f);
    Vec2 b (2.0f, 4.0f);
    CHECK_EQ(Vec2::lerp(a, b, 0.25), Vec2(1.25f, 2.5f));
}

TEST_CASE("Vec2Tests.testPerComponentMin") {
    Vec2 a (1.0f, 2.0f);
    Vec2 b (2.0f, 1.0f);
    CHECK_EQ(Vec2::min(a, b), Vec2(1.0f, 1.0f));
}

TEST_CASE("Vec2Tests.testPerComponentMax") {
    Vec2 a (1.0f, 2.0f);
    Vec2 b (2.0f, 1.0f);
    CHECK_EQ(Vec2::max(a, b), Vec2(2.0f, 2.0f));
}

TEST_CASE("Vec2Tests.testNormalize") {
    Vec2 x (3, 4);
    Vec2 expected (3.0f / 5.0f, 4.0f / 5.0f);
    Vec2 returnValue = x.normalize();
    
    // Ensure return value is correct
    CHECK_NEAR(returnValue.x, expected.x, 1e-8);
    CHECK_NEAR(returnValue.y, expected.y, 1e-8);
    
    // Ensure x is mutated
    CHECK_NEAR(x.x, expected.x, 1e-8);
    CHECK_NEAR(x.y, expected.y, 1e-8);
}

TEST_CASE("Vec2Tests.testNormalizeZeroVector") {
    Vec2 v (Vec2(0.0f, 0.0f).normalize());
    CHECK(std::isnan(v.x));
    CHECK(std::isnan(v.y));
}

TEST_CASE("Vec2Tests.testNormalizedZeroVector") {
    Vec2 v (Vec2::normalize(Vec2(0.0f, 0.0f)));
    CHECK(std::isnan(v.x));
    CHECK(std::isnan(v.y));
}

TEST_CASE("Vec2Tests.testAngleBetween") {
    CHECK_EQ(Vec2::angleBetween(Vec2(2, 0), Vec2(3, 0)), 0.0f);
    CHECK_NEAR(Vec2::angleBetween(Vec2(2, 0), Vec2(0, 2)), M_PI_2, 1e-6);
    CHECK_NEAR(Vec2::angleBetween(Vec2(1, 1), Vec2(-1, -1)), M_PI, 1e-6);
    
    CHECK(std::isnan(Vec2::angleBetween(Vec2(0, 0), Vec2(1, 0))));
    CHECK(std::isnan(Vec2::angleBetween(Vec2(1, 0), Vec2(0, 0))));
}

TEST_CASE("Vec2Tests.testEquality") {
    CHECK(Vec2(1, 2) == Vec2(1, 2));
    CHECK_FALSE(Vec2(0, 2) == Vec2(1, 2));
    CHECK_FALSE(Vec2(1, 0) == Vec2(1, 2));
}

TEST_CASE("Vec2Tests.testInequality") {
    CHECK_FALSE(Vec2(1, 2) != Vec2(1, 2));
    CHECK(Vec2(0, 2) != Vec2(1, 2));
    CHECK(Vec2(1, 0) != Vec2(1, 2));
}

TEST_CASE("Vec2Tests.testUnaryMinus") {
    CHECK_EQ(-Vec2(1, 2), Vec2(-1, -2));
}

TEST_CASE("Vec2Tests.testArithmetic") {
    CHECK_EQ(Vec2(1, 2) + Vec2(4, 5), Vec2(5, 7));
    CHECK_EQ(Vec2(1, 2) - Vec2(4, 5), Vec2(-3, -3));
    CHECK_EQ(Vec2(1, 2) * Vec2(4, 5), Vec2(4, 10));
    CHECK_EQ(Vec2(2, 4) / Vec2(1, 2), Vec2(2, 2));
}

TEST_CASE("Vec2Tests.testVec2FloatArithmetic") {
    CHECK_EQ(Vec2(1, 2) + 3, Vec2(4, 5));
    CHECK_EQ(Vec2(1, 2) - 3, Vec2(-2, -1));
    CHECK_EQ(Vec2(1, 2) * 3, Vec2(3, 6));
    CHECK_EQ(Vec2(2, 4) / 2, Vec2(1, 2));
}

TEST_CASE("Vec2Tests.testFloatVec2Arithmetic") {
    CHECK_EQ(3 + Vec2(1, 2), Vec2(4, 5));
    CHECK_EQ(3 - Vec2(1, 2), Vec2(2, 1));
    CHECK_EQ(3 * Vec2(1, 2), Vec2(3, 6));
    CHECK_EQ(16 / Vec2(2, 4), Vec2(8, 4));
}

TEST_CASE("Vec2Tests.testVec2Vec2ArithmeticAssignmentOperators") {
    Vec2 x1 (1, 2);
    CHECK_EQ((x1 += Vec2(4, 5)), Vec2(5, 7));
    CHECK_EQ(x1, Vec2(5, 7));
    
    Vec2 x2 (1, 2);
    CHECK_EQ((x2 -= Vec2(4, 5)), Vec2(-3, -3));
    CHECK_EQ(x2, Vec2(-3, -3));
    
    Vec2 x3 (1, 2);
    CHECK_EQ((x3 *= Vec2(4, 5)), Vec2(4, 10));
    CHECK_EQ(x3, Vec2(4, 10));
    
    Vec2 x4 (4, 8);
    CHECK_EQ((x4 /= Vec2(2, 4)), Vec2(2, 2));
    CHECK_EQ(x4, Vec2(2, 2));
}

TEST_CASE("Vec2Tests.testVec2FloatArithmeticAssignmentOperators") {
    Vec2 x1 (1, 2);
    CHECK_EQ((x1 += 1), Vec2(2, 3));
    CHECK_EQ(x1, Vec2(2, 3));
    
    Vec2 x2 (1, 2);
    CHECK_EQ((x2 -= 1), Vec2(0, 1));
    CHECK_EQ(x2, Vec2(0, 1));
    
    Vec2 x3 (1, 2);
    CHECK_EQ((x3 *= 3), Vec2(3, 6));
    CHECK_EQ(x3, Vec2(3, 6));
    
    Vec2 x4 (4, 8);
    CHECK_EQ((x4 /= 2), Vec2(2, 4));
    CHECK_EQ(x4, Vec2(2, 4));
}

TEST_CASE("Vec2Tests.testVec2Pow") {
    Vec2 x1 (1, 2);
    CHECK_EQ(Vec2::pow(x1, 2.0), Vec2(1, 4));
    CHECK_EQ(x1, Vec2(1, 2));
}

TEST_CASE("Vec2Tests.testVec2Constructor") {
    CHECK_EQ(Vec2(1, 2), Vec2(1, 2));
    CHECK_EQ(Vec2(10), Vec2(10));
    CHECK_EQ(Vec2(standard_cyborg::sc3d::Size2D{1, 2}), Vec2(1.0f, 2.0f));
}
