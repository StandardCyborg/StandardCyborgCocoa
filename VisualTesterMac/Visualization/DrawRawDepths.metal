//
//  DrawRawDepths.metal
//  VisualTesterMac
//
//  Created by Aaron Thompson on 10/17/18.
//

#include <metal_stdlib>
using namespace metal;

kernel void DrawRawDepths(device float *depths [[buffer(0)]],
                          texture2d<float, access::write> textureOut [[texture(0)]],
                          uint2 gid [[thread_position_in_grid]],
                          uint2 threadsPerGrid [[threads_per_grid]])
{
    uint bufferIndex = gid.y * threadsPerGrid.x + gid.x;
    float value = depths[bufferIndex];
    
    // Just picking a reasonable value for visualization
    const float kMaxDepth = 1;
    const float normalizedValue = min(value / kMaxDepth, 1.0);
    
    float4 color = { 0, 0, 0, 1 };
    if (value == 9999) {
        color.r = 1;
    } else {
        color.g = sin(normalizedValue * M_PI_F);
        color.b = sin(normalizedValue * M_PI_F + M_PI_2_F);
    }
    
    textureOut.write(color, gid);
}
