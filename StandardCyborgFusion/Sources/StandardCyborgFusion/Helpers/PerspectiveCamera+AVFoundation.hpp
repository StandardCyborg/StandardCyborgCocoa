//
//  PerspectiveCamera+AVFoundation.h
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 12/19/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <standard_cyborg/sc3d/PerspectiveCamera.hpp>

@class AVCameraCalibrationData;

extern standard_cyborg::sc3d::PerspectiveCamera PerspectiveCameraFromAVCameraCalibrationData(AVCameraCalibrationData *calibrationData, size_t pixelsWide, size_t pixelsHigh);
