//
//  EigenHelpers.cpp
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 7/23/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#include "EigenHelpers.hpp"

#include <cmath>
#include <cstdio>
#include <iostream>
#include <string>

Eigen::Vector3f Vec3TransformMat4(Eigen::Vector3f a, const Eigen::Matrix4f& m) {
    const float x = a.x(), y = a.y(), z = a.z();
    float w = m(3, 0) * x + m(3, 1) * y + m(3, 2) * z + m(3, 3);
    if (w == 0) { w = 1.0; }
    const float wInverse = 1.0 / w;
    
    return Eigen::Vector3f(
                    (m(0, 0) * x + m(0, 1) * y + m(0, 2) * z + m(0, 3)) * wInverse,
                    (m(1, 0) * x + m(1, 1) * y + m(1, 2) * z + m(1, 3)) * wInverse,
                    (m(2, 0) * x + m(2, 1) * y + m(2, 2) * z + m(2, 3)) * wInverse
                    );
}


Eigen::Matrix3f NormalMatrixFromMat4(const Eigen::Matrix4f& a) {
    // To match the version prototyped in JS, a#1#2 is taken to mean a's #1 column, #2 row
    float a00 = a(0, 0), a01 = a(1, 0), a02 = a(2, 0), a03 = a(3, 0);
    float a10 = a(0, 1), a11 = a(1, 1), a12 = a(2, 1), a13 = a(3, 1);
    float a20 = a(0, 2), a21 = a(1, 2), a22 = a(2, 2), a23 = a(3, 2);
    float a30 = a(0, 3), a31 = a(1, 3), a32 = a(2, 3), a33 = a(3, 3);
    
    float b00 = a00 * a11 - a01 * a10;
    float b01 = a00 * a12 - a02 * a10;
    float b02 = a00 * a13 - a03 * a10;
    float b03 = a01 * a12 - a02 * a11;
    float b04 = a01 * a13 - a03 * a11;
    float b05 = a02 * a13 - a03 * a12;
    float b06 = a20 * a31 - a21 * a30;
    float b07 = a20 * a32 - a22 * a30;
    float b08 = a20 * a33 - a23 * a30;
    float b09 = a21 * a32 - a22 * a31;
    float b10 = a21 * a33 - a23 * a31;
    float b11 = a22 * a33 - a23 * a32;
    
    // Calculate the determinant
    float det = b00 * b11
              - b01 * b10
              + b02 * b09
              + b03 * b08
              - b04 * b07
              + b05 * b06;
    
    if (det == 0) { return Eigen::Matrix3f::Zero(); }
    
    det = 1.0 / det;
    
    Eigen::Matrix3f result = Eigen::Matrix3f::Zero();
    result(0, 0) = (a11 * b11 - a12 * b10 + a13 * b09) * det;
    result(1, 0) = (a12 * b08 - a10 * b11 - a13 * b07) * det;
    result(2, 0) = (a10 * b10 - a11 * b08 + a13 * b06) * det;
    
    result(0, 1) = (a02 * b10 - a01 * b11 - a03 * b09) * det;
    result(1, 1) = (a00 * b11 - a02 * b08 + a03 * b07) * det;
    result(2, 1) = (a01 * b08 - a00 * b10 - a03 * b06) * det;
    
    result(0, 2) = (a31 * b05 - a32 * b04 + a33 * b03) * det;
    result(1, 2) = (a32 * b02 - a30 * b05 - a33 * b01) * det;
    result(2, 2) = (a30 * b04 - a31 * b02 + a33 * b00) * det;
    
    return result;
}

simd_float3x3 toSimdFloat3x3(const Eigen::Matrix3f& m) {
    simd_float3x3 result;
    result.columns[0].x = m.col(0).x();
    result.columns[0].y = m.col(0).y();
    result.columns[0].z = m.col(0).z();
    result.columns[1].x = m.col(1).x();
    result.columns[1].y = m.col(1).y();
    result.columns[1].z = m.col(1).z();
    result.columns[2].x = m.col(2).x();
    result.columns[2].y = m.col(2).y();
    result.columns[2].z = m.col(2).z();
    
    return result;
}

simd_float4x4 toSimdFloat4x4(const Eigen::Matrix4f& m) {
    simd_float4x4 result;
    result.columns[0].x = m.col(0).x();
    result.columns[0].y = m.col(0).y();
    result.columns[0].z = m.col(0).z();
    result.columns[0].w = m.col(0).w();
    result.columns[1].x = m.col(1).x();
    result.columns[1].y = m.col(1).y();
    result.columns[1].z = m.col(1).z();
    result.columns[1].w = m.col(1).w();
    result.columns[2].x = m.col(2).x();
    result.columns[2].y = m.col(2).y();
    result.columns[2].z = m.col(2).z();
    result.columns[2].w = m.col(2).w();
    result.columns[3].x = m.col(3).x();
    result.columns[3].y = m.col(3).y();
    result.columns[3].z = m.col(3).z();
    result.columns[3].w = m.col(3).w();
    
    return result;
}

