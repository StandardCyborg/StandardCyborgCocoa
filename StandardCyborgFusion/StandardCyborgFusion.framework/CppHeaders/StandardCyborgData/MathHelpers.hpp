//
//  MathHelpers.hpp
//  StandardCyborgSDK
//
//  Created by Ricky Reusser on 5/16/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <algorithm>
#include <cmath>
#include <limits>

namespace StandardCyborg {

inline bool almostEqual(
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

} // namespace StandardCyborg
