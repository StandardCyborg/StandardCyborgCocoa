//
//  PerspectiveCamera+AVFoundation.mm
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 12/19/18.
//

#import <AVFoundation/AVFoundation.h>
#import <standard_cyborg/util/DataUtils.hpp>
#import <standard_cyborg/math/Mat3x3.hpp>
#import <standard_cyborg/math/Mat3x4.hpp>
#import <standard_cyborg/math/Vec2.hpp>
#import <vector>

#import "EigenHelpers.hpp"
#import "GeometryHelpers.hpp"
#import "PerspectiveCamera+AVFoundation.hpp"

using namespace standard_cyborg;

sc3d::PerspectiveCamera PerspectiveCameraFromAVCameraCalibrationData(AVCameraCalibrationData *calibrationData, size_t pixelsWide, size_t pixelsHigh)
{
    NSData *lensDistortionLookupTableData = calibrationData.lensDistortionLookupTable;
    NSData *inverseLensDistortionLookupTableData = calibrationData.inverseLensDistortionLookupTable;
    
    std::vector<float> lensDistortionTable {};
    const float* tableData = (float*)[lensDistortionLookupTableData bytes];
    lensDistortionTable.assign(tableData, tableData + [lensDistortionLookupTableData length] / sizeof(float));

    std::vector<float> inverseLensDistortionTable {};
    const float* inverseTableData = (float*)[inverseLensDistortionLookupTableData bytes];
    inverseLensDistortionTable.assign(inverseTableData, inverseTableData + [inverseLensDistortionLookupTableData length] / sizeof(float));

    sc3d::PerspectiveCamera camera;

    // Determined after length investigation of the effect of focal length on dimensional accuracy and
    // loop closure. There's a fair bit of scatter in the data, so this is certainly not "correct",
    // merely "much, much better than nothing." For more details, see:
    //    https://github.com/StandardCyborg/ScanAnalysis/tree/master/ScanAnalysis/2019-05-09-dimensional-accuracy-calibration
    camera.setFocalLengthScaleFactor(1.019f);

    math::Mat3x4 extrinsicMatrix(toMat3x4(calibrationData.extrinsicMatrix));
    camera.setExtrinsicMatrix(extrinsicMatrix);

    // The extrinsic matrix Apple gives us in calibrationData.extrinsicMatrix doesn't match the
    // coordinate system we otherwise prefer, so we post-multiply by an orientation matrix to
    // produce world space with x to the right, y up, and z toward the user.
    //
    // On 4:3 TrueDepth sensors, the sensor's pixel-u axis points UP in portrait, so we swap
    // X↔Y and negate Z. On 16:9 sensors (iPhone 17 Pro+), the pixel-u axis points RIGHT in
    // portrait, so no swap is needed — just negate Y and Z.
    CGSize refDims = calibrationData.intrinsicMatrixReferenceDimensions;
    bool isWidescreenSensor = (refDims.width / refDims.height) > 1.5f;

    math::Mat3x4 desiredOrientation = isWidescreenSensor
        ? math::Mat3x4({
            1, 0, 0, 0,
            0, -1, 0, 0,
            0, 0, -1, 0
        })
        : math::Mat3x4({
            0, 1, 0, 0,
            1, 0, 0, 0,
            0, 0, -1, 0
        });
    // We construct a baseline extrinsic matrix strictly to nail down our desired output given
    // expected input, with the additional expectation that if the extrinsic matrix returned by
    // Apple changed for some reason, we'd actually want to pick up those changes. That is to say,
    // all of this is constructed to save and use Apple's extrinsic matrix but post-multiply it
    // by our own matrix strictly to achieve the matrix we *want*.
    math::Mat3x4 baselineExtrinsicMatrix = math::Mat3x4::Identity();
    math::Mat3x4 orientationMatrix(desiredOrientation * baselineExtrinsicMatrix.inverse());
    
    camera.setOrientationMatrix(orientationMatrix);
    
    camera.setNominalIntrinsicMatrix(toMat3x3(calibrationData.intrinsicMatrix));
    camera.setIntrinsicMatrixReferenceSize(math::Vec2(calibrationData.intrinsicMatrixReferenceDimensions.width,
                                                           calibrationData.intrinsicMatrixReferenceDimensions.height));
    camera.setLensDistortionLookupTable(lensDistortionTable);
    camera.setInverseLensDistortionLookupTable(inverseLensDistortionTable);
    camera.setLegacyImageSize({(int)pixelsWide, (int)pixelsHigh});
    
    return camera;
}
