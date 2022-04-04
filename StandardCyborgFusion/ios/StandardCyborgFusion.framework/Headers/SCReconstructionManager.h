//
//  SCReconstructionManager.h
//  StandardCyborgFusion
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <CoreVideo/CoreVideo.h>
#import <Foundation/Foundation.h>
#import <simd/simd.h>

#if !TARGET_OS_OSX

#import <StandardCyborgFusion/SCAssimilatedFrameMetadata.h>
#import <StandardCyborgFusion/SCReconstructionManagerParameters.h>

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

/**
 Performs 3D reconstruction by assimilating color, depth, and IMU data.
 All calls are threadsafe.
 */
@interface SCReconstructionManager : NSObject <SCReconstructionManagerParameters>

/** Calls to delegate are guaranteeed to be called on the main thread */
@property (nonatomic, weak) id<SCReconstructionManagerDelegate> delegate;

/** Continually updated while reconstruction is in process */
@property (nonatomic, readonly) SCReconstructionManagerStatistics currentStatistics;

/** The gravity vector of the current scan. */
@property (nonatomic, readonly) simd_float3 gravity;

/** When YES, the input depth frames are flipped horizontally before assimilating or reconstructing single frames. Useful when scanning using a mirror. */
@property (nonatomic) BOOL flipsInputHorizontally;

/** When YES, assimilated frame metadata will be provided with the depth buffer for the frame that was assimilated. Uses more RAM. */
@property (nonatomic) BOOL includesDepthBuffersInMetadata;

/** When YES, assimilated frame metadata will be provided with the color buffer for the frame that was assimilated. Uses more RAM. */
@property (nonatomic) BOOL includesColorBuffersInMetadata;

/** When non-empty, clips the reconstruction region of incoming depth buffers and frames to this position and size, each normalized from [0..1] */
@property (nonatomic) CGRect normalizedFrameClipRegion;

/** The camera calibration data used by the most recently passed depth frame. */
@property (nonatomic, readonly) AVCameraCalibrationData *latestCameraCalibrationData;
@property (nonatomic, readonly) NSInteger latestCameraCalibrationFrameWidth;
@property (nonatomic, readonly) NSInteger latestCameraCalibrationFrameHeight;

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

/** Sets a manual clipping distance in meters. */
- (void)setMaxDepth:(float)maxDepth;

/** Resets manual clipping distance back to center-weighted strategy */
- (void)clearMaxDepth;

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

#endif // !TARGET_OS_OSX
