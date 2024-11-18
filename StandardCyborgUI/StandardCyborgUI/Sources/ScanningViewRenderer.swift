//
//  ScanningViewRenderer.swift
//  TrueDepthFusion
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import AVFoundation
import Foundation
import Metal
import StandardCyborgFusion

@objc public protocol ScanningViewRenderer {
    
    /** Set to true if you want to mirror across x, such as when a mirror bracket is attached */
    var flipsInputHorizontally: Bool { get set }
    
    /** The designated initializer. Pass the same MTLDevice instance as is used
     for the rest of your rendering and your CAMetalLayer */
    init(device: MTLDevice, commandQueue: MTLCommandQueue)
    
    /** Renders into the specified CAMetalLayer */
    func draw(colorBuffer: CVPixelBuffer?,
              pointCloud: SCPointCloud?,
              depthCameraCalibrationData: AVCameraCalibrationData,
              viewMatrix: matrix_float4x4,
              into metalLayer: CAMetalLayer)
}