Eigen::Matrix3f toMatrix3f(const simd_float3x3 m) {
    Eigen::Matrix3f result;
    result.col(0).x() = m.columns[0].x;
    result.col(0).y() = m.columns[0].y;
    result.col(0).z() = m.columns[0].z;
    result.col(1).x() = m.columns[1].x;
    result.col(1).y() = m.columns[1].y;
    result.col(1).z() = m.columns[1].z;
    result.col(2).x() = m.columns[2].x;
    result.col(2).y() = m.columns[2].y;
    result.col(2).z() = m.columns[2].z;
    
    return result;
}

extern Eigen::Matrix4f toMatrix4f(const matrix_float4x3 m) {
    // 3 rows, 4 columns
    Eigen::Matrix4f result;
    result.col(0).x() = m.columns[0].x;
    result.col(0).y() = m.columns[0].y;
    result.col(0).z() = m.columns[0].z;
    result.col(1).x() = m.columns[1].x;
    result.col(1).y() = m.columns[1].y;
    result.col(1).z() = m.columns[1].z;
    result.col(2).x() = m.columns[2].x;
    result.col(2).y() = m.columns[2].y;
    result.col(2).z() = m.columns[2].z;
    result.col(3).x() = m.columns[3].x;
    result.col(3).y() = m.columns[3].y;
    result.col(3).z() = m.columns[3].z;
    
    // It is hereby a stated assumption that these are the identity:
    result(3, 0) = 0.0;
    result(3, 1) = 0.0;
    result(3, 2) = 0.0;
    result(3, 3) = 1.0;

    return result;
}

extern Eigen::Matrix4f toMatrix4f(const simd_float4x4 m) {
    Eigen::Matrix4f result;
    result.col(0).x() = m.columns[0].x;
    result.col(0).y() = m.columns[0].y;
    result.col(0).z() = m.columns[0].z;
    result.col(0).w() = m.columns[0].w;
    result.col(1).x() = m.columns[1].x;
    result.col(1).y() = m.columns[1].y;
    result.col(1).z() = m.columns[1].z;
    result.col(1).w() = m.columns[1].w;
    result.col(2).x() = m.columns[2].x;
    result.col(2).y() = m.columns[2].y;
    result.col(2).z() = m.columns[2].z;
    result.col(2).w() = m.columns[2].w;
    result.col(3).x() = m.columns[3].x;
    result.col(3).y() = m.columns[3].y;
    result.col(3).z() = m.columns[3].z;
    result.col(3).w() = m.columns[3].w;
    
    return result;
}

void FillEigenMatrix3XfFromRGBVector(Eigen::Matrix3Xf& matrixOut, const std::vector<uint8_t>& vector, float multiplier) {
    assert(matrixOut.cols() > 0);
    const float gammaCorrection = 1.0;
    
    for (off_t matrixIndex = 0, vectorIndex = 0;
         matrixIndex < matrixOut.cols();
         ++matrixIndex, vectorIndex += 3)
    {
        uint8_t valueR = vector[vectorIndex];
        uint8_t valueG = vector[vectorIndex+1];
        uint8_t valueB = vector[vectorIndex+2];
        float normalizedR = valueR * multiplier;
        float normalizedG = valueG * multiplier;
        float normalizedB = valueB * multiplier;
        float gammaCorrectedR = powf(normalizedR, gammaCorrection);
        float gammaCorrectedG = powf(normalizedG, gammaCorrection);
        float gammaCorrectedB = powf(normalizedB, gammaCorrection);
        matrixOut(0, matrixIndex) = gammaCorrectedR;
        matrixOut(1, matrixIndex) = gammaCorrectedG;
        matrixOut(2, matrixIndex) = gammaCorrectedB;
    }
}

EigenMappedSurfelPositions getEigenMappedSurfelPositions (const Surfels& surfels) {
    return EigenMappedSurfelPositions ((float*)&surfels[0].position, 3, surfels.size());
}

EigenMappedSurfelNormals getEigenMappedSurfelNormals (const Surfels& surfels) {
    return EigenMappedSurfelNormals ((float*)&surfels[0].normal, 3, surfels.size());
}

EigenMappedSurfelColors getEigenMappedSurfelColors (const Surfels& surfels) {
    return EigenMappedSurfelColors ((float*)&surfels[0].color, 3, surfels.size());
}
