//
//  ClearPass.m
//  VisualTesterMac
//
//  Created by Ricky Reusser on 9/13/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ClearPass.hpp"

@implementation ClearPass {
}

- (instancetype)initWithDevice:(nonnull id<MTLDevice>)device library:(nonnull id<MTLLibrary>)library {
    self = [super init];
    return self;
}

- (void)encodeCommandsWithDevice:(id<MTLDevice>)device
                   commandBuffer:(id<MTLCommandBuffer>)commandBuffer
                         surfels:(const std::vector<Surfel> &)surfels
                       icpResult:(ICPResult&)icpResult
                      viewMatrix:(matrix_float4x4)viewlMatrix
                projectionMatrix:(matrix_float4x4)projectionlMatrix
                     intoTexture:(id<MTLTexture>)texture
                    depthTexture:(id<MTLTexture>)depthTexture
{
    MTLRenderPassDescriptor *passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    passDescriptor.colorAttachments[0].texture = texture;
    passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.8, 0.8, 0.8, 1);
    passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    passDescriptor.depthAttachment.texture = depthTexture;
    passDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
    passDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
    passDescriptor.depthAttachment.clearDepth = 1.0;
    
    id<MTLRenderCommandEncoder> commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
    commandEncoder.label = @"ClearPass.commandEncoder";
    
    [commandEncoder endEncoding];
}

@end
