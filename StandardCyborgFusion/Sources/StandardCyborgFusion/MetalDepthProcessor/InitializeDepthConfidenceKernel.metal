//
//  InitializeDepthConfidenceKernel.metal
//  DepthRenderer
//
//  Created by Aaron Thompson on 7/23/18.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#include <metal_stdlib>
using namespace metal;

struct Uniforms {
    uint frameWidth;
    uint frameHeight;
};

kernel void initializeDepthConfidence(depth2d<float, access::read> depthTexture [[texture(0)]],
                                      device float *inputConfidencesResultBuffer [[buffer(0)]],
                                      constant Uniforms *uniforms [[buffer(1)]],
                                      uint2 gid [[thread_position_in_grid]])
{
    uint bufferIndex = gid.y * uniforms->frameWidth + gid.x;
    
    float depthN = depthTexture.read(gid + uint2( 0,  1));
    float depthW = depthTexture.read(gid + uint2(-1,  0));
    float depth  = depthTexture.read(gid);
    float depthE = depthTexture.read(gid + uint2( 1,  0));
    float depthS = depthTexture.read(gid + uint2( 0, -1));
    
    // Compute confidence
    float2 range = min(min(min(min(
                       float2(depth,  -depth),
                       float2(depthN, -depthN)),
                       float2(depthS, -depthS)),
                       float2(depthE, -depthE)),
                       float2(depthW, -depthW));
    range.y = -range.y;
    float confidence = smoothstep(0.03, 0.01, range.y - range.x);
    if (isnan(confidence)) { confidence = 0; }
    
    inputConfidencesResultBuffer[bufferIndex] = confidence;
}
