//
//  DrawSurfelIndexMap.metal
//  VisualTesterMac
//
//  Created by Aaron Thompson on 10/17/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#include <metal_stdlib>
using namespace metal;

kernel void DrawSurfelIndexMap(device uint *surfelIndexMapBuffer [[buffer(0)]],
                               texture2d<float, access::write> textureOut [[texture(0)]],
                               uint2 gid [[thread_position_in_grid]],
                               uint2 threadsPerGrid [[threads_per_grid]])
{
    uint bufferIndex = gid.y * threadsPerGrid.x + gid.x;
    uint value = surfelIndexMapBuffer[bufferIndex];
    
    float4 color;
    color.r = float((value >>  0) & 0xff) / 255.0;
    color.g = float((value >>  8) & 0xff) / 255.0;
    color.b = float((value >> 16) & 0xff) / 255.0;
    color.a = 1;
    
    textureOut.write(color, gid);
}
