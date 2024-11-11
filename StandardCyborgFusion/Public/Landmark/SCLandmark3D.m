//
//  SCLandmark3D.m
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 6/13/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import "SCLandmark3D.h"

@implementation SCLandmark3D

- (instancetype)initWithLabel:(NSString *)label
                   labelIndex:(int)labelIndex
                     position:(simd_float3)position
                        color:(simd_float3)color
{
    self = [super init];
    if (self) {
        _label = label;
        _labelIndex = labelIndex;
        _position = position;
        _color = color;
    }
    return self;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"Landmark %@ is at %f, %f, %f", _label, _position.x, _position.y, _position.z];
}

- (float)distanceToLandmark3D:(SCLandmark3D *)other
{
    return simd_distance(_position, other.position);
}

@end
