//
//  SmoothDepthKernel.mm
//  DepthRenderer
//
//  Created by Aaron Thompson on 7/25/18.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import "SmoothDepthKernel.h"

using namespace standard_cyborg;

struct SmoothDepthKernelUniforms {
    int minValidNeighborsBeforeBleed = 1;
    float edgeDiffusion = 1.0;
    float depthThreshold = 0.003; // meters

    unsigned int frameWidth;
    unsigned int frameHeight;
    
    void setPerspectiveCamera(const sc3d::PerspectiveCamera& camera, size_t width, size_t height)
    {
        frameWidth = (unsigned int)width;
        frameHeight = (unsigned int)height;
    }
};

static SmoothDepthKernelUniforms __makeUniforms(int minValidNeighborsBeforeBleed) {
    SmoothDepthKernelUniforms result;
    result.minValidNeighborsBeforeBleed = minValidNeighborsBeforeBleed;
    return result;
}

static SmoothDepthKernelUniforms __uniforms[] = {
    __makeUniforms(1),
    __makeUniforms(1),
    __makeUniforms(7),
    __makeUniforms(7)
};
static size_t __uniformsCount = sizeof(__uniforms) / sizeof(SmoothDepthKernelUniforms);


@implementation SmoothDepthKernel

// MARK: - DepthComputeKernel

@synthesize pipelineState = _pipelineState;

- (instancetype)initWithDevice:(id<MTLDevice>)device library:(id<MTLLibrary>)library {
    self = [super init];
    if (self) {
        id<MTLFunction> kernel = [library newFunctionWithName:@"smoothDepth"];
        kernel.label = @"smoothDepth";
        _pipelineState = [device newComputePipelineStateWithFunction:kernel error:NULL];
    }
    return self;
}

- (void)setPerspectiveCamera:(const sc3d::PerspectiveCamera&)camera
                 frameWidth:(size_t)width
                frameHeight:(size_t)height
{
    for (size_t i = 0; i < __uniformsCount; ++i) {
        SmoothDepthKernelUniforms *uniforms = &__uniforms[i];
        uniforms->setPerspectiveCamera(camera, width, height);
    }
}


- (void)encodeCommandsWithDevice:(id<MTLDevice>)device
                  commandEncoder:(id<MTLComputeCommandEncoder>)encoder
                    threadgroups:(MTLSize)threadgroups
           threadsPerThreadgroup:(MTLSize)threadsPerThreadgroup
             depthProcessorData:(MetalDepthProcessorData&)depthProcessorData
{
    // The first pass uses the raw depth as input and the work texture as output. Subsequent iterations of
    // the loop will start with the work smoothed depth texture (the final output) as the input.
    id<MTLTexture> input = depthProcessorData.depthTexture;
    id<MTLTexture> output = depthProcessorData.workTexture;
    
    // These are a little different since the final output destination is the same as the input. That means
    // we can just set them directly without a special case for the input.
    id<MTLBuffer> inputConfidencesBuffer = depthProcessorData.inputConfidencesBuffer;
    id<MTLBuffer> workBuffer = depthProcessorData.workBuffer;

    for (size_t i = 0; i < __uniformsCount; ++i) {
        // Perform two passes with each set of uniforms so that we ping pong back and forth and land on the
        // correct output.
        SmoothDepthKernelUniforms *uniforms = &__uniforms[i];

        [encoder setTexture:input atIndex:0];
        [encoder setTexture:output atIndex:1];
        [encoder setBuffer:inputConfidencesBuffer offset:0 atIndex:0];
        [encoder setBuffer:workBuffer offset:0 atIndex:1];
        [encoder setBytes:uniforms length:sizeof(SmoothDepthKernelUniforms) atIndex:2];
        [encoder dispatchThreads:threadgroups threadsPerThreadgroup:threadsPerThreadgroup];
        
        input = depthProcessorData.workTexture;
        output = depthProcessorData.smoothedDepthTexture;

        [encoder setTexture:input atIndex:0];
        [encoder setTexture:output atIndex:1];
        [encoder setBuffer:workBuffer offset:0 atIndex:0];
        [encoder setBuffer:inputConfidencesBuffer offset:0 atIndex:1];
        [encoder setBytes:uniforms length:sizeof(SmoothDepthKernelUniforms) atIndex:2];
        [encoder dispatchThreads:threadgroups threadsPerThreadgroup:threadsPerThreadgroup];
        
        input = depthProcessorData.smoothedDepthTexture;
        output = depthProcessorData.workTexture;
    }
}

@end
