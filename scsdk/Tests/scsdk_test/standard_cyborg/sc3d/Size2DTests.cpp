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

#include <standard_cyborg/sc3d/Size2D.hpp>

using namespace standard_cyborg::sc3d;

TEST_CASE("VertexSelectionTests.testConstructor") {
    Size2D s;
    CHECK_EQ(s.width, 0.0f);
    CHECK_EQ(s.height, 0.0f);
}

TEST_CASE("VertexSelectionTests.testAccessor") {
    Size2D s {1, 2};
    CHECK_EQ(s[0], 1);
    CHECK_EQ(s[1], 2);
}
