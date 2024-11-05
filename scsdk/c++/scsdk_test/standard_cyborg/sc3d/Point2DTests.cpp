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

#include "standard_cyborg/sc3d/Point2D.hpp"

using namespace standard_cyborg::sc3d;

TEST(Point2DTests, testConstructor) {
    Point2D p;
    EXPECT_EQ(p.x, 0.0f);
    EXPECT_EQ(p.y, 0.0f);
}

TEST(Point2DTests, testAccessor) {
    Point2D p {1, 2};
    EXPECT_EQ(p[0], 1);
    EXPECT_EQ(p[1], 2);
}
