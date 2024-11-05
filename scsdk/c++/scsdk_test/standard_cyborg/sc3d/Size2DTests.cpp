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

#include "standard_cyborg/sc3d/Size2D.hpp"

using namespace standard_cyborg::sc3d;

TEST(VertexSelectionTests, testConstructor) {
    Size2D s;
    EXPECT_EQ(s.width, 0.0f);
    EXPECT_EQ(s.height, 0.0f);
}

TEST(VertexSelectionTests, testAccessor) {
    Size2D s {1, 2};
    EXPECT_EQ(s[0], 1);
    EXPECT_EQ(s[1], 2);
}
