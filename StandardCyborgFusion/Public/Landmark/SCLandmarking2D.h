//
//  SCLandmarking2D.h
//  StandardCyborgFusion
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>

@class CIImage;
@class SCLandmark2D;

NS_ASSUME_NONNULL_BEGIN

@protocol SCLandmarking2D <NSObject>

/** Return an array of landmark names ordered according to their indices */
+ (NSArray<NSString *> *)landmarkNames;

/**
 Optionally return a set of integer landmark indices to exclude for the given
 frame index and amount of model rotation about the y axis
 */
- (NSSet<NSNumber *> * _Nullable)excludedLandmarkIndicesForFrameIndex:(int)frameIndex rotation:(float)radians;

/**
 @param image A CIImage loaded in linear color space
 @param excludedIndices Landmark indices that you should ignore while processing
 */
- (NSArray<SCLandmark2D *> *)findLandmarksOnImage:(CIImage *)image
                         excludingLandmarkIndices:(NSSet<NSNumber *> * _Nullable)excludedIndices
                                         rotation:(float)rotationX;

@optional

/**
 @return RGB values from 0.0 - 1.0
 */
+ (simd_float3)colorForLandmarkIndex:(NSInteger)landmarkIndex;

@end

NS_ASSUME_NONNULL_END

/** A convenience method to add a range of integer NSNumber values to the set */
@interface NSMutableSet (Range)
- (void)addIntegerValuesFrom:(NSInteger)from through:(NSInteger)through;
@end
