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

NS_ASSUME_NONNULL_BEGIN

@interface SCFaceLandmarking3D : NSObject

/** A legend mapping landmark indices to their names */
+ (NSDictionary<NSNumber *, NSString *> *)landmarkNamesByIndex;

- (instancetype)init NS_UNAVAILABLE;

/** @param modelURL  The URL to a mlmodelc directory provided by Standard Cyborg.
                     Please contact sdk@standardcyborg.com for this model. */
- (instancetype)initWithModelURL:(NSURL *)modelURL NS_DESIGNATED_INITIALIZER;

/** @discussion
        While scanning, you may periodically pass in color buffers, along with the viewMatrix
        and projectionMatrix provided by SCReconstructionManager's delegate method, to save
        these frames for later analysis once the scan is finished.
        One frame for every two degrees of rotation of the scanned subject is usually sufficient.
*/
- (void)saveColorBufferForAnalysis:(CVPixelBufferRef)colorBuffer
                    withViewMatrix:(simd_float4x4)viewMatrix
                  projectionMatrix:(simd_float4x4)projectionMatrix
NS_SWIFT_NAME(saveColorBufferForAnalysis(_:withViewMatrix:projectionMatrix:));

/** @discussion
        After finalizing a scanning session, call this with the resulting point cloud to analyze
        the color buffers passed in to saveColorBufferForAnalysis:::
    @param completion A dictionary of landmarks, where keys are NSNumbers per landmark index
*/
- (void)landmarkPointCloud:(SCPointCloud *)pointCloud
                completion:(void (^)(NSArray<SCLandmark3D *> *))completion;

- (void)reset;

@end

NS_ASSUME_NONNULL_END
