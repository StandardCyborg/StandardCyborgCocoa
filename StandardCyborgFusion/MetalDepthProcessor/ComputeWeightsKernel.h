//
//  ComputeWeightsKernel.h
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 8/29/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <Metal/Metal.h>
#import <StandardCyborgFusion/ProcessedFrame.hpp>
#import <standard_cyborg/sc3d/PerspectiveCamera.hpp>
#import "MetalComputeEngine.h"

NS_ASSUME_NONNULL_BEGIN

@interface ComputeWeightsKernel : NSObject <MetalComputeKernel>

- (instancetype)initWithDevice:(id<MTLDevice>)device
                       library:(id<MTLLibrary>)library;

- (void)setPerspectiveCamera:(const sc3d::PerspectiveCamera&)camera
                  frameWidth:(size_t)width
                 frameHeight:(size_t)height;

@end

NS_ASSUME_NONNULL_END
