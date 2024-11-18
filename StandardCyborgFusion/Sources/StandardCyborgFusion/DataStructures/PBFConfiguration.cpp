//
//  PBFConfiguration.cpp
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 12/17/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#include "PBFConfiguration.hpp"

std::ostream& operator<<(std::ostream& os, PBFConfiguration const& config)
{
    os << "PBFConfiguration {\n";
    os << "         icpDownsampleFraction: " << (config.icpDownsampleFraction) << "\n";
    os << "             maxCameraVelocity: " << (config.maxCameraVelocity) << "\n";
    os << "      maxCameraAngularVelocity: " << (config.maxCameraAngularVelocity) << "\n";
    os << "         kdTreeRebuildInterval: " << (config.kdTreeRebuildInterval) << "\n";
    os << "}\n";
    
    return os;
}
