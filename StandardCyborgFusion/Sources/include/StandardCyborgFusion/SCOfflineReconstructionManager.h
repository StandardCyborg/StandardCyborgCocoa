//
//  SCOfflineReconstructionManager.h
//  StandardCyborgFusion
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <CoreVideo/CoreVideo.h>
#import <simd/simd.h>
#import <StandardCyborgFusion/SCAssimilatedFrameMetadata.h>
#import <StandardCyborgFusion/SCReconstructionManagerParameters.h>

#import <StandardCyborgFusion/PBFFinalStatistics.h>

@class AVCameraCalibrationData;
@class CMDeviceMotion;
@class SCPointCloud;
@protocol MTLCommandQueue;
@protocol MTLDevice;

NS_ASSUME_NONNULL_BEGIN

@interface SCOfflineReconstructionManager : NSObject <SCReconstructionManagerParameters>

- (instancetype)init NS_UNAVAILABLE;
- (instancetype)initWithDevice:(id<MTLDevice>)device
                  commandQueue:(id<MTLCommandQueue>)commandQueue
                maxThreadCount:(int)maxThreadCount;

- (void)setMotionDataPath:(NSString *)filePath;
- (SCAssimilatedFrameMetadata)accumulateFromBPLYWithPath:(NSString *)filePath;
- (SCPointCloud *)reconstructRawFrameFromBPLYAtPath:(NSString *)bplyPath;

/** The gravity vector of the current scan. */
@property (nonatomic, readonly) simd_float3 gravity;

/** Coordinate axes aligned with gravity for the current scan */
@property (nonatomic, readonly) simd_float3x3 gravityAlignedAxes;

- (PBFFinalStatistics)finalize;

- (void)reset;

- (SCPointCloud *)buildPointCloud;
- (BOOL)writePointCloudToPLYFile:(NSString *)plyPath;
- (BOOL)writePointCloudToUSDAFile:(NSString *)USDAPath;

@end

NS_ASSUME_NONNULL_END
