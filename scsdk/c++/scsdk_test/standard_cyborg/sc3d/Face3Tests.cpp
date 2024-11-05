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
 
 #include <StandardCyborgData/Face3.hpp>
 
 using namespace StandardCyborg;
 
 @interface Face3Tests : XCTestCase
 
 @end
 
 @implementation Face3Tests
 */

#include <gtest/gtest.h>

#include "standard_cyborg/sc3d/Face3.hpp"

using namespace standard_cyborg::sc3d;

TEST(Face3Tests, testGetIndex)
{
    EXPECT_EQ(Face3(1, 2, 3).getIndex(2), 3);
    EXPECT_EQ(Face3(1, 2, 3).getIndex(-3), -1);
    EXPECT_EQ(Face3(1, 2, 3).getIndex(+3), -1);
}

TEST(Face3Tests, testDefaultConstructors)
{
    Face3 d;
    EXPECT_EQ(d[0], -1);
    EXPECT_EQ(d[1], -1);
    EXPECT_EQ(d[2], -1);
}

TEST(Face3Tests, testOffsetOf)
{
    Face3 d(1, 2, 3);
    EXPECT_EQ(d.offsetOf(1), 0);
    EXPECT_EQ(d.offsetOf(2), 1);
    EXPECT_EQ(d.offsetOf(3), 2);
    EXPECT_EQ(d.offsetOf(10), -1);
}

TEST(Face3Tests, testNextVertexIndex)
{
    Face3 d(1, 2, 10);
    
    EXPECT_EQ(d.nextVertexIndex(1), 2);
    EXPECT_EQ(d.nextVertexIndex(2), 10);
    EXPECT_EQ(d.nextVertexIndex(10), 1);
}

TEST(Face3Tests, testPreviousVertexIndex)
{
    Face3 d(1, 2, 10);
    
    EXPECT_EQ(d.previousVertexIndex(1), 10);
    EXPECT_EQ(d.previousVertexIndex(2), 1);
    EXPECT_EQ(d.previousVertexIndex(10), 2);
}
