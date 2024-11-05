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

#pragma once

#include <limits>
#include <string>
#include <vector>

#include "standard_cyborg/math/Mat3x3.hpp"
#include "standard_cyborg/math/Mat3x4.hpp"
#include "standard_cyborg/math/Mat4x4.hpp"
#include "standard_cyborg/math/Vec2.hpp"
#include "standard_cyborg/math/Vec4.hpp"
#include "standard_cyborg/sc3d/Size2D.hpp"

namespace standard_cyborg {

namespace math {
struct Vec3;
}

namespace sc3d {

class Geometry;
class DepthImage;
class ColorImage;

class PerspectiveCamera {

public:
    PerspectiveCamera() { }
    
    /** This specialized constructor is provided for convenience so that the output of DebugHelpers
      * may return a representation that may be used as valid input.  ?????????????????????????????????????????/
      */
    PerspectiveCamera(math::Mat3x3 nominalIntrinsicMatrix,
                      math::Vec2 intrinsicMatrixReferenceSize,
                      float focalLengthScaleFactor,
                      math::Mat3x4 extrinsicMatrix,
                      math::Mat3x4 orientationMatrix,
                      std::vector<float> lensDistortionCalibration,
                      std::vector<float> inverseLensDistortionCalibration
                      );

    /** Copy another camera into this camera */
    void copy(const PerspectiveCamera& src);

    /** Set the intrinsic matrix reference size */
    void setIntrinsicMatrixReferenceSize(math::Vec2 referenceSize) {
      intrinsicMatrixReferenceSize = referenceSize;
    }
    
    /** Get the intrinsic matrix reference size */
    const math::Vec2& getIntrinsicMatrixReferenceSize() const {
      return intrinsicMatrixReferenceSize;
    }

    /** Set the nominal intrinsic matrix. If the focal length scale factor is 1.0, then this will
      * also be the intrinsic matrix, but if the scale factor is set, that will be applied to the
      * focal lengths when returning the intrinsic matrix. If the camera has been calibrated more
      * precisely. If the camera has been independently calibrated, you should provide that matrix
      * as the nominal intrinsic matrix and leave the focal length scale factor as the default 1.0.
      */
    void setNominalIntrinsicMatrix(const math::Mat3x3& uncalibratedIntrinsicMatrix);
    
    /** Return the unmodified nominal intrinsic matrix */
    math::Mat3x3 getNominalIntrinsicMatrix() const;

    /** Get the calibrated intrinsic matrix */
    const math::Mat3x3& getIntrinsicMatrix() const;
    
    /** Get the inverse of the calibrated intrinsic matrix */
    const math::Mat3x3& getIntrinsicMatrixInverse() const;
    
    /** Set the factor by which the focal lengths are scaled */
    void setFocalLengthScaleFactor(float factor);
    
    /** Get the factor by which the focal lengths are scaled */
    float getFocalLengthScaleFactor() const;

    /** Get the optical center from the intrinsic matrix */
    math::Vec2 getOpticalImageCenter() const;
    
    /** Get the maximum distance from the optical center to the corner of the frame */
    float getOpticalImageMaxRadius() const;

    
    /** Set the orientation transform matrix. Since the extrinsic matrix supplied by Apple devices,
      * for example, is not generally the matrix which gives us the desired model orientation, we
      * insert an extra matrix so that we may use the provided extrinsic matrix but also force the
      * model into the correct orientation.
      */
    void setOrientationMatrix(const math::Mat3x4& _orientationMatrix);
    
    /** Get the orientation transform matrix */
    const math::Mat3x4& getOrientationMatrix() const;
    
    /** Set the extrinsic matrix */
    void setExtrinsicMatrix(const math::Mat3x4& _extrinsicMatrix);

    /** Get the extrinsic matrix */
    const math::Mat3x4& getExtrinsicMatrix() const;
    

    /** Get a 4x4 OpenGL-style perspective matrix for this camera */
    math::Mat4x4 getPerspectiveMatrix(float near = 0.001, float far = 100.0) const;
    
    /** Get a 4x4 matrix containing the complete camera matrix defined by this class.
      * When projecting, the full transform between three dimensions and two dimensions (when
      * following computer graphics conventions—which includes the addition of near and far
      * clipping planes and perspective division) is:
      *
      * Projection:
      *
      *    (perspectiveMatrix * orientationMatrix * extrinsicMatrix) * <CAMERA_POSE_MATRIX> * (x, y, z, 1)
      *    \_______________________________________________________/
      *                           this struct
      *
      * Unprojection:
      *
      *    <CAMERA_POSE_MATRIX>^-1 * extrinsicMatrix^-1 * orientationMatrix^-1 * perspectiveMatrix^-1 * (xNDC, yNDC, zNDC, 1)
      *
      * where NDC means normalized device coordinates between [-1, 1] x [-1, 1] x [-1, 1].
      * Then, using
      *
      *    viewMatrix := orientationMatrix * extrinsicMatrix
      *
      * and additionally
      *
      *    projectionViewMatrix := perspectiveMatrix * orientationMatrix * extrinsicMatrix
      *
      * the above equation for projection simplifies to
      *
      *    projectionViewMatrix * <CAMERA_POSE_MATRIX> * (x, y, z, 1)
      *
      * and for unprojection,
      *
      *    <CAMERA_POSE_MATRIX>^-1 * projectionViewMatrix^-1 * (xNDC, yNDC, zNDC, 1)
      *
      * -------------------------------------------------------------------------------------------
      * IN CASE THIS IS A LOT TO READ, HERE'S WHAT YOU NEED TO KNOW:
      *
      * You may use the `projectionViewMatrix` if you are unprojecting/projecting using depth remapped
      * via clipping planes and perspective division. If you are using the intrinsic matrix to
      * project/unproject using physical depth, then you should just use the view matrix alone. There
      * is not a good reason to use either the orientation matrix or the extrinsic matrix, even
      * internally!
      * -------------------------------------------------------------------------------------------
      */
    math::Mat4x4 getProjectionViewMatrix(float near = 0.001f, float far = 100.0f) const;

