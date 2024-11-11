//
//  SurfelIndexMap.metal
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 7/25/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#include <metal_stdlib>
#include "StandardCyborgFusion/DataStructures/Surfel.hpp"
#include "StandardCyborgFusion/MetalDepthProcessor/ApplyLensCalibration.metal"
using namespace metal;

struct Vertex {
    float2 position [[attribute(0)]];
};

struct ProjectedVertex {
    float4 position [[position]];
    uint index;
};

struct Uniforms {
    float4x4 projectionViewMatrix;
    float4 inverseLensDistortionConstants;
    float2 opticalImageSize;
    float2 opticalImageCenter;
    float maxLensCalibrationRadius;
    float surfelAliasingSafetyFactor;
};

struct FragmentOutput {
    uint color0 [[color(0)]];
};

vertex ProjectedVertex SurfelIndexMapVertex(Vertex v [[stage_in]],
                                            constant Uniforms *uniforms [[buffer(1)]],
                                            constant Surfel *surfels [[buffer(2)]],
                                            uint iid [[instance_id]])
{
    Surfel instance = surfels[iid];
    
    float3 normal = instance.normal;
    
    // Assuming we cross the normal with the vector (0, 1, 0), we can save few
    // multiplications by zero and simply use:
    //
    //    |  i   j   k  |
    //    |  nx  ny  nz |  = -i * nz + k * nx
    //    |  0   1   0  |
    //
    // Making this equivalent to the following line
    //
    // float3 tangent = normalize(cross(normal, float3(0, 1, 0)));
    //
    // Add a small offset to basically ensure it's never singular:
    float3 tangent = normalize(float3(-normal.z, 1e-10, normal.x));
    
    // The bitangent is the *other* surface tangent vector.
    float3 bitangent = cross(normal, tangent);
    
    float3 p = instance.position + (v.position.x * tangent + v.position.y * bitangent) * instance.surfelSize * uniforms->surfelAliasingSafetyFactor;
    
    // Apply the lens calibration. This section is not currently translated into the projection matrix
    // style, but it's not *so* important that it's worth fixing up front.
    float4 projected = uniforms->projectionViewMatrix * float4(p, 1.0);
    projected /= projected.w;
    projected.xy = applyLensCalibration(projected.xy, uniforms->opticalImageSize, uniforms->opticalImageCenter, uniforms->maxLensCalibrationRadius, uniforms->inverseLensDistortionConstants);

    return ProjectedVertex {
        projected,
        iid
    };
}

fragment FragmentOutput SurfelIndexMapFragment(ProjectedVertex inVertex [[stage_in]])
{
    return FragmentOutput {
        inVertex.index
    };
}


vertex ProjectedVertex SurfelIndexMapForColorVertex(Vertex v [[stage_in]],
                                                    constant Uniforms *uniforms [[buffer(1)]],
                                                    constant Surfel *surfels [[buffer(2)]],
                                                    uint iid [[instance_id]])
{
    const float SURFEL_ALIASING_SAFETY_FACTOR = 1.3;
    
    Surfel instance = surfels[iid];
    
    float3 normal = instance.normal;
    
    // Assuming we cross the normal with the vector (0, 1, 0), we can save few
    // multiplications by zero and simply use:
    //
    //    |  i   j   k  |
    //    |  nx  ny  nz |  = -i * nz + k * nx
    //    |  0   1   0  |
    //
    // Making this equivalent to the following line
    //
    // float3 tangent = normalize(cross(normal, float3(0, 1, 0)));
    //
    // Add a small offset to basically ensure it's never singular:
    float3 tangent = normalize(float3(-normal.z, 1e-10, normal.x));
    
    // The bitangent is the *other* surface tangent vector.
    float3 bitangent = cross(normal, tangent);
    
    float3 p = instance.position + (v.position.x * tangent + v.position.y * bitangent) * instance.surfelSize * SURFEL_ALIASING_SAFETY_FACTOR;
    
    float4 projected = uniforms->projectionViewMatrix * float4(p, 1.0);
    projected /= projected.w;
    
    return ProjectedVertex {
        projected,
        iid
    };
}
