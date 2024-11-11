//
//  ComputePointsKernel.metal
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 8/29/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#include <metal_stdlib>
#include "ApplyLensCalibration.metal"
using namespace metal;

struct Uniforms
{
    float2 opticalImageSize;
    float2 opticalImageCenter;
    float maxLensCalibrationRadius;
    float4 lensDistortionConstants;
    float4x4 viewMatrixInverse;
    float3x3 intrinsicMatrixInv;
    uint frameWidth;
    uint frameHeight;
    float2 frameSizeFloat;
};

kernel void computePoints(texture2d<float, access::read> depthTexture [[texture(0)]],
                          constant Uniforms *uniforms [[buffer(0)]],
                          device packed_float4 *pointsOutBuffer [[buffer(1)]],
                          uint2 gid [[thread_position_in_grid]])
{
    uint bufferIndex = gid.y * uniforms->frameWidth + gid.x;
    float depth = depthTexture.read(gid).r;
    
    // Input is a index position. We convert it to (texel-centered) NDC coordinates
    // in the range [-1, 1] x [-1, 1]
    float2 xy = (float2(gid) + 0.5) / uniforms->frameSizeFloat * 2.0 - 1.0;
    
    // Apply the lens distortion calibration
    xy = applyLensCalibration(xy, uniforms->opticalImageSize, uniforms->opticalImageCenter, uniforms->maxLensCalibrationRadius, uniforms->lensDistortionConstants);
    
    // Convert back to reference dimension coordinates
    xy = (0.5 + 0.5 * xy) * uniforms->opticalImageSize;
    // Equivalent to:
    //xy = (0.5 + 0.5 * xy) * uniforms->opticalImageSize;
    
    // Flip upside down
    xy.y = uniforms->opticalImageSize.y - xy.y;
    
    float3 unprojected = -depth * uniforms->intrinsicMatrixInv * float3(xy, 1);

    float4 unprojected2 = uniforms->viewMatrixInverse * float4(unprojected, 1);
    
    // Perspective division:
    unprojected2.xyz /= unprojected2.w;
    
    pointsOutBuffer[bufferIndex] = float4(unprojected2.xyz, 0.0);
}
