//
//  GeometryHelpers.cpp
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 8/24/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#include "GeometryHelpers.hpp"
#include "standard_cyborg/math/Mat3x3.hpp"
#include "standard_cyborg/math/Mat3x4.hpp"
#include "standard_cyborg/math/Mat3x4.hpp"
#include "standard_cyborg/math/Mat4x4.hpp"
#include "standard_cyborg/math/Vec2.hpp"
#include "standard_cyborg/math/Vec3.hpp"
#include "standard_cyborg/math/Vec4.hpp"

using namespace standard_cyborg;

Eigen::Matrix4f perspectiveMatrixFromIntrinsics(Eigen::Matrix3f& intrinsicMatrix,
                                         Eigen::Vector2f& referenceDimensions,
                                         float near,
                                         float far)
{
    Eigen::Matrix4f result = Eigen::Matrix4f::Zero();
    result(0, 0) = -2.0 * intrinsicMatrix(0, 0) / referenceDimensions(0);
    result(0, 2) = 1.0 - 2.0 * intrinsicMatrix(0, 2) / referenceDimensions(0);
    result(1, 1) = -2.0 * intrinsicMatrix(1, 1) / referenceDimensions(1);
    result(1, 2) = 1.0 - 2.0 * intrinsicMatrix(1, 2) / referenceDimensions(1);
    result(2, 2) = -(far + near) / (far - near);
    result(2, 3) = -2.0 * far * near / (far - near);
    result(3, 2) = -1.0;
    return result;
}

float angleBetweenVectors(Eigen::Vector3f a, Eigen::Vector3f b) {
    return acosf(a.dot(b) / a.norm() / b.norm());
}

// SC --> simd

simd_float2 toSimdFloat2(math::Vec2 v) {
    return simd_make_float2(v.x, v.y);
}

simd_float3 toSimdFloat3(math::Vec3 v) {
    return simd_make_float3(v.x, v.y, v.z);
}

simd_float4 toSimdFloat4(math::Vec4 v) {
    return simd_make_float4(v.x, v.y, v.z, v.w);
}

simd_float3x3 toSimdFloat3x3(const math::Mat3x3& m) {
    simd_float3x3 result;
    result.columns[0].x = m.m00;
    result.columns[0].y = m.m10;
    result.columns[0].z = m.m20;
    result.columns[1].x = m.m01;
    result.columns[1].y = m.m11;
    result.columns[1].z = m.m21;
    result.columns[2].x = m.m02;
    result.columns[2].y = m.m12;
    result.columns[2].z = m.m22;
    
    return result;
}

simd_float4x4 toSimdFloat4x4(const math::Mat3x4& m) {
    simd_float4x4 result;
    result.columns[0].x = m.m00;
    result.columns[0].y = m.m10;
    result.columns[0].z = m.m20;
    result.columns[0].w = 0;
    result.columns[1].x = m.m01;
    result.columns[1].y = m.m11;
    result.columns[1].z = m.m21;
    result.columns[1].w = 0;
    result.columns[2].x = m.m02;
    result.columns[2].y = m.m12;
    result.columns[2].z = m.m22;
    result.columns[2].w = 0;
    result.columns[3].x = m.m03;
    result.columns[3].y = m.m13;
    result.columns[3].z = m.m23;
    result.columns[3].w = 1;
    
    return result;
}

simd_float4x4 toSimdFloat4x4(const math::Mat4x4& m) {
    simd_float4x4 result;
    result.columns[0].x = m.m00;
    result.columns[0].y = m.m10;
    result.columns[0].z = m.m20;
    result.columns[0].w = m.m30;
    result.columns[1].x = m.m01;
    result.columns[1].y = m.m11;
    result.columns[1].z = m.m21;
    result.columns[1].w = m.m31;
    result.columns[2].x = m.m02;
    result.columns[2].y = m.m12;
    result.columns[2].z = m.m22;
    result.columns[2].w = m.m32;
    result.columns[3].x = m.m03;
    result.columns[3].y = m.m13;
    result.columns[3].z = m.m23;
    result.columns[3].w = m.m33;
    
    return result;
}

// simd --> SC

math::Mat3x3 toMat3x3(simd_float3x3 m) {
    return math::Mat3x3(
        m.columns[0].x, m.columns[1].x, m.columns[2].x,
        m.columns[0].y, m.columns[1].y, m.columns[2].y,
        m.columns[0].z, m.columns[1].z, m.columns[2].z
    );
}

math::Mat3x4 toMat3x4(simd_float3x3 m) {
    return math::Mat3x4(
        m.columns[0].x, m.columns[1].x, m.columns[2].x, 0.0f,
        m.columns[0].y, m.columns[1].y, m.columns[2].y, 0.0f,
        m.columns[0].z, m.columns[1].z, m.columns[2].z, 0.0f
    );
}

math::Mat3x4 toMat3x4(simd_float4x3 m) {
    return math::Mat3x4(
        m.columns[0].x, m.columns[1].x, m.columns[2].x, m.columns[3].x,
        m.columns[0].y, m.columns[1].y, m.columns[2].y, m.columns[3].y,
        m.columns[0].z, m.columns[1].z, m.columns[2].z, m.columns[3].z
    );
}

math::Mat3x4 toMat3x4(simd_float4x4 m) {
    return math::Mat3x4(
        m.columns[0].x, m.columns[1].x, m.columns[2].x, m.columns[3].x,
        m.columns[0].y, m.columns[1].y, m.columns[2].y, m.columns[3].y,
        m.columns[0].z, m.columns[1].z, m.columns[2].z, m.columns[3].z
    );
}

Eigen::Matrix3f rotationFromEulerAngles(const Eigen::Vector3f& alphaBetaGamma) {
    // Beware that this is *not* a general function and carries with it a very specific
    // set of asumptions about the ordering of rotations. It is derived from the following
    // Maxima code for rotation about the X, Y, and Z axes:
    //
    //     A:matrix([ 1,   0,  0], [ 0, ca, -sa], [  0, sa, ca]);
    //     B:matrix([cb,   0, sb], [ 0,  1,   0], [-sb,  0, cb]);
    //     C:matrix([cg, -sg,  0], [sg, cg,   0], [  0,  0,  1]);
    //     R: A . B . C;

    float calpha = cos(alphaBetaGamma(0));
    float salpha = sin(alphaBetaGamma(0));
    float cbeta = cos(alphaBetaGamma(1));
    float sbeta = sin(alphaBetaGamma(1));
    float cgamma = cos(alphaBetaGamma(2));
    float sgamma = sin(alphaBetaGamma(2));

    Eigen::Matrix3f R;
    R << cbeta * cgamma,
         -cbeta * sgamma,
         sbeta,
         calpha * sgamma + salpha * sbeta * cgamma,
         calpha * cgamma - salpha * sbeta * sgamma,
         -salpha * cbeta,
         salpha * sgamma - calpha * sbeta * cgamma,
         calpha * sbeta * sgamma + salpha * cgamma,
         calpha * cbeta;
    
    return R;
}
