//
//  EarLandmarkingAnalysis.h
//  EarLandmarking
//
//  Created by Aaron Thompson on 5/23/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <CoreML/CoreML.h>
#import <Foundation/Foundation.h>

@class CIImage;
@class SCLandmark2D;

NS_ASSUME_NONNULL_BEGIN

@interface EarLandmarkingAnalysis : NSObject

/** @param modelURL Pointing to the mlmodelc directory provided by Standard Cyborg */
- (instancetype)initWithModelAtURL:(NSURL *)modelURL;

/**
 @param image Expected to be in portrait orientation
 @param normalizedEarBoundingBox The bounding box of the ear, normalized from (0, 0) to (1, 1), origin at bottom left
 @return An array of ear landmarks found in the image
 */
- (NSArray<SCLandmark2D *> *)analyzeImage:(CIImage *)image
                 normalizedEarBoundingBox:(CGRect)normalizedEarBoundingBox
                                isLeftEar:(BOOL)isLeftEar;

/** Return an array of landmark names ordered according to their indices */
+ (NSArray<NSString *> *)landmarkNames;

@end

NS_ASSUME_NONNULL_END
