//
//  SCPointCloudRenderer.h
//  TrueDepthFusion
//
//  Created by Aaron Thompson on 9/23/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <simd/simd.h>

@class AVCameraCalibrationData;
@class SCPointCloud;

NS_ASSUME_NONNULL_BEGIN

@interface SCPointCloudRenderer : NSObject

- (instancetype)initWithDevice:(id<MTLDevice>)device library:(id<MTLLibrary>)library;

- (void)encodeCommandsOntoBuffer:(id<MTLCommandBuffer>)commandBuffer
                      pointCloud:(SCPointCloud *)pointCloud
      depthCameraCalibrationData:(AVCameraCalibrationData *)depthCameraCalibrationData
                      viewMatrix:(matrix_float4x4)viewMatrix
                   outputTexture:(id<MTLTexture>)outputTexture
                  depthFrameSize:(CGSize)depthFrameSize
          flipsInputHorizontally:(BOOL)flipsInputHorizontally
NS_SWIFT_NAME(encodeCommands(onto:pointCloud:depthCameraCalibrationData:viewMatrix:outputTexture:depthFrameSize:flipsInputHorizontally:));

@end

NS_ASSUME_NONNULL_END
