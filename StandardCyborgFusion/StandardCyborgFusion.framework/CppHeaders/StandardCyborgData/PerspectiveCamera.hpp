//
//  PerspectiveCamera.hpp
//  StandardCyborgData
//
//  Created by Ricky Reusser on 8/22/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <vector>

#include <StandardCyborgData/Mat3x3.hpp>
#include <StandardCyborgData/Mat3x4.hpp>
#include <StandardCyborgData/Mat4x4.hpp>
#include <StandardCyborgData/Vec2.hpp>
namespace StandardCyborg {

class PerspectiveCamera {
public:
    PerspectiveCamera();

    /** Copy another camera into this camera */
    void copy(const PerspectiveCamera& src);
    
    /** Set the intrinsic matrix reference size */
    void setIntrinsicMatrixReferenceSize(Vec2 referenceSize);
    
    /** Get the intrinsic matrix reference size */
    const Vec2& getIntrinsicMatrixReferenceSize() const;

    /** Set the factor by which the focal lengths are scaled */
    void setFocalLengthScaleFactor(float factor);

    /** Get the factor by which the focal lengths are scaled */
    float getFocalLengthScaleFactor() const;
    
    /** Set the *uncalibrated* intrinsic matrix. Upon setting, the focal length scale factor will be applied */
    void setUncalibratedIntrinsicMatrix(const Mat3x3& uncalibratedIntrinsicMatrix);
    
    /** Return the original uncalibrated intrinsic matrix */
    Mat3x3 getUncalibratedIntrinsicMatrix() const;

    /** Set the extrinsic matrix */
    void setExtrinsicMatrix(const Mat3x4& _extrinsicMatrix);
    
    /** Get the calibrated intrinsic matrix */
    const Mat3x3& getIntrinsicMatrix() const;

    /** Get the inverse of the calibrated intrinsic matrix */
    const Mat3x3& getIntrinsicMatrixInverse() const;
    
    /** Get the extrinsic matrix */
    const Mat3x4& getExtrinsicMatrix() const;
    
    /** Get the extrinsic matrix */
    const Mat3x4& getExtrinsicMatrixInverse() const;
    
    /** Get the approximate size of the camera in bytes */
    int getSizeInBytes() const;

    /** Get a 4x4 OpenGL-style perspective matrix for this camera */
    Mat4x4 getPerspectiveMatrix(float near, float far) const;

private:
    float focalLengthScaleFactor = 1.0;
    
    Vec2 intrinsicMatrixReferenceSize;
    Mat3x3 uncalibratedIntrinsicMatrix;
    Mat3x3 intrinsicMatrix;
    Mat3x3 intrinsicMatrixInverse;
    
    Mat3x4 extrinsicMatrix;
    Mat3x4 extrinsicMatrixInverse;
    
    std::vector<float> lensDistortionCalibration;
    std::vector<float> inverseLensDistortionCalibration;
};


} // namespace StandardCyborg
