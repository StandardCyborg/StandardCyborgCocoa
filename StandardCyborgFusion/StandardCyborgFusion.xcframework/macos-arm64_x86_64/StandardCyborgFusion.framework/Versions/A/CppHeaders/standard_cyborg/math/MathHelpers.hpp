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

#include <algorithm>
#include <cmath>
#include <limits>

namespace standard_cyborg {
namespace math {

inline bool AlmostEqual(
    float a,
    float b,
    float absoluteTolerance = std::numeric_limits<float>::epsilon(),
    float relativeTolerance = std::numeric_limits<float>::epsilon()
) {
    float delta = std::abs(a - b);
    
    if (delta <= absoluteTolerance) return true;
    if (delta <= relativeTolerance * std::min(std::abs(a), std::abs(b))) return true;
    
    return a == b;
}

} // namespace math
} // namespace standard_cyborg
