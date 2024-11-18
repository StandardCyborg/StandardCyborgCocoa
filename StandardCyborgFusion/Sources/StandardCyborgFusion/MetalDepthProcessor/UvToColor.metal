//
//  UvToColor.metal
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 8/2/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#include <metal_stdlib>
using namespace metal;

struct Uniforms {
    float textureResolution;
    uint frameWidth;
    uint frameHeight;
};

kernel void uvToColor(texture2d<float, access::read> uvTexture [[texture(0)]],
                      texture2d<float, access::read> frameTexture [[texture(1)]],
                      texture2d<float, access::read_write> targetTexture [[texture(2)]],
                      
                      constant Uniforms *uniforms [[buffer(0)]],
                      uint2 threadsPerGrid [[threads_per_grid]],
                      uint2 gid [[thread_position_in_grid]])
{
    if (gid.x >= uniforms->frameWidth || gid.y >= uniforms->frameHeight) {
        return;
    }
    
    // for each color pixel in the rgb frame, output it to the uv-mapped texture.
    // based on the uvs we just rendered out, to uvData.

    float4 uvSample = uvTexture.read(gid);
    float4 color = frameTexture.read(gid);
    
    float u = uvSample.x;
    float v = uvSample.y;
    float weight = uvSample.w;
    
    if (!isnan(u)) {
        int y = (v * uniforms->textureResolution);
        int x = (u * uniforms->textureResolution);
        
        float4 sample = targetTexture.read(uint2(x, y));
        
        sample += float4(weight * color.xyz, weight);
        
        targetTexture.write(sample, uint2(x, y));
    }
}
