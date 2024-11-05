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

#include "standard_cyborg/sc3d/Landmark.hpp"

namespace math = standard_cyborg::math;

/*
 
 #import <XCTest/XCTest.h>
 
 
 #include <algorithm>
 #include <vector>
 
 #include <StandardCyborgData/Landmark.hpp>
 
 
 namespace math = StandardCyborg::math;
 */

TEST(LandmarkTests, tesInitialization) {
    
    using math::Vec3;
    
    standard_cyborg::sc3d::Landmark l;
    l.setName("nose");
    l.setPosition(Vec3{1.0f, 2.0f, 3.0f});
    
    EXPECT_TRUE(l.getPosition() == Vec3({1.0f, 2.0f, 3.0f}));
    EXPECT_TRUE(l.getName() == "nose");
    
    EXPECT_TRUE(l == standard_cyborg::sc3d::Landmark({"nose", Vec3({1.0f, 2.0f, 3.0f})}));
    EXPECT_TRUE(l != standard_cyborg::sc3d::Landmark({"nosee", Vec3({1.0f, 2.0f, 3.0f})}));
    EXPECT_TRUE(l != standard_cyborg::sc3d::Landmark({"nose", Vec3({9.0f, 2.0f, 3.0f})}));
    EXPECT_FALSE(l != standard_cyborg::sc3d::Landmark({"nose", Vec3({1.0f, 2.0f, 3.0f})}));
    
}
