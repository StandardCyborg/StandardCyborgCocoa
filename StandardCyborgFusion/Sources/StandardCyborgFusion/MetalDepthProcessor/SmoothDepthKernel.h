//
//  SmoothDepthKernel.h
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 8/1/18.
//

#import <Metal/Metal.h>
#import "ProcessedFrame.hpp"
#import "MetalComputeEngine.h"

NS_ASSUME_NONNULL_BEGIN

@interface SmoothDepthKernel : NSObject <MetalComputeKernel>

- (instancetype)initWithDevice:(id<MTLDevice>)device library:(id<MTLLibrary>)library;

- (void)setPerspectiveCamera:(const sc3d::PerspectiveCamera&)camera
                 frameWidth:(size_t)width
                frameHeight:(size_t)height;

@end

NS_ASSUME_NONNULL_END
