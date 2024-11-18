//
//  BPLYDepthDataAccumulator.h
//  DepthRenderer
//
//  Created by Aaron Thompson on 7/5/18.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <AVKit/AVKit.h>
#import <CoreMotion/CoreMotion.h>
#import <CoreVideo/CoreVideo.h>
#import <Foundation/Foundation.h>
#import <SceneKit/SceneKit.h>

#import <StandardCyborgFusion/StandardCyborgFusion.h>

NS_ASSUME_NONNULL_BEGIN

@class SCPointCloud;

@interface BPLYDepthDataAccumulator : NSObject

- (instancetype)init;

- (void)accumulateColorBuffer:(CVPixelBufferRef)colorBuffer
                    colorTime:(CMTime)colorTime
                  depthBuffer:(CVPixelBufferRef)depthBuffer
                    depthTime:(CMTime)depthTime
              calibrationData:(AVCameraCalibrationData *)calibrationData
NS_SWIFT_NAME(accumulate(colorBuffer:colorTime:depthBuffer:depthTime:calibrationData:));

- (void)accumulatePointCloud:(SCPointCloud *)pointCloud;

- (NSURL *)exportFrameSequenceToZip;

- (nullable SCPointCloud *)loadNextPointCloud;
- (void)resetNextPointCloud;

- (NSString *)containerPath;

- (void)accumulateDeviceMotion:(CMDeviceMotion *)motion
NS_SWIFT_NAME(accumulate(deviceMotion:));

@end

NS_ASSUME_NONNULL_END
