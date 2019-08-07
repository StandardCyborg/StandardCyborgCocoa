//
//  SCLandmark3D.h
//  StandardCyborgFusion
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <simd/simd.h>

NS_ASSUME_NONNULL_BEGIN

@interface SCLandmark3D : NSObject

/** A string uniquely representing this label, e.g. "leftPupil" */
@property (nonatomic, readonly) NSString *label;

/** The 0-based index of this landmark's label */
@property (nonatomic, readonly) NSInteger labelIndex;

/** Units in meters */
@property (nonatomic, readonly) simd_float3 position;

- (instancetype)init NS_UNAVAILABLE;
- (instancetype)initWithLabel:(NSString *)label
                   labelIndex:(int)labelIndex
                     position:(simd_float3)position;

/** Returns the Euclidean distance from this landmark to another */
- (float)distanceToLandmark3D:(SCLandmark3D *)other;

@end

NS_ASSUME_NONNULL_END
