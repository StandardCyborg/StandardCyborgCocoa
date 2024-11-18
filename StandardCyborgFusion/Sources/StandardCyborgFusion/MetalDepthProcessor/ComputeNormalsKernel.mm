//
//  ComputeNormalsKernel.mm
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 8/2/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <simd/simd.h>
#import "ComputeNormalsKernel.h"
#import "GeometryHelpers.hpp"
#import "ProcessedFrame.hpp"

using namespace standard_cyborg;

struct ComputeNormalsKernelUniforms {
    simd_float2 opticalImageSize, opticalImageCenter, resolution;
    float maxLensCalibrationRadius;
    simd_float4 lensDistortionConstants;
    simd_float4x4 viewMatrixInverse;
    simd_float3x3 intrinsicMatrixInv;
    
    unsigned int frameWidth;
    unsigned int frameHeight;
    
    void setPerspectiveCamera(const sc3d::PerspectiveCamera& camera, size_t width, size_t height)
    {
        resolution.x = 1.0f / width;
        resolution.y = 1.0f / height;
        
        opticalImageSize = toSimdFloat2(camera.getIntrinsicMatrixReferenceSize());
        opticalImageCenter = toSimdFloat2(camera.getOpticalImageCenter());
        maxLensCalibrationRadius = camera.getOpticalImageMaxRadius();
        lensDistortionConstants = toSimdFloat4(camera.getLensDistortionCurveFit());
        viewMatrixInverse = toSimdFloat4x4(camera.getViewMatrixInverse());
        intrinsicMatrixInv = toSimdFloat3x3(camera.getIntrinsicMatrixInverse());
        
        frameWidth = (unsigned int)width;
        frameHeight = (unsigned int)height;
    }
};

@implementation ComputeNormalsKernel {
    ComputeNormalsKernelUniforms _uniforms;
}

- (instancetype)initWithDevice:(id<MTLDevice>)device
                       library:(id<MTLLibrary>)library
{
    self = [super init];
    if (self) {
        id<MTLFunction> kernelFunction = [library newFunctionWithName:@"computeNormals"];
        kernelFunction.label = @"computeNormals";
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
    id<MTLBuffer> normalsBuffer = depthProcessorData.normalsBuffer;
    id<MTLBuffer> surfelSizesBuffer = depthProcessorData.surfelSizesBuffer;

    [encoder setTexture:depthProcessorData.smoothedDepthTexture atIndex:0];
    [encoder setBytes:&_uniforms length:sizeof(ComputeNormalsKernelUniforms) atIndex:0];
    [encoder setBuffer:normalsBuffer offset:0 atIndex:1];
    [encoder setBuffer:surfelSizesBuffer offset:0 atIndex:2];
    
    [encoder dispatchThreads:threadgroups threadsPerThreadgroup:threadsPerThreadgroup];
}

@end
