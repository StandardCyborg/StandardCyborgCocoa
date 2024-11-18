//
//  MetalVisualizationEngine.mm
//  VisualTesterMac
//
//  Created by Ricky Reusser on 8/31/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#include "MetalVisualizationEngine.hpp"

@implementation MetalVisualizationEngine {
    id<MTLLibrary> _library;
    id<MTLCommandQueue> _commandQueue;
}

- (instancetype)initWithDevice:(id<MTLDevice>)device
                  commandQueue:(id<MTLCommandQueue>)commandQueue
                       library:(id<MTLLibrary>)library
                visualizations:(NSArray *)kernels
{
    self = [super init];
    if (self) {
        _device = device;
        _commandQueue = commandQueue;
        _library = library;
        _kernels = kernels;
    }
    return self;
}

- (void)renderSurfels:(const Surfels&)surfels
            icpResult:(ICPResult&)icpResult
           viewMatrix:(matrix_float4x4)viewMatrix
     projectionMatrix:(matrix_float4x4)projectionMatrix
         intoDrawable:(id<CAMetalDrawable>)drawable
{
    id<MTLTexture> resultTexture = [drawable texture];
    
    MTLTextureDescriptor *depthDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                                                               width:resultTexture.width
                                                                                              height:resultTexture.height
                                                                                           mipmapped:NO];
    depthDescriptor.usage = MTLTextureUsageRenderTarget;
    depthDescriptor.storageMode = MTLStorageModePrivate;
    id<MTLTexture> depthTexture = [_device newTextureWithDescriptor:depthDescriptor];
    depthTexture.label = @"MetalVisualizationEngine.depthTexture";
    
    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    commandBuffer.label = @"MetalVisualizationEngine.commandBuffer";
    
    // Have each kernel encode commands to the command encoder
    for (id<MetalVisualization> kernel in _kernels) {
        [kernel encodeCommandsWithDevice:_device
                           commandBuffer:commandBuffer
                                 surfels:surfels
                               icpResult:icpResult
                              viewMatrix:viewMatrix
                        projectionMatrix:projectionMatrix
                             intoTexture:resultTexture
                            depthTexture:depthTexture];
    }
    
    [commandBuffer presentDrawable:drawable];
    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];
}

@end
