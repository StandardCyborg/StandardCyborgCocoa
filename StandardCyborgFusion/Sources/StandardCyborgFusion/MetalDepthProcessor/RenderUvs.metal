//
//  DrawPointCloud.metal
//  StandardCyborgFusion
//
//  Created by eric on 7/25/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#include <metal_stdlib>
#include "ApplyLensCalibration.metal"

using namespace metal;

struct Vertex {
    float4 position [[attribute(0)]];
    float4 color [[attribute(1)]];
    float4 texCoord [[attribute(2)]];
    float4 normal [[attribute(3)]];
};

struct ProjectedVertex {
    float4 position [[position]];
    float3 color;
    float3 normal;
};

struct Uniforms {
    float4x4 projectionView;
    float4x4 orientationView;
    
    float4 inverseLensDistortionConstants;
    float2 opticalImageSize;
    float2 opticalImageCenter;
    float maxLensCalibrationRadius;
};

vertex ProjectedVertex RenderUvsVertex(const Vertex v [[stage_in]],
                                       constant Uniforms *uniforms [[buffer(1)]],
                                       const device float *right [[buffer(2)]])
{
    float4 projected = uniforms->projectionView * float4(v.position.xyz, 1.0);

    projected /= projected.w;
    
    projected.xy = applyLensCalibration(projected.xy, uniforms->opticalImageSize, uniforms->opticalImageCenter, uniforms->maxLensCalibrationRadius, uniforms->inverseLensDistortionConstants);
   
    return ProjectedVertex{
        projected,
        float3(v.texCoord.x, v.texCoord.y, 0.0f),
        v.normal.xyz
    };
}

fragment float4 RenderUvsFragment(const ProjectedVertex inVertex [[stage_in]],
                                  constant Uniforms *uniforms [[buffer(1)]])
{
    float3 viewDir = float3(-uniforms->orientationView[0][2],
                            -uniforms->orientationView[1][2],
                            -uniforms->orientationView[2][2]);
    
    float3 n = inVertex.normal;
    
    float weight = dot(n, -viewDir);
    
    if (weight < 0.0 || weight > 1.0) {
        return float4(NAN, NAN, NAN, 1);
    }
    
    return float4(inVertex.color, weight);
    
}
