//
//  MetalComputeEngine.mm
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 8/1/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import "MetalComputeEngine.h"
#import <Metal/Metal.h>

@implementation MetalComputeEngine {
    id<MTLLibrary> _library;
    id<MTLCommandQueue> _commandQueue;
}

- (instancetype)initWithDevice:(id<MTLDevice>)device
                  commandQueue:(id<MTLCommandQueue>)commandQueue
                       library:(id<MTLLibrary>)library
                computeKernels:(NSArray *)kernels
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

- (void)runWithDepthProcessorData:(MetalDepthProcessorData&)depthProcessorData
{
    NSUInteger width = [depthProcessorData.depthTexture width];
    NSUInteger height = [depthProcessorData.depthTexture height];
    
    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    id<MTLComputeCommandEncoder> commandEncoder = [commandBuffer computeCommandEncoder];
    commandBuffer.label = @"MetalComputeEngine.commandBuffer";
    commandEncoder.label = @"MetalComputeEngine.commandEncoder";
    
    // Have each kernel encode commands to the command encoder
    [_kernels enumerateObjectsUsingBlock:^(id<MetalComputeKernel> kernel, NSUInteger i, BOOL *stop) {
        id<MTLComputePipelineState> pipelineState = [kernel pipelineState];
        [commandEncoder setComputePipelineState:pipelineState];
        
        // https://developer.apple.com/documentation/metal/calculating_threadgroup_and_grid_sizes
        MTLSize threadsPerThreadgroup = MTLSizeMake([pipelineState threadExecutionWidth],
                                                    [pipelineState maxTotalThreadsPerThreadgroup] / [pipelineState threadExecutionWidth],
                                                    1);
        MTLSize threadsPerGrid = MTLSizeMake(width, height, 1);
        
        [kernel encodeCommandsWithDevice:_device
                          commandEncoder:commandEncoder
                            threadgroups:threadsPerGrid
                   threadsPerThreadgroup:threadsPerThreadgroup
                      depthProcessorData:depthProcessorData];
    }];
    [commandEncoder endEncoding];
    
    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];
}

@end
