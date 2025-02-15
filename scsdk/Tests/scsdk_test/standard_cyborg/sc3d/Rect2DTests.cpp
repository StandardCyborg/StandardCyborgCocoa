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

#include <standard_cyborg/sc3d/Rect2D.hpp>

TEST(Rect2DTests, test) {
    standard_cyborg::sc3d::Rect2D p;
    EXPECT_EQ(p.origin.x, 0.0f);
    EXPECT_EQ(p.origin.y, 0.0f);
    EXPECT_EQ(p.size.width, 0.0f);
    EXPECT_EQ(p.size.height, 0.0f);
}
