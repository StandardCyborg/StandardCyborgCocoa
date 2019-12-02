//
//  SCLandmark2D.h
//  FaceLandmarking
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <simd/simd.h>

NS_ASSUME_NONNULL_BEGIN

/**
 Represents a ML-generated landmark on a 2D image
 */
@interface SCLandmark2D : NSObject

+ (instancetype)landmarkNamed:(NSString *)name
                        index:(int)index
                     position:(simd_float2)position
                   confidence:(float)confidence
                        color:(simd_float3)color
NS_SWIFT_NAME(init(named:index:position:confidence:color:));

- (instancetype)init NS_UNAVAILABLE;

/** A name that can uniquely identify this landmark */
@property (nonatomic, readonly) NSString *landmarkName;

/** The 0-based index of this landmark within a set of ML-generated landmarks */
@property (nonatomic, readonly) int landmarkIndex;

/** The x position within the image, normalized. 0 is left. */
@property (nonatomic, readonly) float x;

/** The y position within the image, normalized. 0 is top. */
@property (nonatomic, readonly) float y;

/** The confidence of this landmark, from 0.0-1.0 */
@property (nonatomic, readonly) float confidence;

/** Convenience getter and setter for x and y */
@property (nonatomic, readonly) simd_float2 simdPosition;

/** RGB from 0.0 - 1.0 */
@property (nonatomic) simd_float3 color;

@end

NS_ASSUME_NONNULL_END
