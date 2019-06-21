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

- (instancetype)initWithDelegate:(id<SCFaceTrackingDelegate>)delegate;

- (void)analyzePixelBuffer:(CVPixelBufferRef)pixelBuffer
               orientation:(CGImagePropertyOrientation)orientation;

@end

@protocol SCFaceTrackingDelegate <NSObject>

- (void)faceTracking:(id<SCFaceTracking>)faceTracking
     didDetectFaceAt:(CGRect)normalizedRect
        observations:(NSArray<VNFaceObservation *> *)observations;

@optional

- (void)faceTrackingDidLoseTracking:(id<SCFaceTracking>)faceTracking
NS_SWIFT_NAME(faceTrackingDidLoseTracking(_:));

@end


@interface SCSingleShotFaceTracking : NSObject <SCFaceTracking>
@end

@interface SCContinuousFaceTracking : NSObject <SCFaceTracking>
@end


NS_ASSUME_NONNULL_END
