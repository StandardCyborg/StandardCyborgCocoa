//
//  GravityEstimator.cpp
//  StandardCyborgFusion
//
//  Created by Eric Arneback on 2019-03-15.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#include "GravityEstimator.hpp"

GravityEstimator::GravityEstimator() {
    reset();
}

void GravityEstimator::reset() {
    hasGravity = false;
    
    // Default to y-axis-aligned gravity
    gravity = Eigen::Vector3f(0, -1, 0);
}

void GravityEstimator::accumulate(simd_float3 gravitySample, simd_float4 attitudeSample)

{
    if (hasGravity) {
        return; // already has gotten our gravity assigned.
    }
    
    gravity =  Eigen::Vector3f(gravitySample.x, gravitySample.y, gravitySample.z);

    hasGravity = true;
}

simd_float3 GravityEstimator::getGravity() const
{
    if (!hasGravity) {
        return simd_make_float3(-1.0f, 0.0f, 0.0f);
    }
    
    Eigen::Vector3f v = gravity;
    
    return simd_make_float3(v.x(), v.y(), v.z());
}


simd_float3x3 GravityEstimator::getCoordinateFrame() const
{
    simd_float3 y = simd_normalize(simd_make_float3(-gravity.x(), -gravity.y(), -gravity.z()));
    
    // If gravity is almost identically the x axis, then to avoid gimbal lock,
    // use the global z axis as a reference axis instead of x
    simd_float3 referenceAxis = simd_make_float3(1, 0, 0);
    if (std::abs(y.x) > 0.999) {
        referenceAxis = simd_make_float3(0, 0, 1);
    }

    simd_float3 z = simd_normalize(simd_cross(referenceAxis, y));
    simd_float3 x = simd_normalize(simd_cross(y, z));
    
    return simd_transpose(simd_matrix_from_rows(x, y, z));
}
