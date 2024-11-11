//
//  PBFConfiguration.hpp
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 12/17/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#pragma once

#include <ostream>
#include <cmath>

struct PBFConfiguration {
    float maxCameraVelocity = 0.6;
    float maxCameraAngularVelocity = 2.5;
    
    float icpDownsampleFraction = 0.05;
    
    int kdTreeRebuildInterval = 6;
};

std::ostream& operator<<(std::ostream& os, PBFConfiguration const& config);
