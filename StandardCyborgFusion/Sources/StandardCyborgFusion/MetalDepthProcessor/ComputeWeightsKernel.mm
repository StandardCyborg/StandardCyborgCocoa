//
//  ComputeWeightsKernel.mm
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 8/29/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import "ComputeWeightsKernel.h"
#import <simd/simd.h>

struct ComputeWeightsKernelUniforms {
    simd_uint2 frameSize;
    simd_float2 inverseFrameSize; // { 1 / width, 1 / height }
    simd_float2 negativeFrameCenter; // { -0.5 * width, -0.5 * height }
    
    void setPerspectiveCamera(const sc3d::PerspectiveCamera& camera, size_t width, size_t height)
    {
        frameSize = simd_make_uint2((unsigned int)width,
                                    (unsigned int)height);
        inverseFrameSize = simd_make_float2(1.0 / (float)width,
                                            1.0 / (float)height);
        negativeFrameCenter = simd_make_float2(-0.5 * (float)width,
                                               -0.5 * (float)height);
    }
};

@implementation ComputeWeightsKernel {
    ComputeWeightsKernelUniforms _uniforms;
}

- (instancetype)initWithDevice:(id<MTLDevice>)device library:(id<MTLLibrary>)library
{
    self = [super init];
    if (self) {
        id<MTLFunction> kernelFunction = [library newFunctionWithName:@"computeWeights"];
        kernelFunction.label = @"computeWeights";
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

// MARK: - DepthComputeKernel

@synthesize pipelineState = _pipelineState;

- (void)encodeCommandsWithDevice:(id<MTLDevice>)device
                  commandEncoder:(id<MTLComputeCommandEncoder>)encoder
                    threadgroups:(MTLSize)threadgroups
           threadsPerThreadgroup:(MTLSize)threadsPerThreadgroup
             depthProcessorData:(MetalDepthProcessorData&)depthProcessorData
{
    [encoder setBuffer:depthProcessorData.normalsBuffer offset:0 atIndex:0];
    [encoder setBuffer:depthProcessorData.weightsBuffer offset:0 atIndex:1];
    [encoder setBytes:&_uniforms length:sizeof(ComputeWeightsKernelUniforms) atIndex:2];

    [encoder dispatchThreads:threadgroups threadsPerThreadgroup:threadsPerThreadgroup];
}

@end
