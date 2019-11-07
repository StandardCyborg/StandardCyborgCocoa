//
//  PointCloudCommandEncoder.metal
//  StandardCyborgFusion
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#include <metal_stdlib>
using namespace metal;

struct Vertex {
    float3 position [[attribute(0)]];
    float3 normal [[attribute(1)]];
    float3 color [[attribute(2)]];
    float weight [[attribute(3)]];
};

struct ProjectedVertex {
    float4 position [[position]];
    float3 color;
    float pointSize [[point_size]];
};

struct Uniforms {
    float3x3 normalMatrix;
    float4x4 modelView;
    float4x4 modelViewProjection;
    float pointSize;
};

#define INV_SQRT_8 0.35355339059327373

float2 computeMatcap(float3 normalizedPositionRelEye, float3 normalizedNormal) {
    float3 reflected = reflect(normalizedPositionRelEye, normalizedNormal);
    return reflected.xy * INV_SQRT_8 * rsqrt(reflected.z + 1.0) + 0.5;
}

vertex ProjectedVertex RenderSCPointCloudVertex(Vertex v [[stage_in]],
                                                constant Uniforms *uniforms [[buffer(1)]],
                                                texture2d<float, access::sample> matcap [[texture(0)]])
{
    constexpr sampler matcapSampler(coord::normalized, address::clamp_to_edge, filter::linear);
    
    // Similar to exponential fade-in, but cheaper than the exp function
    float fadeIn = v.weight / (v.weight + 20.0);
    
    float3 viewPosition = (uniforms->modelView * float4(v.position, 1)).xyz;
    float3 normal = normalize(uniforms->normalMatrix * v.normal);
    float pointSize = (normal.z > 0 && v.weight > 0.0) ? (uniforms->pointSize * fadeIn) : 0;
    
    // Eye vector is implicitly at the origin since we're in view coordinates
    float3 positionRelEye = normalize(viewPosition);
    float2 matcapLookup = computeMatcap(positionRelEye, normal);
    
    float3 matcapColor = matcap.sample(matcapSampler, float2(1.0 - matcapLookup.x, matcapLookup.y)).xyz;
    
    return ProjectedVertex {
        uniforms->modelViewProjection * float4(v.position, 1),
        matcapColor,
        pointSize
    };
}

fragment float4 RenderSCPointCloudFragment(ProjectedVertex inVertex [[stage_in]])
{
    return float4(inVertex.color, 1);
}
