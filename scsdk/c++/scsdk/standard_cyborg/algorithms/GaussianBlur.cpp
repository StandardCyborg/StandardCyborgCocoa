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
#include "standard_cyborg/algorithms/GaussianBlur.hpp"

#include "standard_cyborg/sc3d/ColorImage.hpp"
#include "standard_cyborg/sc3d/DepthImage.hpp"

using standard_cyborg::sc3d::ColorImage;
using standard_cyborg::sc3d::DepthImage;

namespace standard_cyborg {

namespace algorithms {

void computeGaussianBlurKernel(std::vector<float>& kernelOut, float radius)
{
    int iRadius = std::max(1, static_cast<int>(std::ceil(radius * 3.0)));
    
    kernelOut.resize(iRadius + 1);
    float coeff = 1.0f / std::sqrt(M_2_PI) / radius;
    float sum = 0.0f;
    
    for (int i = 0; i <= iRadius; i++) {
        float ifloat = static_cast<float>(i);
        kernelOut[i] = coeff * std::exp(-ifloat * ifloat * 0.5 / (radius * radius));
        sum += kernelOut[i];
        if (i > 0) {
            sum += kernelOut[i];
        }
    }
    
    for (int i = 0; i <= iRadius; i++) {
        kernelOut[i] /= sum;
    }
}

void GaussianBlur(ColorImage& output, const ColorImage& input, float radius)
{
    SCASSERT(output.getWidth() == input.getWidth(), "Output image width must match input image width");
    SCASSERT(output.getHeight() == input.getHeight(), "Output image height must match input image height");
    
    GaussianBlur(output.getData(), input.getData(), output.getWidth(), output.getHeight(), radius);
}

void GaussianBlur(DepthImage& output, const DepthImage& input, float radius)
{
    SCASSERT(output.getWidth() == input.getWidth(), "Output image width must match input image width");
    SCASSERT(output.getHeight() == input.getHeight(), "Output image height must match input image height");
    
    GaussianBlur(output.getData(), input.getData(), output.getWidth(), output.getHeight(), radius);
}

}

} // namespace StandardCyborg
