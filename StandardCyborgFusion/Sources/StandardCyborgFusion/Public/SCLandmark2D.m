//
//  SCLandmark2D.m
//  
//
//  Created by Aaron Thompson on 5/23/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import "SCLandmark2D_Private.h"

@implementation SCLandmark2D

+ (instancetype)landmarkNamed:(NSString *)name
                        index:(int)index
                     position:(simd_float2)position
                   confidence:(float)confidence
                        color:(simd_float3)color
{
    SCLandmark2D *landmark = [[SCLandmark2D alloc] _init];
    
    landmark.landmarkName = name;
    landmark.landmarkIndex = index;
    landmark.simdPosition = position;
    landmark.confidence = confidence;
    landmark.color = color;
    
    return landmark;
}

- (instancetype)_init {
    self = [super init];
    return self;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"Landmark %d (%@): %.3f, %.3f confidence %.4f",
            _landmarkIndex, _landmarkName, _x, _y, _confidence];
}

- (simd_float2)simdPosition
{
    return simd_make_float2(_x, _y);
}

- (void)setSimdPosition:(simd_float2)simdPosition
{
    _x = simdPosition.x;
    _y = simdPosition.y;
}

@end
