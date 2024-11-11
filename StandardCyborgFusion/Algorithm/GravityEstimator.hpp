//
//  GravityEstimator.hpp
//  StandardCyborgFusion
//
//  Created by Eric Arneback on 2019-03-15.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <standard_cyborg/util/IncludeEigen.hpp>
#include <simd/simd.h>

class GravityEstimator {
public:
    GravityEstimator();
    
    void accumulate(simd_float3 gravitySample, simd_float4 attitudeSample);
    
    void reset();
    
    simd_float3 getGravity() const;
    simd_float3x3 getCoordinateFrame() const;
    
private:
    bool hasGravity; // whether we have gotten our first IMU sample, and thus assigned a gravity vector yet.
    Eigen::Vector3f gravity;

    // Prohibit copying and assignment
    GravityEstimator(const GravityEstimator&) = delete;
    GravityEstimator& operator=(const GravityEstimator&) = delete;
    
};
