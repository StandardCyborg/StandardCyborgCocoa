//
//  SCOfflineReconstructionManager.h
//  StandardCyborgFusion
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <CoreVideo/CoreVideo.h>
#import <simd/simd.h>
#import <StandardCyborgFusion/PBFFinalStatistics.h>
#import <StandardCyborgFusion/SCAssimilatedFrameMetadata.h>
#import <StandardCyborgFusion/SCReconstructionManagerParameters.h>

#ifdef __cplusplus
#import <StandardCyborgFusion/ICP.hpp>
#import <StandardCyborgFusion/PBFAssimilatedFrameMetadata.hpp>
#import <StandardCyborgFusion/RawFrame.hpp>
#import <StandardCyborgFusion/Surfel.hpp>
#endif

@class AVCameraCalibrationData;
@class CMDeviceMotion;
@class SCPointCloud;
@protocol SCOfflineReconstructionManagerDelegate;

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

@property (nonatomic, weak, nullable) id<SCOfflineReconstructionManagerDelegate> delegate;

#ifdef __cplusplus
/** The most recently processed rawFrame */
- (std::shared_ptr<RawFrame>)lastRawFrame;

- (const Surfels&)surfels;
- (const std::vector<uint32_t>&)surfelIndexMap;
- (id<MTLTexture>)surfelIndexMapTexture;
- (std::unique_ptr<RawFrame>)readBPLYWithPath:(NSString *)filePath;
- (SCAssimilatedFrameMetadata)accumulateFromRawFrame:(const RawFrame&)rawFrame;
- (const std::vector<PBFAssimilatedFrameMetadata>)assimilatedFrameMetadata;
#endif

@end


@protocol SCOfflineReconstructionManagerDelegate <NSObject>
@optional

#ifdef __cplusplus
- (void)reconstructionManager:(SCOfflineReconstructionManager *)manager didIterateICPWithResult:(ICPResult)result;
#endif

@end

NS_ASSUME_NONNULL_END
