//
//  GeometryHelpers.hpp
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 8/24/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#pragma once

#include <standard_cyborg/util/IncludeEigen.hpp>
#include <simd/simd.h>

namespace standard_cyborg {

namespace math {
struct Mat3x3;
struct Mat3x4;
struct Mat4x4;
struct Vec2;
struct Vec3;
struct Vec4;
}


}

extern float angleBetweenVectors(Eigen::Vector3f a, Eigen::Vector3f b);

extern Eigen::Matrix4f perspectiveMatrixFromIntrinsics(Eigen::Matrix3f& intrinsicMatrix,
                                                       Eigen::Vector2f& referenceDimensions,
                                                       float near,
                                                       float far);

extern Eigen::Matrix3f rotationFromEulerAngles(const Eigen::Vector3f& alphaBetaGamma);

// SC --> simd
extern simd_float3x3 toSimdFloat3x3(const standard_cyborg::math::Mat3x3& m);
extern simd_float4x4 toSimdFloat4x4(const standard_cyborg::math::Mat3x4& m);
extern simd_float4x4 toSimdFloat4x4(const standard_cyborg::math::Mat4x4& m);
extern simd_float2 toSimdFloat2(standard_cyborg::math::Vec2 m);
extern simd_float3 toSimdFloat3(standard_cyborg::math::Vec3 m);
extern simd_float4 toSimdFloat4(standard_cyborg::math::Vec4 m);

// simd --> SC
extern standard_cyborg::math::Mat3x3 toMat3x3(simd_float3x3 m);
extern standard_cyborg::math::Mat3x4 toMat3x4(simd_float3x3 m);
extern standard_cyborg::math::Mat3x4 toMat3x4(simd_float4x4 m);
extern standard_cyborg::math::Mat3x4 toMat3x4(simd_float4x3 m);

// Not really geometry, but close enough
static const float kGammaCorrection = 1.0/2.2;
static const float kGammaCorrectionInv = 2.2/1.0;

static inline float applyGammaCorrection(float x) {
    return powf(x, kGammaCorrection);
}
static inline float unapplyGammaCorrection(float x) {
    return powf(x, kGammaCorrectionInv);
}
