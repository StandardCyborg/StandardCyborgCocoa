//
//  ComputeNormalsKernel.metal
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 8/2/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#include <metal_stdlib>
#include "ApplyLensCalibration.metal"
using namespace metal;

struct Uniforms
{
    float2 opticalImageSize, opticalImageCenter, resolution;
    float maxLensCalibrationRadius;
    float4 lensDistortionConstants;
    float4x4 viewMatrixInverse;
    float3x3 intrinsicMatrixInv;
    
    uint frameWidth;
    uint frameHeight;
    
};

static inline float4 unproject(float2 xy, float depth, float3x3 intrinsicMatrixInv, float4x4 viewMatrixInverse)
{
    return viewMatrixInverse * float4(-depth * intrinsicMatrixInv * float3(xy, 1.0), 1.0);
}

kernel void computeNormals(texture2d<float, access::read> depthTexture [[texture(0)]],
                           constant Uniforms *uniforms [[buffer(0)]],
                           device packed_float4 *normalsOutBuffer [[buffer(1)]],
                           device float *surfelSizesOutBuffer [[buffer(2)]],
                           uint2 gid [[thread_position_in_grid]])
{
    uint bufferIndex = gid.y * uniforms->frameWidth + gid.x;
    
    float depthN = depthTexture.read(gid + uint2( 0,  1)).r;
    float depthW = depthTexture.read(gid + uint2(-1,  0)).r;
    float depth0 = depthTexture.read(gid).r;
    float depthE = depthTexture.read(gid + uint2( 1,  0)).r;
    float depthS = depthTexture.read(gid + uint2( 0, -1)).r;
    
    float2 uv = (float2)gid * uniforms->resolution;
    float2 uvE = uv + float2(uniforms->resolution.x, 0.0);
    float2 uvW = uv - float2(uniforms->resolution.x, 0.0);
    float2 uvN = uv + float2(0.0, uniforms->resolution.y);
    float2 uvS = uv - float2(0.0, uniforms->resolution.y);
    
    // Note: we don't apply lens calibration to the positions because it
    // wouldn't make any appreciable difference in the resulting normals
    float2 xy0 = float2(uv.x,  1.0 - uv.y)  * uniforms->opticalImageSize;
    float2 xyE = float2(uvE.x, 1.0 - uvE.y) * uniforms->opticalImageSize;
    float2 xyW = float2(uvW.x, 1.0 - uvW.y) * uniforms->opticalImageSize;
    float2 xyN = float2(uvN.x, 1.0 - uvN.y) * uniforms->opticalImageSize;
    float2 xyS = float2(uvS.x, 1.0 - uvS.y) * uniforms->opticalImageSize;
    
    float3 xyz0 = unproject(xy0, depth0, uniforms->intrinsicMatrixInv, uniforms->viewMatrixInverse).xyz;
    float3 xyzE = unproject(xyE, depthE, uniforms->intrinsicMatrixInv, uniforms->viewMatrixInverse).xyz;
    float3 xyzW = unproject(xyW, depthW, uniforms->intrinsicMatrixInv, uniforms->viewMatrixInverse).xyz;
    float3 xyzN = unproject(xyN, depthN, uniforms->intrinsicMatrixInv, uniforms->viewMatrixInverse).xyz;
    float3 xyzS = unproject(xyS, depthS, uniforms->intrinsicMatrixInv, uniforms->viewMatrixInverse).xyz;
    
    float3 normal = normalize(cross(xyzN - xyzS, xyzE - xyzW));
    
    // Scale the normal based on a few factors
    float growth = min(3.0, length(xyz0) / abs(dot(xyz0, normal)));
    float pixelSizeAtDepth = depth0 * uniforms->resolution.x;
    float surfelSize = 0.707 * pixelSizeAtDepth * growth;
    
    normalsOutBuffer[bufferIndex] = float4(normal, 0.0);
    surfelSizesOutBuffer[bufferIndex] = surfelSize;
    
}
