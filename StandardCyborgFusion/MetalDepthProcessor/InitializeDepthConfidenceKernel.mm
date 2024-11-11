//
//  InitializeConfidenceKernel.mm
//  DepthRenderer
//
//  Created by Aaron Thompson on 7/25/18.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import "InitializeDepthConfidenceKernel.h"

struct InitializeDepthConfidenceKernelUniforms {
    unsigned int frameWidth;
    unsigned int frameHeight;
    
    void setPerspectiveCamera(const sc3d::PerspectiveCamera& camera, size_t width, size_t height)
    {
        frameWidth = (unsigned int)width;
        frameHeight = (unsigned int)height;
    }
};

@implementation InitializeDepthConfidenceKernel {
    InitializeDepthConfidenceKernelUniforms _uniforms;
}

// MARK: - DepthComputeKernel

@synthesize pipelineState = _pipelineState;

- (instancetype)initWithDevice:(id<MTLDevice>)device library:(id<MTLLibrary>)library
{
    self = [super init];
    if (self) {
        id<MTLFunction> kernelFunction = [library newFunctionWithName:@"initializeDepthConfidence"];
        kernelFunction.label = @"initializeDepthConfidence";
        _pipelineState = [device newComputePipelineStateWithFunction:kernelFunction error:NULL];
    }
    return self;
}

- (void)setPerspectiveCamera:(const sc3d::PerspectiveCamera&)camera
                 frameWidth:(size_t)width
                frameHeight:(size_t)height
{
    _uniforms.setPerspectiveCamera(camera, width, height);
}

- (void)encodeCommandsWithDevice:(id<MTLDevice>)device
                  commandEncoder:(id<MTLComputeCommandEncoder>)encoder
                    threadgroups:(MTLSize)threadgroups
           threadsPerThreadgroup:(MTLSize)threadsPerThreadgroup
              depthProcessorData:(MetalDepthProcessorData&)depthProcessorData
{
    [encoder setTexture:depthProcessorData.depthTexture atIndex:0];
    [encoder setBuffer:depthProcessorData.inputConfidencesBuffer offset:0 atIndex:0];
    [encoder setBytes:&_uniforms length:sizeof(InitializeDepthConfidenceKernelUniforms) atIndex:1];
    [encoder dispatchThreads:threadgroups threadsPerThreadgroup:threadsPerThreadgroup];
}

@end
