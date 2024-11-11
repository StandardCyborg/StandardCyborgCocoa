//
//  ScanningViewRenderer.swift
//  TrueDepthFusion
//
//  Created by Aaron Thompson on 9/23/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import AVFoundation
import Foundation
import Metal
import StandardCyborgFusion

class ScanningViewRenderer
{
    private let _device: MTLDevice
    private let _library: MTLLibrary
    private let _commandQueue: MTLCommandQueue
    private let _depthColoringFilter: DepthColoringFilter
    private let _pointCloudRenderer: SCPointCloudRenderer
    
    init(device: MTLDevice, commandQueue: MTLCommandQueue) {
        _device = device
        _commandQueue = commandQueue
        _library = device.makeDefaultLibrary()!
        
        _depthColoringFilter = DepthColoringFilter(device: _device, library: _library)
        _pointCloudRenderer = SCPointCloudRenderer(device: _device, library: _library)
    }
    
    func draw(colorBuffer: CVPixelBuffer,
              depthBuffer: CVPixelBuffer?,
              pointCloud: SCPointCloud?,
              depthCameraCalibrationData: AVCameraCalibrationData,
              viewMatrix: matrix_float4x4,
              into metalLayer: CAMetalLayer,
              flipsInputHorizontally: Bool)
    {
        autoreleasepool {
            let commandBuffer = _commandQueue.makeCommandBuffer()!
            commandBuffer.label = "ScanningViewRenderer.commandBuffer"
            
            guard let drawable = metalLayer.nextDrawable() else { return }
            let outputTexture = drawable.texture
            
            _depthColoringFilter.encodeCommands(onto: commandBuffer,
                                                colorBuffer: colorBuffer,
                                                depthBuffer: nil,
                                                outputTexture: outputTexture)
            
            if let depthBuffer = depthBuffer,
               let pointCloud = pointCloud,
               pointCloud.pointCount > 0
            {
                let depthFrameSize = CGSize(width: CVPixelBufferGetWidth(depthBuffer),
                                            height: CVPixelBufferGetHeight(depthBuffer))
                
                _pointCloudRenderer.encodeCommands(onto: commandBuffer,
                                                   pointCloud: pointCloud,
                                                   depthCameraCalibrationData: depthCameraCalibrationData,
                                                   viewMatrix: viewMatrix,
                                                   outputTexture: outputTexture,
                                                   depthFrameSize: depthFrameSize,
                                                   flipsInputHorizontally: flipsInputHorizontally)
            }
            
            commandBuffer.present(drawable)
            commandBuffer.commit()
            commandBuffer.waitUntilCompleted()
        }
    }
}
