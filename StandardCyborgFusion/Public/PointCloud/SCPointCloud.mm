//
//  SCPointCloud.m
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 12/13/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>

#include "SCPointCloud.h"
#include "SCPointCloud_Private.h"

#include <StandardCyborgFusion/Surfel.hpp>

@implementation SCPointCloud

- (instancetype)initWithSurfelData:(NSData *)data gravity:(simd_float3)gravity
{
    self = [super init];
    if (self) {
        _pointsData = data;
        
        _gravity = gravity;
    }
    return self;
}

- (NSInteger)pointCount
{
    return [_pointsData length] / [SCPointCloud pointStride];
}

+ (size_t)pointStride { return sizeof(Surfel); }

+ (size_t)positionOffset { return offsetof(Surfel, position); }
+ (size_t)normalOffset { return offsetof(Surfel, normal); }
+ (size_t)colorOffset { return offsetof(Surfel, color); }
+ (size_t)weightOffset { return offsetof(Surfel, weight); }
+ (size_t)pointSizeOffset { return offsetof(Surfel, surfelSize); }

+ (size_t)positionComponentCount { return 3; }
+ (size_t)normalComponentCount { return 3; }
+ (size_t)colorComponentCount { return 3; }
+ (size_t)weightComponentCount { return 1; }
+ (size_t)pointSizeComponentCount { return 1; }

+ (size_t)positionComponentSize { return sizeof(float); }
+ (size_t)normalComponentSize { return sizeof(float); }
+ (size_t)colorComponentSize { return sizeof(float); }
+ (size_t)weightComponentSize { return sizeof(float); }
+ (size_t)pointSizeComponentSize { return sizeof(float); }

- (simd_float3)centerOfMass
{
    Vector3f result = Vector3f::Zero();
    
    NSInteger pointCount = [self pointCount];
    Surfel *surfels = (Surfel *)[_pointsData bytes];
    
    for (NSInteger i = 0; i < pointCount; ++i) {
        Vector3f position = surfels[i].position;
        result += position;
    }
    
    result /= (float)pointCount;
    
    return simd_make_float3(result.x(), result.y(), result.z());
}

@end
