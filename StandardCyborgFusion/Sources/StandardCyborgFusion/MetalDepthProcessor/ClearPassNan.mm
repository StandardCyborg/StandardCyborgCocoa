//
//  ClearPassNan.mm
//  StandardCyborgFusion
//
//  Created by eric on 9/13/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <cmath>

#import "ClearPassNan.hpp"

@implementation ClearPassNan

- (void)encodeCommandsWithDevice:(id<MTLDevice>)device
                   commandBuffer:(id<MTLCommandBuffer>)commandBuffer
                     intoTexture:(id<MTLTexture>)texture
                    depthTexture:(id<MTLTexture>)depthTexture
{
    double nan = std::nan("0");
    
    MTLRenderPassDescriptor *passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    
    passDescriptor.colorAttachments[0].texture = texture;
    passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(nan, nan, nan, nan);
    passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    
    passDescriptor.depthAttachment.texture = depthTexture;
    passDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
    passDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
    passDescriptor.depthAttachment.clearDepth = 1.0;
    
    id<MTLRenderCommandEncoder> commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
    commandEncoder.label = @"ClearPassNan.commandEncoder";
    
    [commandEncoder endEncoding];
}

@end
