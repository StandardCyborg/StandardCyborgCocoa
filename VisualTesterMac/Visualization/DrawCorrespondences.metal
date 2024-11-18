//
//  DrawCorrespondences.metal
//  VisualTesterMac
//
//  Created by Aaron Thompson on 9/17/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#include <metal_stdlib>
using namespace metal;

struct Vertex {
    float3 position [[attribute(0)]];
    float color [[attribute(1)]];
};

struct ProjectedVertex {
    float4 position [[position]];
    float color;
};

struct Uniforms {
    float4x4 projection;
    float4x4 view;
    float4x4 model;
};

vertex ProjectedVertex DrawCorrespondencesVertex(Vertex v [[stage_in]],
                                                 constant Uniforms *uniforms [[buffer(1)]],
                                                 uint iid [[instance_id]])
{
    ProjectedVertex result;
    
    result.position = uniforms->projection * uniforms->view * uniforms->model * float4(v.position, 1.0);
    result.color = v.color;
    
    return result;
}

fragment float4 DrawCorrespondencesFragment(ProjectedVertex inVertex [[stage_in]])
{
    return float4(0, inVertex.color, 0, 1);
}
