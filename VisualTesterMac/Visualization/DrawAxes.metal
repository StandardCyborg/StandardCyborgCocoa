//
//  DrawAxes.metal
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 7/25/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#include <metal_stdlib>
using namespace metal;

struct Vertex {
    float3 position [[attribute(0)]];
    float3 color [[attribute(1)]];
};

struct VertexOutput {
    float4 position [[position]];
    float4 color;
};

struct Uniforms {
    float4x4 projection;
    float4x4 view;
    float4x4 model;
};

vertex VertexOutput DrawAxesVertex(Vertex v [[stage_in]], constant Uniforms &uniforms [[buffer(1)]]) {
    return VertexOutput {
        uniforms.projection * (uniforms.view * uniforms.model) * float4(v.position, 1.0),
        float4(v.color, 0.0)
    };
    
}

fragment float4 DrawAxesFragment(VertexOutput inVertex [[stage_in]]) {
    return inVertex.color;
}

