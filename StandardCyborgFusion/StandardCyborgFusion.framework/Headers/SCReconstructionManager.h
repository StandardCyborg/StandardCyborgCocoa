//
//  SCReconstructionManager.h
//  StandardCyborgFusion
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <CoreVideo/CoreVideo.h>
#import <Foundation/Foundation.h>
#import <simd/simd.h>

#import <StandardCyborgFusion/SCReconstructionManagerParameters.h>
#import <StandardCyborgFusion/SCAssimilatedFrameMetadata.h>

@class AVCameraCalibrationData;
@class CMDeviceMotion;
@protocol MTLCommandQueue;
@protocol MTLDevice;
@class SCPointCloud;
@protocol SCReconstructionManagerDelegate;

NS_ASSUME_NONNULL_BEGIN

/**
 Cumulative statistics reported as the 3D reconstruction progresses
 */
typedef struct {
    NSInteger succeededCount;
    NSInteger lostTrackingCount;
    NSInteger consecutiveLostTrackingCount;
    NSInteger droppedFrameCount;
} SCReconstructionManagerStatistics;


extern NSString * const SCReconstructionManagerAPIErrorDomain;

/**
 Error codes within SCReconstructionManagerAPIErrorDomain
 These are only reported for Standard Cyborg API usage
 */
typedef NS_ENUM(NSUInteger, SCReconstructionManagerAPIError) {
    SCReconstructionManagerAPIErrorInvalidAPIKey = 1,
    SCReconstructionManagerAPIErrorExceededScanCountLimit = 2
};

/**
 Performs 3D reconstruction by assimilating color, depth, and IMU data.
 All calls are threadsafe.
 */
@interface SCReconstructionManager : NSObject <SCReconstructionManagerParameters>

/** Calls to delegate are guaranteeed to be called on the main thread */
@property (nonatomic, weak) id<SCReconstructionManagerDelegate> delegate;

/** Continually updated while reconstruction is in process */
@property (nonatomic, readonly) SCReconstructionManagerStatistics currentStatistics;


- (instancetype)init NS_UNAVAILABLE;
- (instancetype)initWithDevice:(id<MTLDevice>)device
                  commandQueue:(id<MTLCommandQueue>)commandQueue
                maxThreadCount:(int)maxThreadCount;

/** Useful for un-projecting a single depth frame into a 3D point cloud,
    such as for live pre-scan visualization */
- (SCPointCloud *)reconstructSingleDepthBuffer:(CVPixelBufferRef)depthBuffer
                                   colorBuffer:(_Nullable CVPixelBufferRef)colorBuffer
                           withCalibrationData:(AVCameraCalibrationData *)calibrationData
                               smoothingPoints:(BOOL)smoothPoints;

/** Pass in synchronized color and depth buffers as fast as they are made available by the system.
    Automatically drops frames if more are accumulated than can be processed in real time. */
- (void)accumulateDepthBuffer:(CVPixelBufferRef)depthBuffer
                  colorBuffer:(CVPixelBufferRef)colorBuffer
              calibrationData:(AVCameraCalibrationData *)calibrationData
NS_SWIFT_NAME(accumulate(depthBuffer:colorBuffer:calibrationData:));

/** Pass in device motion updates as fast as they are made available by the system */
- (void)accumulateDeviceMotion:(CMDeviceMotion *)deviceMotion;

/** Call this when finished scanning to perform final cleanup for the scan */
- (void)finalize:(dispatch_block_t)completion;

/** Resets the state of reconstruction, such as for another reconstruction attempt.
    Must call -finalize: first. */
- (void)reset;

/** May be called at any point during or after scanning to build a copy of the currently reconstructed point cloud */
- (SCPointCloud *)buildPointCloud;

@end


@protocol SCReconstructionManagerDelegate

- (void)reconstructionManager:(SCReconstructionManager *)manager
       didProcessWithMetadata:(SCAssimilatedFrameMetadata)metadata
                   statistics:(SCReconstructionManagerStatistics)statistics;

- (void)reconstructionManager:(SCReconstructionManager *)manager
         didEncounterAPIError:(NSError *)error;

@end

NS_ASSUME_NONNULL_END
