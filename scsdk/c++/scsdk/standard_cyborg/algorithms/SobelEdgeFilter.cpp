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

#include "standard_cyborg/algorithms/SobelEdgeFilter.hpp"

#include <algorithm>

#include "standard_cyborg/util/AssertHelper.hpp"
#include "standard_cyborg/sc3d/ColorImage.hpp"
#include "standard_cyborg/sc3d/DepthImage.hpp"

using standard_cyborg::sc3d::ColorImage;
using standard_cyborg::sc3d::DepthImage;


namespace standard_cyborg {

namespace algorithms {

void SobelEdgeFilter(ColorImage& dst, const ColorImage& src, float threshold) {
    SCASSERT(dst.getWidth() == src.getWidth(), "Destination image width must match source image width");
    SCASSERT(dst.getHeight() == src.getHeight(), "Destination image height must match source image height");
    SCASSERT(&dst != &src, "Input and output images may not be the same image");
    
    int width = src.getWidth();
    int height = src.getHeight();

    for (int row = 0; row < height; row++) {
        int rowm1 = std::max(0, row - 1);
        int rowp1 = std::min(height - 1, row + 1);
        
        for (int col = 0; col < width; col++) {
            int colm1 = std::max(0, col - 1);
            int colp1 = std::min(width - 1, col + 1);
            
            float l0 = src.getLightness(colm1, rowm1);
            float l1 = src.getLightness(col,   rowm1);
            float l2 = src.getLightness(colp1, rowm1);
            float l3 = src.getLightness(colm1, row);
            float l5 = src.getLightness(colp1, row);
            float l6 = src.getLightness(colm1, rowp1);
            float l7 = src.getLightness(col,   rowp1);
            float l8 = src.getLightness(colp1, rowp1);
            
            float S1 = l2 - l0 + l8 - l6 + 2.0f * (l5 - l3);
            float S2 = l6 - l0 + l8 - l2 + 2.0f * (l7 - l1);
            
            float S = std::sqrt(S1 * S1 + S2 * S2);
            S = S < threshold ? 0.0f : S;
        
            dst.setPixelAtColRow(col, row, math::Vec4{math::Vec3{S}, 1.0f});
        }
    }
}

void SobelEdgeFilter(DepthImage& dst, const DepthImage& src, float threshold) {
    SCASSERT(dst.getWidth() == src.getWidth(), "Destination image width must match source image width");
    SCASSERT(dst.getHeight() == src.getHeight(), "Destination image height must match source image height");
    SCASSERT(&dst != &src, "Input and output images may not be the same image");
    
    int width = src.getWidth();
    int height = src.getHeight();

    for (int row = 0; row < height; row++) {
        int rowm1 = std::max(0, row - 1);
        int rowp1 = std::min(height - 1, row + 1);
        
        for (int col = 0; col < width; col++) {
            int colm1 = std::max(0, col - 1);
            int colp1 = std::min(width - 1, col + 1);
            
            float l0 = src.getPixelAtColRow(colm1, rowm1);
            float l1 = src.getPixelAtColRow(col,   rowm1);
            float l2 = src.getPixelAtColRow(colp1, rowm1);
            float l3 = src.getPixelAtColRow(colm1, row);
            float l5 = src.getPixelAtColRow(colp1, row);
            float l6 = src.getPixelAtColRow(colm1, rowp1);
            float l7 = src.getPixelAtColRow(col,   rowp1);
            float l8 = src.getPixelAtColRow(colp1, rowp1);
            
            float S1 = l2 - l0 + l8 - l6 + 2.0f * (l5 - l3);
            float S2 = l6 - l0 + l8 - l2 + 2.0f * (l7 - l1);
            
            float S = std::sqrt(S1 * S1 + S2 * S2);
            S = S < threshold ? 0.0f : S;
            
            dst.setPixelAtColRow(col, row, S);
        }
    }
}

}

} // namespace StandardCyborg
