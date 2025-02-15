//
//  RenderUvs.hpp
//  VisualTesterMac
//
//  Created by Eric on 8/31/18.
//

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
// #import "MetalVisualizationEngine.hpp"

#import <standard_cyborg/sc3d/Geometry.hpp>
#import <standard_cyborg/sc3d/PerspectiveCamera.hpp>

using namespace standard_cyborg;

@interface RenderUvs : NSObject

- (instancetype)initWithDevice:(id<MTLDevice>)device library:(id<MTLLibrary>)library;

- (void)encodeCommandsWithDevice:(id<MTLDevice>)device
                   commandBuffer:(id<MTLCommandBuffer>)commandBuffer
                          camera:(sc3d::PerspectiveCamera)camera
                    triangleMesh:(const sc3d::Geometry&)triangleMesh
                      viewMatrix:(matrix_float4x4)viewMatrix
                projectionMatrix:(matrix_float4x4)projectionMatrix
                     intoTexture:(id<MTLTexture>)texture
                    depthTexture:(id<MTLTexture>)depthTexture;


@end
