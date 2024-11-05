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
 #define _USE_MATH_DEFINES
 #include <cmath>
 #include <iostream>
 
 #include <StandardCyborgData/BoundingBox3.hpp>
 #include <StandardCyborgData/Geometry.hpp>
 #include <StandardCyborgData/DebugHelpers.hpp>
 #include <StandardCyborgData/Vec3.hpp>
 #include <StandardCyborgData/Polyline.hpp>
 
 using StandardCyborg::BoundingBox3;
 
 namespace math = StandardCyborg::math;
 using math::Vec3;
 
 @interface BoundingBox3Tests : XCTestCase
 
 @end
 
 @implementation BoundingBox3Tests
 */

#include <gtest/gtest.h>

#include "standard_cyborg/sc3d/BoundingBox3.hpp"

#include "standard_cyborg/sc3d/Polyline.hpp"
#include "standard_cyborg/sc3d/Geometry.hpp"


namespace math = standard_cyborg::math;
using math::Vec3;

using standard_cyborg::sc3d::BoundingBox3;


TEST(BoundingBox3Tests, testEmptyConstructor) {
    BoundingBox3 bbox {};
    EXPECT_EQ(bbox.lower, Vec3({INFINITY, INFINITY, INFINITY}));
    EXPECT_EQ(bbox.upper, Vec3({-INFINITY, -INFINITY, -INFINITY}));
}

TEST(BoundingBox3Tests, testVec3Initialization) {
    BoundingBox3 bbox {Vec3{0.0f, 1.0f, 2.0f}, Vec3{2.0f, 3.0f, 4.0f}};
    EXPECT_EQ(bbox.lower, Vec3({0.0f, 1.0f, 2.0f}));
    EXPECT_EQ(bbox.upper, Vec3({2.0f, 3.0f, 4.0f}));
    
}

TEST(BoundingBox3Tests, testFloatInitialization) {
    BoundingBox3 bbox {0.0f, 1.0f, 2.0f, 2.0f, 3.0f, 4.0f};
    EXPECT_EQ(bbox.lower, Vec3({0.0f, 1.0f, 2.0f}));
    EXPECT_EQ(bbox.upper, Vec3({2.0f, 3.0f, 4.0f}));
}

TEST(BoundingBox3Tests, testCenter) {
    BoundingBox3 bbox {Vec3{-1.0f}, Vec3{3.0f}};
    EXPECT_EQ(bbox.center(), Vec3(1.0f));
}

TEST(BoundingBox3Tests, testExtent) {
    BoundingBox3 bbox {Vec3{-1.0f}, Vec3{3.0f}};
    EXPECT_EQ(bbox.extent(), Vec3(2.0f));
}

TEST(BoundingBox3Tests, testShape) {
    BoundingBox3 bbox {Vec3{-1.0f}, Vec3{3.0f}};
    EXPECT_EQ(bbox.shape(), Vec3(4.0f));
}

TEST(BoundingBox3Tests, testRadius) {
    BoundingBox3 bbox {Vec3{0.0f}, Vec3{6.0f, 8.0f, 0.0f}};
    EXPECT_EQ(bbox.radius(), 5.0f);
}

TEST(BoundingBox3Tests, testSquaredRadius) {
    BoundingBox3 bbox {{0.0f}, {6.0f, 8.0f, 0.0f}};
    EXPECT_EQ(bbox.squaredRadius(), 25.0f);
}

TEST(BoundingBox3Tests, testEquality) {
    BoundingBox3 a {Vec3{0.0f}, Vec3{1.0f}};
    BoundingBox3 b {Vec3{0.0f}, Vec3{1.0f}};
    EXPECT_TRUE(a == b);
}

TEST(BoundingBox3Tests, testInequality) {
    BoundingBox3 a {Vec3{0.0f}, Vec3{2.0f}};
    BoundingBox3 b {Vec3{0.0f}, Vec3{1.0f}};
    EXPECT_TRUE(a != b);
}

TEST(BoundingBox3Tests, testApproximateEquality) {
    BoundingBox3 a {Vec3{0.0f}, Vec3{1.0f}};
    BoundingBox3 b {Vec3{0.0f}, Vec3{1.0f + FLT_EPSILON}};
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(BoundingBox3::almostEqual(a, b));
}

TEST(BoundingBox3Tests, testInitializationFromGeometry) {
    BoundingBox3 bbox {standard_cyborg::sc3d::Geometry({Vec3{1, 4, 3}, Vec3{2, 2, 4}})};
    BoundingBox3 expected {{1.0f, 2.0f, 3.0f}, {2.0f, 4.0f, 4.0f}};
    EXPECT_EQ(bbox, expected);
    
    {
        Vec3 lower{+1.0f, +2.0f, +3.0f};
        
        Vec3 upper{+2.0f, +4.0f, +4.0f};
        
        float tx = 4.0f;
        float ty = 2.0f;
        float tz = 9.0f;
        
        math::Mat3x4 m(
                       1,0,0,4,
                       0,1,0,2,
                       0,0,1,9
                       );
        
        BoundingBox3 aabb {standard_cyborg::sc3d::Geometry({lower, upper}), m};
        
        EXPECT_EQ(aabb.getLower(), lower + Vec3(tx, ty, tz));
        EXPECT_EQ(aabb.getUpper(), upper +  Vec3(tx, ty, tz));
    }
}

