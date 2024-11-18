//
//  SCEarTracking.h
//  StandardCyborgFusion
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class CIImage;
@class SCEarTracking;

@protocol SCEarTrackingDelegate <NSObject>
/**
 @param normalizedRect  The bounding box of the detected ear, normalized from (0, 0) to (1, 1)
 @param confidence  The confidence of the detected ear, from 0-1
 */
- (void)earTracking:(SCEarTracking *)earTracking
     didDetectEarAt:(CGRect)normalizedRect
         confidence:(CGFloat)confidence;

@optional
- (void)earTrackingDidLoseTracking:(SCEarTracking *)earTracking
NS_SWIFT_NAME(earTrackingDidLoseTracking(_:));

@end


/** Efficiently calculates the bounding box of a ear. */
@interface SCEarTracking : NSObject

@property (nonatomic, weak, nullable) id<SCEarTrackingDelegate> delegate;

/** The amount of smoothing between frames, from [0..1). 0 is no smoothing, 1 is infinite smoothing. Defaults to 0.5.
    Smoothing reduces the amount of jitter between subsequent calls by applying a moving average. */
@property (nonatomic) float smoothing;

/**
@param modelURL Local file URL to the SCEarTracking.mlmodelc directory provided by Standard Cyborg.
                Please contact sdk@standardcyborg.com for these models.
*/
- (instancetype)initWithModelURL:(NSURL *)modelURL
                        delegate:(id<SCEarTrackingDelegate> _Nullable)delegate;

- (instancetype)init NS_UNAVAILABLE;

- (void)analyzePixelBuffer:(CVPixelBufferRef)pixelBuffer orientation:(CGImagePropertyOrientation)orientation;

/**
 @param boundingBoxOut  The bounding box of the detected ear, normalized from 0-1
 @param confidenceOut  The confidence of the detected ear, normalized from 0-1
 */
- (BOOL)synchronousAnalyzeImage:(CIImage *)image
                    orientation:(CGImagePropertyOrientation)orientation
                 boundingBoxOut:(CGRect *)boundingBoxOut
                  confidenceOut:(float *)confidenceOut
                          error:(NSError **)errorOut;

@end

NS_ASSUME_NONNULL_END
