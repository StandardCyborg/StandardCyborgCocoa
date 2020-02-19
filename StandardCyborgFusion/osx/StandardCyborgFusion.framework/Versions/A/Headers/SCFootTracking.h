//
//  SCFootTracking.h
//  StandardCyborgFusion
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class SCFootTracking;


@protocol SCFootTrackingDelegate <NSObject>
/**
 @param normalizedRect  The bounding box of the detected foot, normalized from 0-1
 @param confidence  The confidence of the detected foot, from 0-1
 */
- (void)footTracking:(SCFootTracking *)footTracking
     didDetectFootAt:(CGRect)normalizedRect
          confidence:(CGFloat)confidence;

@optional
- (void)footTrackingDidLoseTracking:(SCFootTracking *)footTracking
NS_SWIFT_NAME(footTrackingDidLoseTracking(_:));

@end


/** Efficiently calculates the bounding box of a foot. */
@interface SCFootTracking : NSObject

@property (nonatomic, weak) id<SCFootTrackingDelegate> delegate;

/** The amount of smoothing between iterations, from [0..1). 0 is no smoothing, 1 is infinite smoothing. Defaults to 0.5 */
@property (nonatomic) float smoothing;

/**
@param modelURL Local file URL to the SCFootTracking.mlmodelc directory provided by Standard Cyborg.
                Please contact sdk@standardcyborg.com for these models.
*/
- (instancetype)initWithModelURL:(NSURL *)modelURL
                        delegate:(id<SCFootTrackingDelegate>)delegate;

- (instancetype)init NS_UNAVAILABLE;

- (void)analyzePixelBuffer:(CVPixelBufferRef)pixelBuffer orientation:(CGImagePropertyOrientation)orientation;

@end

NS_ASSUME_NONNULL_END
