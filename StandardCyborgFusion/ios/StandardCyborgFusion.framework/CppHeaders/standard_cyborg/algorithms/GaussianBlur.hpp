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
#pragma once

#include "standard_cyborg/util/AssertHelper.hpp"

#include <vector>
#include <algorithm>

#define _USE_MATH_DEFINES
#include <cmath>

namespace standard_cyborg {

namespace sc3d {
class ColorImage;
class DepthImage;
}

namespace algorithms {

void computeGaussianBlurKernel (std::vector<float>& kernelOut, float radius);

template <class T>
void GaussianBlur(std::vector<T>& output, const std::vector<T>& input, int width, int height, float radius);

void GaussianBlur(standard_cyborg::sc3d::ColorImage& output, const standard_cyborg::sc3d::ColorImage& input, float radius);
void GaussianBlur(standard_cyborg::sc3d::DepthImage& output, const standard_cyborg::sc3d::DepthImage& input, float radius);


/* Generic template implementation */

template <class T>
void GaussianBlur(std::vector<T>& output, const std::vector<T>& input, int width, int height, float radius) {
    int dataSize = width * height;
    SCASSERT(input.size() == dataSize, "Input size must match width and height");
    SCASSERT(output.size() == dataSize, "Output size must match width and height");
    
    std::vector<float> kernel;
    computeGaussianBlurKernel(kernel, radius);
    int iRadius = static_cast<int>(kernel.size());

    std::vector<T> tmpData (dataSize);

    // Guassian blur is a separable convolution filter, so we blur first along rows, then along columns
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            int index = row * width + col;
            T sum = kernel[0] * input[index];
            for (int iRow = 1; iRow < iRadius; iRow++) {
                int iRowM1 = std::max(0, row - iRow);
                int iRowP1 = std::min(height - 1, row + iRow);
                sum += kernel[iRow] * (input[iRowM1 * width + col] + input[iRowP1 * width + col]);
            }
            tmpData[index] = sum;
        }
    }
    
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            int index = row * width + col;
            T sum = kernel[0] * tmpData[index];
            for (int iCol = 1; iCol < iRadius; iCol++) {
                int iColM1 = std::max(0, col - iCol);
                int iColP1 = std::min(width - 1, col + iCol);
                sum += kernel[iCol] * (tmpData[row * width + iColM1] + tmpData[row * width + iColP1]);
            }
            output[index] = sum;
        }
    }
}

}

}
