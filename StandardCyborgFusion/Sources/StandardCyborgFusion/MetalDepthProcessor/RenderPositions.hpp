//
//  RenderPositions.hpp
//  VisualTesterMac
//
//  Created by Eric on 8/31/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <standard_cyborg/sc3d/Geometry.hpp>

using namespace standard_cyborg;

@interface RenderPositions : NSObject

- (instancetype)initWithDevice:(id<MTLDevice>)device
                       library:(id<MTLLibrary>)library;

- (void)encodeCommandsWithDevice:(id<MTLDevice>)device
                   commandBuffer:(id<MTLCommandBuffer>)commandBuffer
                    triangleMesh:(const sc3d::Geometry&)triangleMesh
                     intoTexture:(id<MTLTexture>)texture;

@end
