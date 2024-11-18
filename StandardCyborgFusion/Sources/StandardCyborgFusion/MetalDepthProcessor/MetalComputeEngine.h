//
//  MetalComputeEngine.h
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 8/1/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import "MetalDepthProcessorData.hpp"

NS_ASSUME_NONNULL_BEGIN

@protocol MetalComputeKernel

@property (nonatomic, readonly) id<MTLComputePipelineState> pipelineState;

- (void)encodeCommandsWithDevice:(id<MTLDevice>)device
                  commandEncoder:(id<MTLComputeCommandEncoder>)encoder
                    threadgroups:(MTLSize)threadgroups
           threadsPerThreadgroup:(MTLSize)threadsPerThreadgroup
             depthProcessorData:(MetalDepthProcessorData&)depthProcessorData;

@end


@interface MetalComputeEngine : NSObject

@property (nonatomic, readonly) id<MTLDevice> device;
@property (nonatomic, readonly) NSArray<id<MetalComputeKernel>> *kernels;

- (instancetype)initWithDevice:(id<MTLDevice>)device
                  commandQueue:(id<MTLCommandQueue>)commandQueue
                       library:(id<MTLLibrary>)library
                computeKernels:(NSArray<id<MetalComputeKernel>> *)kernels;

- (void)runWithDepthProcessorData:(MetalDepthProcessorData&)depthProcessorData;

@end

NS_ASSUME_NONNULL_END
