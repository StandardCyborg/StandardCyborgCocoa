//
//  ComputePointsKernel.mm
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 8/29/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <iostream>
#import <simd/simd.h>
#import <standard_cyborg/util/DebugHelpers.hpp>

#import "ComputePointsKernel.h"
#import "GeometryHelpers.hpp"

using namespace standard_cyborg;

struct ComputePointsKernelUniforms {
    simd_float2 opticalImageSize;
    simd_float2 opticalImageCenter;
    float maxLensCalibrationRadius;
    simd_float4 lensDistortionConstants;
    simd_float4x4 viewMatrixInverse;
    simd_float3x3 intrinsicMatrixInv;
    unsigned int frameWidth;
    unsigned int frameHeight;
    simd_float2 frameSizeFloat;
    
    void setPerspectiveCamera(const sc3d::PerspectiveCamera& camera, size_t width, size_t height)
    {
        opticalImageSize = toSimdFloat2(camera.getIntrinsicMatrixReferenceSize());
        opticalImageCenter = toSimdFloat2(camera.getOpticalImageCenter());
        maxLensCalibrationRadius = camera.getOpticalImageMaxRadius();
        lensDistortionConstants = toSimdFloat4(camera.getLensDistortionCurveFit());
        viewMatrixInverse = toSimdFloat4x4(camera.getViewMatrixInverse());
        intrinsicMatrixInv = toSimdFloat3x3(camera.getIntrinsicMatrixInverse());
        
        frameWidth = (unsigned int)width;
        frameHeight = (unsigned int)height;
        frameSizeFloat = simd_make_float2(width, height);
    }
};

@implementation ComputePointsKernel {
    ComputePointsKernelUniforms _uniforms;
}

- (instancetype)initWithDevice:(id<MTLDevice>)device library:(id<MTLLibrary>)library
{
    self = [super init];
    if (self) {
        id<MTLFunction> kernelFunction = [library newFunctionWithName:@"computePoints"];
        kernelFunction.label = @"computePoints";
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
    [encoder setTexture:_useSmoothedDepth ? depthProcessorData.smoothedDepthTexture : depthProcessorData.depthTexture atIndex:0];
    [encoder setBytes:&_uniforms length:sizeof(ComputePointsKernelUniforms) atIndex:0];
    [encoder setBuffer:depthProcessorData.pointsBuffer offset:0 atIndex:1];
    [encoder dispatchThreads:threadgroups threadsPerThreadgroup:threadsPerThreadgroup];
}

@end
