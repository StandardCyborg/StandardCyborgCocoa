//
//  EigenHelpers.hpp
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 7/23/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#pragma once

#import <simd/simd.h>
#import <vector>
#import <standard_cyborg/util/IncludeEigen.hpp>
#import <StandardCyborgFusion/Surfel.hpp>

namespace Eigen {
    typedef Matrix<uint32_t, 1, Dynamic> VectorXu;
};

extern Eigen::Vector3f Vec3TransformMat4(Eigen::Vector3f a, const Eigen::Matrix4f& m);
extern Eigen::Matrix3f NormalMatrixFromMat4(const Eigen::Matrix4f& a);

// Eigen --> simd
extern simd_float3x3 toSimdFloat3x3(const Eigen::Matrix3f& m);
extern simd_float4x4 toSimdFloat4x4(const Eigen::Matrix4f& m);

// simd --> Eigen
extern Eigen::Matrix3f toMatrix3f(const simd_float3x3 m);
extern Eigen::Matrix4f toMatrix4f(const simd_float4x3 m);
extern Eigen::Matrix4f toMatrix4f(const simd_float4x4 m);

extern void FillEigenMatrix3XfFromRGBVector(Eigen::Matrix3Xf& matrixOut, const std::vector<uint8_t>& vector, float multiplier = 1);

template<typename Derived>
static void FillEigenMatrixXfFromColMajorVector(Eigen::MatrixBase<Derived>& matrixOut, const std::vector<float>& vector, size_t rows = 0, size_t cols = 0) {
    assert(matrixOut.rows() > 0);
    assert(matrixOut.cols() > 0);
    if (rows == 0) { rows = matrixOut.rows(); }
    if (cols == 0) { cols = matrixOut.cols(); }
    
    off_t i = 0;
    for (off_t col = 0; col < cols; ++col) {
        for (off_t row = 0; row < rows && i < vector.size(); ++row) {
            matrixOut(row, col) = vector[i];
            ++i;
        }
    }
}

typedef Eigen::Map<const Eigen::Matrix3Xf, Eigen::Aligned, Eigen::Stride<sizeof(Surfel) / sizeof(float), 1>> EigenMappedSurfelPositions;
typedef Eigen::Map<const Eigen::Matrix3Xf, Eigen::Unaligned, Eigen::Stride<sizeof(Surfel) / sizeof(float), 1>> EigenMappedSurfelNormals;
typedef Eigen::Map<const Eigen::Matrix3Xf, Eigen::Unaligned, Eigen::Stride<sizeof(Surfel) / sizeof(float), 1>> EigenMappedSurfelColors;

EigenMappedSurfelPositions getEigenMappedSurfelPositions(const Surfels& surfels);
EigenMappedSurfelNormals getEigenMappedSurfelNormals(const Surfels& surfels);
EigenMappedSurfelColors getEigenMappedSurfelColors(const Surfels& surfels);
