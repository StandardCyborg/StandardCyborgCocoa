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

#include <standard_cyborg/math/MathHelpers.hpp>

using namespace standard_cyborg::math;

TEST(MathHelpersTests, testAlmostEquals) {
    EXPECT_FALSE(standard_cyborg::math::AlmostEqual(100.0, 100.0 + 1e-4));
    EXPECT_TRUE(standard_cyborg::math::AlmostEqual(100.0, 100.0 + 1e-5));
    EXPECT_TRUE(standard_cyborg::math::AlmostEqual(100.0, 100.0 + 1e-4, 1e-6, 1e-6));
}
