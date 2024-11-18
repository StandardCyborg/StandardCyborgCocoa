//
//  RenderPositions.metal
//  StandardCyborgFusion
//
//  Created by eric on 7/25/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#include <metal_stdlib>

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
};

vertex ProjectedVertex RenderPositionsVertex(Vertex v [[stage_in]])
{
    float4 projected;
    
    projected.xy = v.texCoord.xy * 2.0 + float2(-1.0, -1.0);
    projected.z = 0.0;
    projected.w = 1.0;
    
    return ProjectedVertex {
        projected,
        float3(v.position.x, v.position.y, v.position.z)
    };
}

fragment float4 RenderPositionsFragment(ProjectedVertex inVertex [[stage_in]])
{
    return float4(inVertex.color, 1);
}
