//
//  SCFaceTracking.h
//  StandardCyborgFusion
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <CoreGraphics/CoreGraphics.h>
#import <CoreVideo/CoreVideo.h>
#import <ImageIO/ImageIO.h>
#import <Foundation/Foundation.h>

@protocol SCFaceTrackingDelegate;
@class VNFaceObservation;

NS_ASSUME_NONNULL_BEGIN


@protocol SCFaceTracking

@property (nonatomic, weak) id<SCFaceTrackingDelegate> delegate;
@property (nonatomic) BOOL detectsFaceLandmarks;

- (void)analyzePixelBuffer:(CVPixelBufferRef)pixelBuffer
               orientation:(CGImagePropertyOrientation)orientation;

@end

@protocol SCFaceTrackingDelegate <NSObject>

/** @param normalizedRect The bounding box of the face, normalized from (0, 0) to (1, 1) */
- (void)faceTracking:(id<SCFaceTracking>)faceTracking
     didDetectFaceAt:(CGRect)normalizedRect
        observations:(NSArray<VNFaceObservation *> *)observations;

@optional

- (void)faceTrackingDidLoseTracking:(id<SCFaceTracking>)faceTracking
NS_SWIFT_NAME(faceTrackingDidLoseTracking(_:));

@end


@interface SCSingleShotFaceTracking : NSObject <SCFaceTracking>
- (instancetype)initWithDelegate:(id<SCFaceTrackingDelegate>)delegate;
@end

@interface SCContinuousFaceTracking : NSObject <SCFaceTracking>
- (instancetype)initWithDelegate:(id<SCFaceTrackingDelegate>)delegate;
@end

/** Efficiently calculates only the bounding box of a face.
    It is robust at detecting faces at high yaw angles. */
@interface SCRobustFaceTracking : NSObject <SCFaceTracking>

- (instancetype)initWithFaceTrackingModelURL:(NSURL *)faceTrackingModelURL
                                    delegate:(id<SCFaceTrackingDelegate>)delegate;

/** The amount of smoothing between frames, from [0..1). 0 is no smoothing, 1 is infinite smoothing. Defaults to 0.5.
    Smoothing reduces the amount of jitter between subsequent calls by applying a moving average. */
@property (nonatomic) float smoothing;

@end

NS_ASSUME_NONNULL_END