    /** Get the view matrix, defined as `extrinsic * orientation` */
    math::Mat3x4 getViewMatrix() const;
    
    /** Get the inverse view matrix, defined as `(extrinsic * orientation)^-1`, which is
     * equivalent to `orientation^-1 * extrinsic^-1` */
    math::Mat3x4 getViewMatrixInverse() const;
    
    
    /** Set the lens distortion calibration lookup table */
    void setLensDistortionLookupTable(const std::vector<float>& calibration);
    
    /** Set the lens distortion calibration lookup table */
    void setInverseLensDistortionLookupTable(const std::vector<float>& calibration);
    
    /** Get the lens distortion calibration lookup table */
    const std::vector<float>& getLensDistortionCalibration() const;
    
    /** Get the lens distortion calibration lookup table */
    const std::vector<float>& getInverseLensDistortionCalibration() const;

    /** Get the lens distortion calibration curve fit coefficients */
    math::Vec4 getLensDistortionCurveFit() const;
    
    /** Get the lens distortion calibration curve fit coefficients */
    math::Vec4 getInverseLensDistortionCurveFit() const;
    
    
    /** Get the approximate size of the camera in bytes */
    int getSizeInBytes() const;

    /** Get a transform which may be applied to a ColorImage or DepthImage to align it with
      * this camera */
    math::Mat3x4 getImageAlignmentTransform() const;
    
    
    /* The methods below presume a particular image size, which we do not desire for this class
     * to track since they physical model of the camera does not correspond to any particular
     * image resolution. We rename and provide these functions for interim compatibility, but
     * seek to remove these in favor of storing image dimensions on RawFrame, for example,
     * instead. */

    /** Get image height (deprecated) */
    Size2D getLegacyImageSize() const;

    /** Set image size (deprecated) */
    void setLegacyImageSize(Size2D size);

    /* Unproject a single pixel at (`pixelCol`, `pixelRow`) with depth `depth`
     * to a point in 3-d space (in the frame of this camera) */
    math::Vec3 unprojectDepthSample(
      int imageWidth,
      int imageHeight,
      float pixelCol,
      float pixelRow,
      float depth) const;

    /* Unproject a `color` and `depth` image into a colored point cloud
     * `Geometry`.  Optionally filter depth by `minDepth` and `maxDepth`. */
    // Geometry unprojectFrame(
    //   const DepthImage& depth,
    //   const ColorImage& color,
    //   float minDepth=0,
    //   float maxDepth=std::numeric_limits<float>::max()) const;


    void setFrame(const std::string &f) { frame = f; }
    const std::string &getFrame() const { return frame; }

    bool operator==(const PerspectiveCamera& other) const;

private:
    std::string frame;

    float focalLengthScaleFactor = 1.0;
    
    math::Vec2 intrinsicMatrixReferenceSize;
    
    math::Mat3x3 nominalIntrinsicMatrix;
    math::Mat3x3 intrinsicMatrix;
    math::Mat3x3 intrinsicMatrixInverse;

    math::Mat3x4 extrinsicMatrix;
    math::Mat3x4 extrinsicMatrixInverse;
    
    math::Mat3x4 orientationMatrix;
    math::Mat3x4 orientationMatrixInverse;
    
    /**
     * Lookup table for lens distortion
     * From Apple docs:
     *
     * Images captured by a camera are geometrically warped by small imperfections in the lens.
     * To project from the 2D image plane back into the 3D world, the images must be distortion
     * corrected, or made rectilinear. Lens distortion is modeled using a one-dimensional
     * lookup table of 32-bit float values evenly distributed along a radius from the center of
     * the distortion to a corner, with each value representing a magnification of the radius.
     * This model assumes symmetrical lens distortion.
     *
     * Source: https://developer.apple.com/documentation/avfoundation/avcameracalibrationdata/2881129-lensdistortionlookuptable
     */
    std::vector<float> lensDistortionCalibration;
    math::Vec4 lensDistortionCurveFit;
    
    /** Lookup table for inverse lens distortion
     * From Apple docs:
     *
     * If you've rectified an image by removing the distortions characterized by the
     * lensDistortionLookupTable property, and now wish to go back to a geometrically distorted
     * image (for example, to render visual effects into the camera image or perform computer
     * vision tasks such as scene reconstruction), use this inverse lookup table.
     *
     * Source: https://developer.apple.com/documentation/avfoundation/avcameracalibrationdata/2881132-inverselensdistortionlookuptable
     */
    std::vector<float> inverseLensDistortionCalibration;
    math::Vec4 inverseLensDistortionCurveFit;
    
    /** Image width. Deprecated since the physical model of a camera has nothing to do with a particular
      * image size, but we're stuck with it for now.
      */
    Size2D legacyImageSize;
};

} // namespace sc3d
} // namespace standard_cyborg

