//
//  SCFaceLandmarking3D.h
//  StandardCyborgFusion
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <CoreVideo/CoreVideo.h>
#import <Foundation/Foundation.h>
#import <simd/simd.h>

@class SCLandmark3D;
@class SCPointCloud;
@protocol SCLandmarking2D;

NS_ASSUME_NONNULL_BEGIN

@interface SCFaceLandmarking3D : NSObject

/** A legend mapping landmark indices to their names */
+ (NSDictionary<NSNumber *, NSString *> *)landmarkNamesByIndex;

- (instancetype)init NS_UNAVAILABLE;

/**
 All model URLs are local file URLs to mlmodelc directories provided by Standard Cyborg.
 Please contact sdk@standardcyborg.com for these models.
 */
- (instancetype)initWithFaceTrackingModelURL:(NSURL *)faceTrackingModelURL
                         landmarkingModelURL:(NSURL *)faceLandmarkingModelURL;

/**
 Use this variant for highly accurate ear landmarks.
 All model URLs are local file URLs to mlmodelc directories provided by Standard Cyborg.
 Please contact sdk@standardcyborg.com for these models.
*/
- (instancetype)initWithFaceTrackingModelURL:(NSURL *)faceTrackingModelURL
                     faceLandmarkingModelURL:(NSURL *)faceLandmarkingModelURL
                         earTrackingModelURL:(NSURL * _Nullable)earTrackingModelURL
                      earLandmarkingModelURL:(NSURL * _Nullable)earLandmarkingModelURL;

- (instancetype)initWithLandmarkProvider:(id<SCLandmarking2D>)landmarkProvider;

/**
 @discussion
    While scanning, you may periodically pass in color buffers, along with the viewMatrix
    and projectionMatrix provided by SCReconstructionManager's delegate method, to save
    these frames for later analysis once the scan is finished.
    One frame for every two degrees of rotation of the scanned subject is usually sufficient.
*/
- (void)saveColorBufferForAnalysis:(CVPixelBufferRef)colorBuffer
                    withViewMatrix:(simd_float4x4)viewMatrix
                  projectionMatrix:(simd_float4x4)projectionMatrix
NS_SWIFT_NAME(saveColorBufferForAnalysis(_:withViewMatrix:projectionMatrix:));

/**
 @discussion
    After finalizing a scanning session, call this with the resulting point cloud to analyze
    the color buffers passed in to saveColorBufferForAnalysis:::
 @param completion A dictionary of landmarks, where keys are NSNumbers per landmark index. Called on the main queue.
*/
- (void)landmarkPointCloud:(SCPointCloud *)pointCloud
                completion:(void (^)(NSSet<SCLandmark3D *> *))completion;

/**
 Builds diagnostic information to send to Standard Cyborg for analysis
 
 @param pointCloud  The same point cloud you passed in to landmarkPointCloud:completion:
 @param completion  Provides the URL to a zip file containing all diagnostic info needed. Called on the main queue.
 @discussion Call this after `landmarkPointCloud:completion`, but before `reset`
 
 Below is an example, in Swift, of how to build diagnostic info and show a share sheet
 to easily copy it off the device for Standard Cyborg to analyze.
 
 ```
 faceLandmarking.gatherDiagnosticInfo { diagnosticInfoURL in
     let shareSheet = UIActivityViewController(activityItems: [diagnosticInfoURL], applicationActivities: nil)
     present(shareSheet, animated: true, completion: nil)
 }
 ```
 */
- (void)gatherDiagnosticInfoWithPointCloud:(SCPointCloud *)pointCloud
                                completion:(void (^)(NSURL *))completion;

- (void)reset;

@end

NS_ASSUME_NONNULL_END
