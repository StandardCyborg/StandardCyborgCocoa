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


#include <vector>

#include <gtest/gtest.h>
#include "standard_cyborg/sc3d/Polyline.hpp"


using standard_cyborg::sc3d::Polyline;
namespace math = standard_cyborg::math;
using math::Vec3;

TEST(PolylineTests, testEmptyConstructor) {
    Polyline p {};
    
    EXPECT_EQ(p.getPositions(), std::vector<Vec3>({}));
}

TEST(PolylineTests, testConstructor) {
    Polyline p {std::vector<Vec3>({
        {1.0f, 2.0f, 3.0f},
        {3.0f, 4.0f, 5.0f}
    })};
    
    EXPECT_EQ(p.getPositions(), std::vector<Vec3>({
        {1.0f, 2.0f, 3.0f},
        {3.0f, 4.0f, 5.0f}
    }));
}

TEST(PolylineTests, testGetNumVertices) {
    Polyline p {std::vector<Vec3>({
        {1.0f, 2.0f, 3.0f},
        {3.0f, 4.0f, 5.0f}
    })};
    
    EXPECT_EQ(p.vertexCount(), 2);
}

TEST(PolylineTests, testCopy) {
    Polyline p {std::vector<Vec3>({
        {1.0f, 2.0f, 3.0f},
        {3.0f, 4.0f, 5.0f}
    })};
    
    Polyline p2 {};
    p2.copy(p);
    
    EXPECT_EQ(p2.getPositions(), std::vector<Vec3>({
        {1.0f, 2.0f, 3.0f},
        {3.0f, 4.0f, 5.0f}
    }));
}

TEST(PolylineTests, testSetPositions) {
    Polyline p {std::vector<Vec3>({})};
    
    p.setPositions(std::vector<Vec3>({
        {1.0f, 2.0f, 3.0f},
        {3.0f, 4.0f, 5.0f}
    }));
    
    EXPECT_EQ(p.getPositions(), std::vector<Vec3>({
        {1.0f, 2.0f, 3.0f},
        {3.0f, 4.0f, 5.0f}
    }));
}

TEST(PolylineTests, testIsClosed) {
    Polyline p {std::vector<Vec3>({})};
    
    p.setPositions(std::vector<Vec3>({
        {1.0f, 2.0f, 3.0f},
        {3.0f, 4.0f, 5.0f},
        {0.9999f, 2.0001f, 3.0001f}
    }));
    
    EXPECT_FALSE(p.isClosed());
    EXPECT_TRUE(p.isClosed(1e-3, 1e-3));
}

TEST(PolylineTests, testLength) {
    Polyline p{std::vector<Vec3>({
        { -1.0f, -1.0f, 0.0f },
        {  1.0f, -1.0f, 0.0f },
        {  1.0f,  1.0f, 0.0f },
        { -1.0f,  1.0f, 0.0f },
        { -1.0f, -1.0f, 0.0f }
    })};
    
    EXPECT_EQ(p.length(), 8.0f);
}