TEST(BoundingBox3Tests, testInitializationFromVec3Vector) {
    BoundingBox3 bbox {std::vector<Vec3>{{1, 4, 3}, {2, 2, 4}}};
    BoundingBox3 expected {{1.0f, 2.0f, 3.0f}, {2.0f, 4.0f, 4.0f}};
    EXPECT_EQ(bbox, expected);
    
    {
        Vec3 lower{+1.0f, +2.0f, +3.0f};
        
        Vec3 upper{+2.0f, +4.0f, +4.0f};
        
        float tx = 4.0f;
        float ty = 2.0f;
        float tz = 9.0f;
        
        math::Mat3x4 m(
                       1,0,0,4,
                       0,1,0,2,
                       0,0,1,9
                       );
        
        BoundingBox3 aabb {std::vector<Vec3>{lower, upper}, m};
        
        EXPECT_EQ(aabb.getLower(), lower + Vec3(tx, ty, tz));
        EXPECT_EQ(aabb.getUpper(), upper +  Vec3(tx, ty, tz));
    }
}

TEST(BoundingBox3Tests, testCombination) {
    {
        BoundingBox3 aabb0 {{+1.0f, +2.0f, +3.0f}, {2.0f, 4.0f, 4.0f}};
        BoundingBox3 aabb1 {{-1.0f, -2.0f, -3.0f}, {4.0f, 6.0f, 7.0f}};
        BoundingBox3 c = BoundingBox3::combination(aabb0, aabb1);
        
        BoundingBox3 expected {{-1.0f, -2.0f, -3.0f}, {4.0f, 6.0f, 7.0f}};
        
        EXPECT_EQ(c, expected);
    }
    
    {
        BoundingBox3 aabb0 {{+1.0f, +2.0f, +3.0f}, {2.0f, 4.0f, 4.0f}};
        BoundingBox3 aabb1 {{+3.0f, +7.0f, +8.0f}, {10.0f, 11.0f, 12.0f}};
        BoundingBox3 c = BoundingBox3::combination(aabb0, aabb1);
        
        BoundingBox3 expected {{+1.0f, +2.0f, +3.0f}, {10.0f, 11.0f, 12.0f}};
        
        EXPECT_EQ(c, expected);
    }
}

TEST(BoundingBox3Tests, testCombine) {
    {
        BoundingBox3 aabb0 {{+1.0f, +2.0f, +3.0f}, {2.0f, 4.0f, 4.0f}};
        BoundingBox3 aabb1 {{-1.0f, -2.0f, -3.0f}, {4.0f, 6.0f, 7.0f}};
        aabb0.combine(aabb1);
        
        BoundingBox3 expected {{-1.0f, -2.0f, -3.0f}, {4.0f, 6.0f, 7.0f}};
        
        EXPECT_EQ(aabb0, expected);
    }
    
    {
        BoundingBox3 aabb0 {{+1.0f, +2.0f, +3.0f}, {2.0f, 4.0f, 4.0f}};
        BoundingBox3 aabb1 {{+3.0f, +7.0f, +8.0f}, {10.0f, 11.0f, 12.0f}};
        aabb0.combine(aabb1);
        
        BoundingBox3 expected {{+1.0f, +2.0f, +3.0f}, {10.0f, 11.0f, 12.0f}};
        
        EXPECT_EQ(aabb0, expected);
    }
}

TEST(BoundingBox3Tests, testGetLowerGetUpper)
{
    BoundingBox3 aabb {{+1.0f, +2.0f, +3.0f}, {+2.0f, +4.0f, +4.0f}};
    
    Vec3 lower{+1.0f, +2.0f, +3.0f};
    Vec3 upper{+2.0f, +4.0f, +4.0f};
    
    EXPECT_EQ(aabb.getLower(), lower);
    EXPECT_EQ(aabb.getUpper(), upper);
}

TEST(BoundingBox3Tests, testPolylineConstructor)
{
    {
        Vec3 lower{+1.0f, +2.0f, +3.0f};
        
        Vec3 upper{+2.0f, +4.0f, +4.0f};
        
        standard_cyborg::sc3d::Polyline polyline{ std::vector<Vec3>{lower, Vec3{+1.5f, +3.0f, +3.5f}, upper} };
        
        BoundingBox3 aabb {polyline};
        
        EXPECT_EQ(aabb.getLower(), lower);
        EXPECT_EQ(aabb.getUpper(), upper);
    }
    
    {
        Vec3 lower{+1.0f, +2.0f, +3.0f};
        
        Vec3 upper{+2.0f, +4.0f, +4.0f};
        
        float tx = 4.0f;
        float ty = 2.0f;
        float tz = 9.0f;
        
        math::Mat3x4 m(
                       1,0,0,4,
                       0,1,0,2,
                       0,0,1,9
                       );
        
        standard_cyborg::sc3d::Polyline polyline{ std::vector<Vec3>{lower, Vec3{+1.5f, +3.0f, +3.5f}, upper} };
        
        BoundingBox3 aabb {polyline, m};
        
        EXPECT_EQ(aabb.getLower(), lower + Vec3(tx, ty, tz));
        EXPECT_EQ(aabb.getUpper(), upper +  Vec3(tx, ty, tz));
    }
    
}
