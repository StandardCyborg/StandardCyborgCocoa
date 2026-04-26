//
//  PBFConfiguration.hpp
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 12/17/18.
//

#pragma once

#import <ostream>
#import <cmath>

struct PBFConfiguration {
    
    float maxCameraVelocity = 6.0;
    float maxCameraAngularVelocity = 20.0;
    float icpDownsampleFraction = 0.05;
    
    int kdTreeRebuildInterval = 6;
};

std::ostream& operator<<(std::ostream& os, PBFConfiguration const& config);
