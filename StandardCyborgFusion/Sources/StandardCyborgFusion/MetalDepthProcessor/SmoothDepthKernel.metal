//
//  SmoothDepthKernel.metal
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 8/2/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#include <metal_stdlib>
using namespace metal;

struct Uniforms {
    int minValidNeighborsBeforeBleed;
    float edgeDiffusion;
    float depthThreshold; // meters
    
    uint frameWidth;
    uint frameHeight;
};

inline uint getBufferIndex (uint2 uv, uint2 threadsPerGrid) {
    uint2 clamped = clamp(uv, uint2(0, 0), threadsPerGrid - 1);
    return clamped.y * threadsPerGrid.x + clamped.x;
}

kernel void smoothDepth(depth2d<float, access::read> depthTexture [[texture(0)]],
                        texture2d<float, access::write> outTexture [[texture(1)]],
                        device float *inputConfidencesBuffer [[buffer(0)]],
                        device float *inputConfidencesResultBuffer [[buffer(1)]],
                        constant Uniforms *uniforms [[buffer(2)]],
                        uint2 gid [[thread_position_in_grid]])
{
    uint2 frameSize(uniforms->frameWidth, uniforms->frameHeight);
    
    uint2 gid0  = gid;
    uint2 gidE  = gid + uint2( 1,  0);
    uint2 gidW  = gid + uint2(-1,  0);
    uint2 gidN  = gid + uint2( 0,  1);
    uint2 gidS  = gid + uint2( 0, -1);
    uint2 gidNE = gid + uint2( 1,  1);
    uint2 gidNW = gid + uint2(-1,  1);
    uint2 gidSE = gid + uint2( 1, -1);
    uint2 gidSW = gid + uint2(-1, -1);
    
    float confidence0  = inputConfidencesBuffer[getBufferIndex(gid0,  frameSize)];
    float confidenceE  = inputConfidencesBuffer[getBufferIndex(gidE,  frameSize)];
    float confidenceW  = inputConfidencesBuffer[getBufferIndex(gidW,  frameSize)];
    float confidenceN  = inputConfidencesBuffer[getBufferIndex(gidN,  frameSize)];
    float confidenceS  = inputConfidencesBuffer[getBufferIndex(gidS,  frameSize)];
    float confidenceNE = inputConfidencesBuffer[getBufferIndex(gidNE, frameSize)];
    float confidenceNW = inputConfidencesBuffer[getBufferIndex(gidNW, frameSize)];
    float confidenceSE = inputConfidencesBuffer[getBufferIndex(gidSE, frameSize)];
    float confidenceSW = inputConfidencesBuffer[getBufferIndex(gidSW, frameSize)];
    
    float depth0  = depthTexture.read(gid0);
    float depthE  = depthTexture.read(gidE);
    float depthW  = depthTexture.read(gidW);
    float depthN  = depthTexture.read(gidN);
    float depthS  = depthTexture.read(gidS);
    float depthNE = depthTexture.read(gidNE);
    float depthNW = depthTexture.read(gidNW);
    float depthSE = depthTexture.read(gidSE);
    float depthSW = depthTexture.read(gidSW);
    
    float2 averageDepthConfidence = float2(0);
    float count = 0.0;
    bool bad = depth0 < 0.0;
    
    if (!bad) {
        averageDepthConfidence += float2(depth0, confidence0) * 4.0;
        count += 4.0;
    }
    
    if ((!bad && abs(depth0 - depthE) < uniforms->depthThreshold) || (bad && depthE > 0.0)) {
        averageDepthConfidence += float2(depthE, confidenceE) * 2.0;
        count += 2.0;
    }
    if ((!bad && abs(depth0 - depthW) < uniforms->depthThreshold) || (bad && depthW > 0.0)) {
        averageDepthConfidence += float2(depthW, confidenceW) * 2.0;
        count += 2.0;
    }
    if ((!bad && abs(depth0 - depthN) < uniforms->depthThreshold) || (bad && depthN > 0.0)) {
        averageDepthConfidence += float2(depthN, confidenceN) * 2.0;
        count += 2.0;
    }
    if ((!bad && abs(depth0 - depthS) < uniforms->depthThreshold) || (bad && depthS > 0.0)) {
        averageDepthConfidence += float2(depthS, confidenceS) * 2.0;
        count += 2.0;
    }
    if ((!bad && abs(depth0 - depthNE) < uniforms->depthThreshold) || (bad && depthNE > 0.0)) {
        averageDepthConfidence += float2(depthNE, confidenceNE);
        count += 1.0;
    }
    if ((!bad && abs(depth0 - depthNW) < uniforms->depthThreshold) || (bad && depthNW > 0.0)) {
        averageDepthConfidence += float2(depthNW, confidenceNW);
        count += 1.0;
    }
    if ((!bad && abs(depth0 - depthSE) < uniforms->depthThreshold) || (bad && depthSE > 0.0)) {
        averageDepthConfidence += float2(depthSE, confidenceSE);
        count += 1.0;
    }
    if ((!bad && abs(depth0 - depthSW) < uniforms->depthThreshold) || (bad && depthSW > 0.0)) {
        averageDepthConfidence += float2(depthSW, confidenceSW);
        count += 1.0;
    }
    if (bad && count < uniforms->minValidNeighborsBeforeBleed) {
        averageDepthConfidence = float2(-1.0, 0.0);
        count = 1.0;
    }
     
    float2 newValue = averageDepthConfidence / count;
    newValue.y = mix(confidence0, newValue.y, uniforms->edgeDiffusion);
    
    outTexture.write(float4(newValue.x, 0, 0, 0), gid);
    inputConfidencesResultBuffer[getBufferIndex(gid, frameSize)] = newValue.y;
}
