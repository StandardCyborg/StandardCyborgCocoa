//
//  MathHelpers.m
//  DepthRenderer
//
//  Created by Aaron Thompson on 5/14/18.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import "MathHelpers.h"

float flerp(float value, float min, float max)
{
    return (max - min) * value + min;
}

float clamp(float value, float min, float max)
{
    return MAX(min, MIN(max, value));
}

simd_float3 simd_float3lerp(float value, simd_float3 min, simd_float3 max)
{
    return simd_make_float3(flerp(value, min.x, max.x),
                            flerp(value, min.y, max.y),
                            flerp(value, min.z, max.z));
}

simd_float4x4 simd_float4x3_homogeneous(simd_float4x3 m)
{
    return (simd_float4x4){
        .columns[0] = { m.columns[0].x, m.columns[0].y, m.columns[0].z, 0 },
        .columns[1] = { m.columns[1].x, m.columns[1].y, m.columns[1].z, 0 },
        .columns[2] = { m.columns[2].x, m.columns[2].y, m.columns[2].z, 0 },
        .columns[3] = { m.columns[3].x, m.columns[3].y, m.columns[3].z, 1 }
    };
}

NSArray *ColMajorNSArrayFromMatrixFloat3x3(matrix_float3x3 m)
{
    return @[
             @(m.columns[0][0]), @(m.columns[0][1]), @(m.columns[0][2]),
             @(m.columns[1][0]), @(m.columns[1][1]), @(m.columns[1][2]),
             @(m.columns[2][0]), @(m.columns[2][1]), @(m.columns[2][2]),
             ];
}

NSArray *ColMajorNSArrayFromMatrixFloat4x3(matrix_float4x3 m)
{
    // 4 columns and 3 rows
    return @[
             @(m.columns[0][0]), @(m.columns[0][1]), @(m.columns[0][2]),
             @(m.columns[1][0]), @(m.columns[1][1]), @(m.columns[1][2]),
             @(m.columns[2][0]), @(m.columns[2][1]), @(m.columns[2][2]),
             @(m.columns[3][0]), @(m.columns[3][1]), @(m.columns[3][2]),
             ];
}

CGFloat CGFloatExponentialMovingAverage(CGFloat new, CGFloat previous, CGFloat alpha)
{
    if (isnan(previous)) {
        return new;
    } else {
        return alpha * new + (1.0 - alpha) * previous;
    }
}

CGRect CGRectExponentialMovingAverage(CGRect new, CGRect previous, CGFloat positionAlpha, CGFloat sizeAlpha)
{
    if (CGRectIsEmpty(previous)) {
        return new;
    } else {
        return CGRectMake(CGFloatExponentialMovingAverage(new.origin.x,  previous.origin.x,  positionAlpha),
                          CGFloatExponentialMovingAverage(new.origin.y,  previous.origin.y,  positionAlpha),
                          CGFloatExponentialMovingAverage(new.size.width,  previous.size.width,  sizeAlpha),
                          CGFloatExponentialMovingAverage(new.size.height, previous.size.height, sizeAlpha));
    }
}


