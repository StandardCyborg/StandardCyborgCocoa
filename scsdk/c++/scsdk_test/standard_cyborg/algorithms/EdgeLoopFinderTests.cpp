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

#include "standard_cyborg/algorithms/EdgeLoopFinder.hpp"

#include "standard_cyborg/math/Vec3.hpp"

#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/sc3d/Face3.hpp"

namespace math = standard_cyborg::math;


TEST(EdgeLoopFinderTests, testFindEdgeLoops) {
    std::vector<math::Vec3> positions {
        {1.0f, 1.0f, 0.0f},
        {3.0f, 1.0f, 0.0f},
        {1.0f, 3.0f, 0.0f},
        
        {1.0f, 1.0f, 3.0f},
        {3.0f, 1.0f, 3.0f},
        {1.0f, 3.0f, 3.0f},
        
        {1.0f, 1.0f, 10.0f},
        {3.0f, 1.0f, 10.0f},
        {1.0f, 3.0f, 10.0f},
        {3.0f, 3.0f, 10.0f},
    };
    
    std::vector<math::Vec3> normals {
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
    };
    
    std::vector<math::Vec3> colors {
        {0.0f, 0.2f, 0.0f},
        {0.0f, 0.2f, 0.0f},
        {0.0f, 0.2f, 0.0f},
        
        {0.0f, 0.2f, 0.0f},
        {0.0f, 0.2f, 0.0f},
        {0.0f, 0.2f, 0.0f},
        
        {0.0f, 0.2f, 0.0f},
        {0.0f, 0.2f, 0.0f},
        {0.0f, 0.2f, 0.0f},
        {0.0f, 0.2f, 0.0f},
    };
    
    std::vector<standard_cyborg::sc3d::Face3> faces {
        { 0, 1, 2 },
        { 3, 4, 5 },
        
        { 6, 7, 8 },
        { 7, 9, 8 },
    };
    
    standard_cyborg::sc3d::Geometry geometry0 (positions, normals, colors, faces);
    
    std::vector<std::vector<std::pair<int,int>>> edgeLoops =  standard_cyborg::algorithms::findEdgeLoops(geometry0);
    
    {
        std::vector<std::pair<int,int> > loop0 = {
            {0, 1},
            {1, 2},
            {2, 0},
        };
        
        EXPECT_TRUE(edgeLoops[0] == loop0);
        
        std::vector<std::pair<int,int> > loop1 = {
            {9, 8},
            {8, 6},
            {6, 7},
            {7, 9},
        };
        
        EXPECT_TRUE(edgeLoops[1] == loop1);
        
        std::vector<std::pair<int,int> > loop2 = {
            {4, 5},
            {5, 3},
            {3, 4},
        };
        
        EXPECT_TRUE(edgeLoops[2] == loop2);
    }
}
