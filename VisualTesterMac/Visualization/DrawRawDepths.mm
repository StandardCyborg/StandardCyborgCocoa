//  DrawRawDepths.mm
//
//  Created by Aaron Thompson on 6/26/23.
//

#import "DrawRawDepths.hpp"

#import <QuartzCore/QuartzCore.h>

NS_ASSUME_NONNULL_BEGIN

@implementation DrawRawDepths {
    id<MTLDevice> _device;
    id<MTLCommandQueue> _commandQueue;
    id<MTLComputePipelineState> _pipelineState;
}

- (instancetype)initWithDevice:(id<MTLDevice>)device
                  commandQueue:(id<MTLCommandQueue>)commandQueue
                       library:(id<MTLLibrary>)library
{
    self = [super init];
    if (self) {
        _device = device;
        _commandQueue = commandQueue;
        
        id<MTLFunction> computeFunction = [library newFunctionWithName:@"DrawRawDepths"];
        computeFunction.label = @"DrawRawDepths";
        
        NSError *error;
        _pipelineState = [device newComputePipelineStateWithFunction:computeFunction error:&error];
        if (_pipelineState == nil) { NSLog(@"Unable to create pipeline state: %@", error); }
    }
    return self;
}

- (void)draw:(const std::shared_ptr<RawFrame>)rawFrame into:(id<CAMetalDrawable>)drawable
{
    if (rawFrame == nullptr || rawFrame->width == 0) { return; }
    
    id<MTLBuffer> mapBuffer = [_device newBufferWithBytes:(void *)rawFrame->depths.data()
                                                   length:rawFrame->depths.size() * sizeof(float)
                                                  options:MTLResourceOptionCPUCacheModeWriteCombined];
    id<MTLTexture> texture = [drawable texture];
    
    NSUInteger width = [texture width];
    NSUInteger height = [texture height];
    MTLSize threadgroupCounts = MTLSizeMake(8, 8, 1);
    MTLSize threadgroups = MTLSizeMake(width / threadgroupCounts.width,
                                       height / threadgroupCounts.height,
                                       1);
    
    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    commandBuffer.label = @"DrawRawDepths.commandBuffer";
    
    id<MTLComputeCommandEncoder> encoder = [commandBuffer computeCommandEncoder];
    encoder.label = @"DrawRawDepths.encoder";
    [encoder setComputePipelineState:_pipelineState];
    [encoder setBuffer:mapBuffer offset:0 atIndex:0];
    [encoder setTexture:texture atIndex:0];
    [encoder dispatchThreadgroups:threadgroups threadsPerThreadgroup:threadgroupCounts];
    [encoder endEncoding];
    
    [commandBuffer presentDrawable:drawable];
    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];
}

@end

NS_ASSUME_NONNULL_END
