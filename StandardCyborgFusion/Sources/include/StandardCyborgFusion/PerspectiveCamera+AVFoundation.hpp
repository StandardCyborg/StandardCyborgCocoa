//
//  PerspectiveCamera+AVFoundation.h
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 12/19/18.
//

#ifdef __cplusplus

#import <standard_cyborg/sc3d/PerspectiveCamera.hpp>

@class AVCameraCalibrationData;

extern standard_cyborg::sc3d::PerspectiveCamera PerspectiveCameraFromAVCameraCalibrationData(AVCameraCalibrationData *calibrationData, size_t pixelsWide, size_t pixelsHigh);

#endif
