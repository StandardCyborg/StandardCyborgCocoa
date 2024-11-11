//
//  MathHelpers.h
//  TrueDepthFusion
//
//  Created by Aaron Thompson on 5/14/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#pragma once

#ifdef __OBJC__
#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>
#endif
#include <simd/simd.h>

#if defined(__cplusplus)
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif


EXTERN float flerp(float value, float min, float max);
EXTERN simd_float3 simd_float3lerp(float value, simd_float3 min, simd_float3 max);
EXTERN simd_float4x4 simd_float4x3_homogeneous(simd_float4x3 m);
EXTERN float clamp(float value, float min, float max);


#ifdef __OBJC__
// clang-format off
EXTERN NSArray *ColMajorNSArrayFromMatrixFloat3x3(matrix_float3x3 m);
EXTERN NSArray *ColMajorNSArrayFromMatrixFloat4x3(matrix_float4x3 m);

EXTERN CGFloat CGFloatExponentialMovingAverage(CGFloat newValue, CGFloat previousValue, CGFloat alpha);
EXTERN CGRect CGRectExponentialMovingAverage(CGRect newValue, CGRect previousValue, CGFloat positionAlpha, CGFloat sizeAlpha);

EXTERN void FindClippedMinMaxPixelValues(size_t count, float bottomClipPercentage, float topClipPercentage, const float *values, float *minValueOut, float *maxValueOut);
// clang-format on
#endif

// https://martin.ankerl.com/2007/10/04/optimized-pow-approximation-for-java-and-c-c/
static inline float fastPow(float a, float b)
{
    union {
        float d;
        int x[2];
    } u = {a};
    u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
    u.x[0] = 0;
    return u.d;
}

// https://en.wikipedia.org/wiki/Fast_inverse_square_root
/// A fast approximation to 1/sqrt(x)
static inline float fastInverseSqrt(float number)
{
    long i;
    float x2, y;
    const float threehalfs = 1.5F;
    
    x2 = number * 0.5F;
    y = number;
    i = *(long*)&y; // evil floating point bit level hacking
    i = 0x5f3759df - (i >> 1); // what the fuck?
    y = *(float*)&i;
    y = y * (threehalfs - (x2 * y * y)); // 1st iteration
    //  y  = y * (threehalfs - (x2 * y * y)); // 2nd iteration,
    // this can be removed
    
    return y;
}


// https://hero.handmade.network/forums/code-discussion/t/1278-approach_for_faster_software_srgb_gamma_computation
/// Applies a x^2.2 gamma correction factor using a computationally efficient approximation
static inline float fastApplyGammaCorrection(float input)
{
    float commonTerm = 0.2f * (input - 1.f);
    return input * input * (1.f + commonTerm * (1.f - 2.f * commonTerm));
}

// http://mimosa-pudica.net/fast-gamma/
/// Applies a x^1/2.2 gamma correction factor using a computationally efficient approximation
static inline float fastUnapplyGammaCorrection(float input)
{
    static const float a = 0.00279491;
    static const float b = 1.15907984;
    float c = b * fastInverseSqrt(1.0f + a) - 1.0f;
    
    return (b * fastInverseSqrt(input + a) - c) * input;
}

static inline size_t roundUpToMultiple(size_t value, size_t multiple)
{
    return ((value + multiple - 1) / multiple) * multiple;
}
