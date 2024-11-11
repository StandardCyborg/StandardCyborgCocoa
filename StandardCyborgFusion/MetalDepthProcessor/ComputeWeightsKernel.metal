//
//  ComputeWeightsKernel.metal
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 8/29/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#include <metal_stdlib>
using namespace metal;

struct Uniforms
{
    uint2 frameSize;
    float2 inverseFrameSize;
    float2 negativeFrameCenter;
};

inline uint clampBufferIndex(uint2 frameSize, uint2 gid)
{
    uint2 clampedGid = clamp(gid, uint2(0, 0), frameSize - uint2(1, 1));
    return clampedGid.y * frameSize.x + clampedGid.x;
}

kernel void computeWeights(device packed_float4 *normalsBuffer [[buffer(0)]],
                           device float *weightsOutBuffer [[buffer(1)]],
                           constant Uniforms *uniforms [[buffer(2)]],
                           uint2 gid [[thread_position_in_grid]])
{
    uint bufferIndex = gid.y * uniforms->frameSize.x + gid.x;
    
    const uint radius = 6;

    uint indexN = clampBufferIndex(uniforms->frameSize, gid + uint2( 0,  radius));
    uint indexS = clampBufferIndex(uniforms->frameSize, gid + uint2( 0, -radius));
    uint indexE = clampBufferIndex(uniforms->frameSize, gid + uint2( radius,  0));
    uint indexW = clampBufferIndex(uniforms->frameSize, gid + uint2(-radius,  0));

    float4 normalN = normalsBuffer[indexN];
    float4 normalS = normalsBuffer[indexS];
    float4 normalE = normalsBuffer[indexE];
    float4 normalW = normalsBuffer[indexW];
    
    float2 offsetFromCenter = float2(gid) + uniforms->negativeFrameCenter;
    offsetFromCenter *= uniforms->inverseFrameSize;
    // Weighting toward the center. A strength of 4.0 leads to zero weight at the corners
    float centerFocus = 1.0 - 4.0 * dot(offsetFromCenter, offsetFromCenter);
    centerFocus = clamp(centerFocus, 0.0, 1.0);
    
    float flatness = 0.5 * (dot(normalN, normalS) + dot(normalE, normalW));
    flatness = flatness * flatness;
    const float featureStrength = 0.8;
    float featureWeighting = clamp(1.0 - featureStrength * flatness, 0.0, 1.0);
    
    weightsOutBuffer[bufferIndex] = centerFocus * featureWeighting;
}
