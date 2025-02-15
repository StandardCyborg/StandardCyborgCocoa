//
//  ComputeNormalsKernel.h
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 8/2/18.
//

#import <Metal/Metal.h>
#import "ProcessedFrame.hpp"
#import <standard_cyborg/sc3d/PerspectiveCamera.hpp>
#import "MetalComputeEngine.h"

using namespace standard_cyborg;

NS_ASSUME_NONNULL_BEGIN

@interface ComputeNormalsKernel : NSObject <MetalComputeKernel>

- (instancetype)initWithDevice:(id<MTLDevice>)device
                       library:(id<MTLLibrary>)library;

- (void)setPerspectiveCamera:(const sc3d::PerspectiveCamera&)camera
                  frameWidth:(size_t)width
                 frameHeight:(size_t)height;

@end

NS_ASSUME_NONNULL_END
