/*
Copyright 2020 Standard Cyborg

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "standard_cyborg/sc3d/PerspectiveCamera.hpp"

#include <iostream>

#include "standard_cyborg/util/DebugHelpers.hpp"
#include "standard_cyborg/math/Vec3.hpp"
#include "standard_cyborg/math/Vec4.hpp"
#include "standard_cyborg/sc3d/ColorImage.hpp"
#include "standard_cyborg/sc3d/DepthImage.hpp"
#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/util/IncludeEigen.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#ifndef DEBUG
#define EIGEN_NO_DEBUG  1
#endif
#include "Eigen/QR"
#pragma clang diagnostic pop

using standard_cyborg::math::Mat3x4;
using standard_cyborg::math::Mat4x4;
using standard_cyborg::math::Vec4;
using standard_cyborg::math::Vec2;

namespace standard_cyborg {
namespace sc3d {

static math::Vec4 computeLensDistortionCurveFit(const std::vector<float>& table) {
    // This function performs a least-squares fit for an emprically-determined equation
    // of the form:
    //   y(x) = c0 * x^2 + c1 * x^3 + c2 * x^4 + c3 * x^5
    //
    // In practice, this should be evaluated using Horner's method as
    //   y(x) = x^2*(c0 + x*(c1 + x*(c2 + x*c3)))
    size_t n = table.size();
    Eigen::MatrixXf A (n, 4);
    Eigen::VectorXf b (n);
    for (off_t i = 0; i < n; i++) {
        float t = (float)i / (n - 1);
        A(i, 0) = std::pow(t, 2);
        A(i, 1) = std::pow(t, 3);
        A(i, 2) = std::pow(t, 4);
        A(i, 3) = std::pow(t, 5);
        b(i) = table[i];
    }
    Eigen::VectorXf x (A.colPivHouseholderQr().solve(b));
    return math::Vec4 {x(0), x(1), x(2), x(3)};
}

PerspectiveCamera::PerspectiveCamera(math::Mat3x3 nominalIntrinsicMatrix,
                  math::Vec2 intrinsicMatrixReferenceSize,
                  float focalLengthScaleFactor,
                  math::Mat3x4 extrinsicMatrix,
                  math::Mat3x4 orientationMatrix,
                  std::vector<float> lensDistortionCalibration,
                  std::vector<float> inverseLensDistortionCalibration
                  )
{
    setNominalIntrinsicMatrix(nominalIntrinsicMatrix);
    setIntrinsicMatrixReferenceSize(intrinsicMatrixReferenceSize);
    setFocalLengthScaleFactor(focalLengthScaleFactor);
    setExtrinsicMatrix(extrinsicMatrix);
    setOrientationMatrix(orientationMatrix);
    setLensDistortionLookupTable(lensDistortionCalibration);
    setInverseLensDistortionLookupTable(inverseLensDistortionCalibration);
}

void PerspectiveCamera::setNominalIntrinsicMatrix(const math::Mat3x3& _nominalIntrinsicMatrix)
{
    nominalIntrinsicMatrix = _nominalIntrinsicMatrix;
    
    // Apply the existing scale factor to enforce consistency
    setFocalLengthScaleFactor(focalLengthScaleFactor);
}

float PerspectiveCamera::getFocalLengthScaleFactor() const
{
    return focalLengthScaleFactor;
}

void PerspectiveCamera::setFocalLengthScaleFactor(float scaleFactor)
{
    focalLengthScaleFactor = scaleFactor;
    
    intrinsicMatrix = nominalIntrinsicMatrix;
    intrinsicMatrix.m00 *= focalLengthScaleFactor;
    intrinsicMatrix.m11 *= focalLengthScaleFactor;
    
    intrinsicMatrixInverse = intrinsicMatrix.inverse();
}

math::Mat3x3 PerspectiveCamera::getNominalIntrinsicMatrix() const
{
    return nominalIntrinsicMatrix;
}

const math::Mat3x3& PerspectiveCamera::getIntrinsicMatrix() const
{
    return intrinsicMatrix;
}

const math::Mat3x3& PerspectiveCamera::getIntrinsicMatrixInverse() const
{
    return intrinsicMatrixInverse;
}

void PerspectiveCamera::setExtrinsicMatrix(const Mat3x4& extrinsicMatrix_)
{
    extrinsicMatrix = extrinsicMatrix_;
    extrinsicMatrixInverse = extrinsicMatrix_.inverse();
}

void PerspectiveCamera::setOrientationMatrix(const Mat3x4& orientationMatrix_)
{
    orientationMatrix = orientationMatrix_;
    orientationMatrixInverse = orientationMatrix.inverse();
}

const math::Mat3x4& PerspectiveCamera::getExtrinsicMatrix() const
{
    return extrinsicMatrix;
}

const math::Mat3x4& PerspectiveCamera::getOrientationMatrix() const
{
    return orientationMatrix;
}

void PerspectiveCamera::copy(const PerspectiveCamera& other)
{
    focalLengthScaleFactor = other.getFocalLengthScaleFactor();
    setIntrinsicMatrixReferenceSize(other.getIntrinsicMatrixReferenceSize());
    setNominalIntrinsicMatrix(other.getNominalIntrinsicMatrix());
    setOrientationMatrix(other.getOrientationMatrix());
    setExtrinsicMatrix(other.getExtrinsicMatrix());
    setLensDistortionLookupTable(other.getLensDistortionCalibration());
    setInverseLensDistortionLookupTable(other.getInverseLensDistortionCalibration());
    setFrame(other.getFrame());
}

int PerspectiveCamera::getSizeInBytes() const
{
    return (int)(sizeof(PerspectiveCamera) +
                 sizeof(float) * lensDistortionCalibration.size() +
                 sizeof(float) * inverseLensDistortionCalibration.size());
}

Mat4x4 PerspectiveCamera::getPerspectiveMatrix(float near, float far) const
{
    float m00 = 2.0f * intrinsicMatrix.m00 / intrinsicMatrixReferenceSize.x;
    float m11 = 2.0f * intrinsicMatrix.m11 / intrinsicMatrixReferenceSize.y;
    float m02 = -1.0f + 2.0f * intrinsicMatrix.m02 / intrinsicMatrixReferenceSize.x;
    float m12 = -1.0f + 2.0f * intrinsicMatrix.m12 / intrinsicMatrixReferenceSize.y;
    Mat4x4 P {
        -m00,  0.0f,   -m02,  0.0f,
        0.0f,  -m11,   -m12,  0.0f,
        0.0f,  0.0f,  -(far + near) / (far - near), -2.0f * far * near / (far - near),
        0.0f,  0.0f,  -1.0f,  0.0f
    };
    return P;
}

Mat4x4 PerspectiveCamera::getProjectionViewMatrix(float near, float far) const {
    return getPerspectiveMatrix(near, far) * Mat4x4(getViewMatrix());
}

Mat3x4 PerspectiveCamera::getViewMatrix() const {
    return extrinsicMatrix * orientationMatrix;
}

Mat3x4 PerspectiveCamera::getViewMatrixInverse() const {
    return getViewMatrix().inverse();
}


Mat3x4 PerspectiveCamera::getImageAlignmentTransform() const
{
    float f = 0.5 / getIntrinsicMatrix().m00 * getIntrinsicMatrixReferenceSize().x;
    return getViewMatrixInverse() * Mat3x4::fromTranslation({0.0, 0.0, -1.0}) * Mat3x4::fromScale({-f});
}

void PerspectiveCamera::setLensDistortionLookupTable(const std::vector<float>& table)
{
    lensDistortionCalibration = table;
    lensDistortionCurveFit = computeLensDistortionCurveFit(table);
}

void PerspectiveCamera::setInverseLensDistortionLookupTable(const std::vector<float>& table)
{
    inverseLensDistortionCalibration = table;
    inverseLensDistortionCurveFit = computeLensDistortionCurveFit(table);
}

const std::vector<float>& PerspectiveCamera::getLensDistortionCalibration() const
{
    return lensDistortionCalibration;
}

const std::vector<float>& PerspectiveCamera::getInverseLensDistortionCalibration() const
{
    return inverseLensDistortionCalibration;
}

Vec4 PerspectiveCamera::getLensDistortionCurveFit() const
{
    return lensDistortionCurveFit;
}

Vec4 PerspectiveCamera::getInverseLensDistortionCurveFit() const
{
    return inverseLensDistortionCurveFit;
}

Vec2 PerspectiveCamera::getOpticalImageCenter() const
{
    return Vec2 { intrinsicMatrix.m02, intrinsicMatrix.m12 };
}

float PerspectiveCamera::getOpticalImageMaxRadius() const
{
    Vec2 center = getOpticalImageCenter();
    return Vec2::max(center, getIntrinsicMatrixReferenceSize() - center).norm();
}

Size2D PerspectiveCamera::getLegacyImageSize() const {
    return legacyImageSize;
}

void PerspectiveCamera::setLegacyImageSize(Size2D size) {
    legacyImageSize = size;
}

bool PerspectiveCamera::operator==(const PerspectiveCamera& other) const {
    auto ArraysEqual = [](const std::vector<float> &l, const std::vector<float> &r) {
        if (l.size() != r.size()) { return false; }
        for (size_t i = 0; i < l.size(); ++i) {
            if (l[i] != r[i]) { return false; }
        }
        return true;
    };
    
    return 
        frame == other.frame &&
        focalLengthScaleFactor == other.focalLengthScaleFactor &&
        intrinsicMatrixReferenceSize == other.intrinsicMatrixReferenceSize &&
        nominalIntrinsicMatrix == other.nominalIntrinsicMatrix &&
        // intrinsicMatrix == other.intrinsicMatrix &&
        extrinsicMatrix == other.extrinsicMatrix &&
        orientationMatrix == other.orientationMatrix &&
        ArraysEqual(lensDistortionCalibration, other.lensDistortionCalibration) &&
        lensDistortionCurveFit == other.lensDistortionCurveFit &&
        legacyImageSize == other.legacyImageSize;
}

math::Vec3 PerspectiveCamera::unprojectDepthSample(
      int imageWidth,
      int imageHeight,
      float pixelCol,
      float pixelRow,
      float depth) const {

    using namespace standard_cyborg::math;
    
    const auto& intrinsicMatrixInverse = getIntrinsicMatrixInverse();
    const auto refSize = getIntrinsicMatrixReferenceSize();
    
    math::Vec3 xyHomogeneous{
        (float)pixelCol / (float)imageWidth * refSize.x,
        (1.0f - (float)pixelRow / (float)imageHeight) * refSize.y,
        1.0f
    };
    
    return getViewMatrixInverse() * (-depth * (intrinsicMatrixInverse * xyHomogeneous));
}

/*
Geometry PerspectiveCamera::unprojectFrame(
      const DepthImage& depth,
      const ColorImage& color,
      float minDepth,
      float maxDepth) const {

    std::vector<float> depthData = depth.getData();
    
    std::vector<math::Vec3> positions;
    std::vector<math::Vec3> colors;
    
    int w = depth.getWidth();
    int h = depth.getHeight();

    depth.forEachPixelAtColRow([&](int col, int row, float depth) {
        if (std::isnan(depth) || depth < minDepth || depth > maxDepth) return;
        colors.push_back(color.getPixelAtColRow(col, row).xyz());
        positions.push_back(unprojectDepthSample(w, h, col, row, depth));
    });
    
    Geometry geometryOut;
    geometryOut.setNormals({});
    geometryOut.setFaces({});
    geometryOut.setTexCoords({});
    geometryOut.setNormalsEncodeSurfelRadius(false);
    geometryOut.setPositions(positions);
    geometryOut.setColors(colors);
    return geometryOut;
}
 */

} // namespace sc3d
} // namespace standard_cyborg
