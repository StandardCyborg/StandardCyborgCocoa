//
//  ApplyLensCalibration.metal
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 9/16/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#include <metal_stdlib>
using namespace metal;

inline float applyLensDistortionCurve(float x, float4 calibration)
{
    return x * x * (calibration.x + x * (calibration.y + x * (calibration.z + x * calibration.w)));
}

inline float2 applyLensCalibration(float2 projected, float2 opticalImageSize, float2 opticalCenter, float maxRadius, float4 lensCalibrationConstants)
{
    // Determine the vector from the optical center to the given point.
    float2 xyRelative = (0.5 + 0.5 * projected) * opticalImageSize - opticalCenter;
    
    // Determine the radius of the given point.
    float radius = min(length(xyRelative), maxRadius);
    
    // Compute the magnification from a curve fit of the lookup table
    float magnification = applyLensDistortionCurve(radius / maxRadius, lensCalibrationConstants);
    
    // Apply the magnification
    return ((opticalCenter + xyRelative * (1.0 + magnification)) / opticalImageSize) * 2.0 - 1.0;
    
}
