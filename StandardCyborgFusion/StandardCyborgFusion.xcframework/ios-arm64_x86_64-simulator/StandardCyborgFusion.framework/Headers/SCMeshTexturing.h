//
//  SCMeshTexturing.h
//  StandardCyborgFusion
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <CoreVideo/CoreVideo.h>
#import <Foundation/Foundation.h>
#import <simd/simd.h>

#import <StandardCyborgFusion/SCMesh.h>

@class AVCameraCalibrationData;
@class SCPointCloud;

NS_ASSUME_NONNULL_BEGIN

extern NSString * const SCMeshTexturingAPIErrorDomain;

/**
 Error codes within SCMeshingAPIErrorDomain
 */
typedef NS_ENUM(NSUInteger, SCMeshTexturingAPIError) {
    SCMeshTexturingAPIErrorInvalidAPIKey = 1,
    SCMeshTexturingAPIErrorExceededMeshingCountLimit = 2,
    SCMeshTexturingAPIErrorInternal = 3, // An internal error occurred
    SCMeshTexturingAPIErrorArgument = 4 // Invalid arguments were passed in
};

/**
 Use this class during and at the end of scanning to generate a textured mesh from a SCPointCloud.
 */
@interface SCMeshTexturing : NSObject

/**
 @discussion
  Camera Calibration data that must be specified before running -reconstructMeshWithWithPointCloud:.
  This can be obtained from SCReconstructionManager.
 */
@property (nonatomic) AVCameraCalibrationData *cameraCalibrationData;
@property (nonatomic) NSInteger cameraCalibrationFrameWidth;
@property (nonatomic) NSInteger cameraCalibrationFrameHeight;

/** Flips the color input horizontally, such as when using with a mirror bracket */
@property (nonatomic) BOOL flipsInputHorizontally;

/**
 For typical usage in a scanning session, use this default init method.
 */
- (instancetype)init;

/**
 To load a reconstruction session previously saved via `saveReconstructionSessionDataToDirectoryPath`.
 */
- (nullable instancetype)initWithReconstructionSessionDataDirectory:(NSString *)path;

/**
 Saves off all data internally used for a reconstruction session.
 Useful for running mesh texturing later on with different parameters.
 */
- (void)saveReconstructionSessionDataToDirectory:(NSString *)path;

/**
 @discussion
    While scanning, you may periodically pass in color buffers, along with the viewMatrix
    and projectionMatrix provided by SCReconstructionManager's delegate method, to save
    these frames to be used for texture projection, after the scanning is finished
    Saving only every fifth frame usually yields good results.
 */
- (void)saveColorBufferForReconstruction:(CVPixelBufferRef)colorBuffer
                          withViewMatrix:(simd_float4x4)viewMatrix
                        projectionMatrix:(simd_float4x4)projectionMatrix
    NS_SWIFT_NAME(saveColorBufferForReconstruction(_:withViewMatrix:projectionMatrix:));

/**
 @discussion
    After finalizing a scanning session, call this with the resulting point cloud to
    reconstruct a mesh from the given point cloud and the previously saved color buffers.
 @param textureResolution In pixels, the width or height (square) of the resulting texture
 @param pointCloud Which we apply the texture projection onto
 @param progress Provides progress as the algorithm goes on, from 0.0 to 1.0.
                 Called on an arbitrary thread; callers may want to dispatch to the main queue to update UI.
                 To cancel the operation, set the pointee of `stop` to YES
 @param completion Returns either a non-null NSError in case of error, or a non-null
                   SCMesh instance with the textured projected mesh
 
 */
- (void)reconstructMeshWithWithPointCloud:(SCPointCloud *)pointCloud
                        textureResolution:(NSInteger)textureResolution
                        meshingParameters:(SCMeshingParameters *)meshingParameters
                                 progress:(void (^)(float progress, BOOL *shouldStop))progress
                               completion:(void (^)(NSError * _Nullable, SCMesh * _Nullable))completion
NS_SWIFT_NAME(reconstructMesh(pointCloud:textureResolution:meshingParameters:progress:completion:));

/**
 Call this to reset the internal state for a new scan after performing or abandoning a reconstruction.
 */
- (void)reset;

@end

NS_ASSUME_NONNULL_END
