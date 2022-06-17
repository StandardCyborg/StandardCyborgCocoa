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

namespace standard_cyborg {

namespace sc3d {
class DepthImage;
class ColorImage;
}

namespace algorithms {

/** Locate edges using the Sobel operator. The source and output images must be the same shape and
 * may *not* be the same image instance. To prevent noise that doesn't correspond to edges, the
 * `threshold` parameter is a value below which output values are mapped to zero.
 */
void SobelEdgeFilter(standard_cyborg::sc3d::ColorImage& dst, const standard_cyborg::sc3d::ColorImage& src, float threshold = 0.25f);

void SobelEdgeFilter(standard_cyborg::sc3d::DepthImage& dst, const standard_cyborg::sc3d::DepthImage& src, float threshold = 0.25f);

}

} // namespace StandardCyborg
