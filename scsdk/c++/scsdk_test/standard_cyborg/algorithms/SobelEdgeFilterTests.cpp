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

#include "standard_cyborg/algorithms/SobelEdgeFilter.hpp"

#include <algorithm>

#include "standard_cyborg/util/AssertHelper.hpp"
#include "standard_cyborg/sc3d/ColorImage.hpp"
#include "standard_cyborg/sc3d/DepthImage.hpp"

#include "standard_cyborg/math/Vec2.hpp"
#include "standard_cyborg/math/Vec4.hpp"


using standard_cyborg::sc3d::ColorImage;
using standard_cyborg::sc3d::DepthImage;

using standard_cyborg::algorithms::SobelEdgeFilter;

TEST(SobelEdgeFilterTests, testSobelEdgeFilterDepthImage) {
    DepthImage src (3, 3, {
        1, 2, 3,
        4, 5, 6,
        7, 8, 9,
    });
    
    DepthImage dst;
    dst.resetSize(3, 3);
    
    float tmp;
    {
        SobelEdgeFilter(dst, src, 1.0f);
        
        float gx = (-1)*1 + (-2)*4 + (-1)*7 + (+1)*3 + (+2)*6 + (+1)*9;
        float gy = (-1)*1 + (-2)*2 + (-1)*3 + (+1)*7 + (+2)*8 + (+1)*9;
        float g = sqrt(gx * gx + gy * gy);
        tmp = g;
        
        EXPECT_EQ(dst.getPixelAtColRow(1, 1), g);
    }
    
    // now threshold it to zero:
    {
        SobelEdgeFilter(dst, src, tmp + 1.0f);
        EXPECT_EQ(dst.getPixelAtColRow(1, 1), 0.0f);
    }
    
    // try sampling at corner:
    {
        SobelEdgeFilter(dst, src, 1.0f);
        
        float gx = (-1)*1 + (-2)*1 + (-1)*4 +   (+1)*2 + (+2)*2 + (+1)*5;
        float gy = (-1)*1 + (-2)*1 + (-1)*2 + (+1)*4 + (+2)*4 + (+1)*5;
        float g = sqrt(gx * gx + gy * gy);
        
        EXPECT_EQ(dst.getPixelAtColRow(0, 0), g);
    }
}


TEST(SobelEdgeFilterTests, testSobelEdgeFilterColorImage) {
    ColorImage src (3, 3, {
        {0,0,0,1}, {1,1,1,1}, {1,1,1,1},
        {1,1,1,1}, {1,1,1,1}, {1,1,1,1},
        {1,1,1,1}, {1,1,1,1}, {1,1,1,1}
    });
    
    ColorImage dst;
    dst.resetSize(3, 3);
    
    float tmp;
    {
        SobelEdgeFilter(dst, src, 0.01f);
        
        float gx = (-1)*0 + (-2)*1 + (-1)*1 + (+1)*1 + (+2)*1 + (+1)*1;
        float gy = (-1)*0 + (-2)*1 + (-1)*1 + (+1)*1 + (+2)*1 + (+1)*1;
        float g = sqrt(gx * gx + gy * gy);
        tmp = g;
        
        EXPECT_EQ(dst.getPixelAtColRow(1, 1).x, g);
        EXPECT_EQ(dst.getPixelAtColRow(1, 1).y, g);
        EXPECT_EQ(dst.getPixelAtColRow(1, 1).z, g);
        EXPECT_EQ(dst.getPixelAtColRow(1, 1).w, 1.0f);
        
    }
    
    // now threshold it to zero:
    {
        SobelEdgeFilter(dst, src, tmp + 1.0f);
        EXPECT_EQ(dst.getPixelAtColRow(1, 1).x, 0.0f);
        EXPECT_EQ(dst.getPixelAtColRow(1, 1).y, 0.0f);
        EXPECT_EQ(dst.getPixelAtColRow(1, 1).z, 0.0f);
        EXPECT_EQ(dst.getPixelAtColRow(1, 1).w, 1.0f);
    }
    
    // try sampling at corner:
    {
        SobelEdgeFilter(dst, src, 1.0f);
        
        float gx = (-1)*0 + (-2)*0 + (-1)*1 +   (+1)*1 + (+2)*1 + (+1)*1;
        float gy = (-1)*0 + (-2)*0 + (-1)*1 + (+1)*1 + (+2)*1 + (+1)*1;
        float g = sqrt(gx * gx + gy * gy);
        
        EXPECT_EQ(dst.getPixelAtColRow(0, 0).x, g);
    }
}
