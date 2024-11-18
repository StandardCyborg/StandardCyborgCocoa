//
//  DrawPointCloud.metal
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 7/25/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#include <metal_stdlib>
#include "StandardCyborgFusion/DataStructures/Surfel.hpp"

using namespace metal;

struct Vertex {
    float2 position [[attribute(0)]];
};

struct ProjectedVertex {
    float4 position [[position]];
    float3 color;
};

struct Uniforms {
    float4x4 projection;
    float4x4 view;
    float4x4 model;
    bool colorByNormals;
};

vertex ProjectedVertex DrawPointCloudVertex(Vertex v [[stage_in]],
                                            constant Uniforms *uniforms [[buffer(1)]],
                                            constant Surfel *surfels [[buffer(2)]],
                                            uint iid [[instance_id]])
{
    const float SURFEL_ALIASING_SAFETY_FACTOR = 1.0;
    
    Surfel instance = surfels[iid];
    
    float3 normal = instance.normal;
    float3 tangent = normalize(cross(normal, float3(0, 1, 0)));
    float3 bitangent = normalize(cross(normal, tangent));
    float scale = instance.surfelSize * SURFEL_ALIASING_SAFETY_FACTOR * 1.25;
    
    float3 position = instance.position + (v.position.x * tangent + v.position.y * bitangent) * scale;
    
    float4 projected = uniforms->projection * uniforms->view * uniforms->model * float4(position, 1.0);
    
    // To color by normals:
    float3 color;
    if (uniforms->colorByNormals) {
        color = 0.5 + 0.5 * normalize(normal);
    } else {
        color = pow(instance.color, 1.0 / 2.2);
    }

    return ProjectedVertex {
        projected,
        color,
    };
}

fragment float4 DrawPointCloudFragment(ProjectedVertex inVertex [[stage_in]])
{
    return float4(inVertex.color, 1);
}
