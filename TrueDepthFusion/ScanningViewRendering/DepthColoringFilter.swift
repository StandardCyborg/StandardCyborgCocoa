//
//  DepthColoringFilter.swift
//  TrueDepthFusion
//
//  Created by Aaron Thompson on 8/22/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import CoreImage
import CoreVideo
import Foundation
import MetalPerformanceShaders
import StandardCyborgFusion

class DepthColoringFilter {
    
    private struct Uniforms {
        var minDepth: Float = 0
        var maxDepth: Float = .greatestFiniteMagnitude
        var transform = simd_float3x3(0)
    }
    
    var inputImage: CIImage?
    
    var minDepth: Float {
        get { return _uniforms.minDepth }
        set { _uniforms.minDepth = newValue }
    }
    
    var maxDepth: Float {
        get { return _uniforms.maxDepth }
        set { _uniforms.maxDepth = newValue }
    }
    
    private let _colorPipelineState: MTLComputePipelineState
    private let _depthColorPipelineState: MTLComputePipelineState
    private let _metalTextureCache: CVMetalTextureCache
    private let _blurShader: MPSImageGaussianBlur!
    private var _blurredDepthTexture: MTLTexture?
    private var _uniforms = Uniforms()
    
    private let kBlurRadius = 3
    
    init(device: MTLDevice, library: MTLLibrary) {
        _colorPipelineState = DepthColoringFilter._buildColorPipelineState(withDevice: device, library: library)
        _depthColorPipelineState = DepthColoringFilter._buildDepthColorPipelineState(withDevice: device, library: library)
        
        var cache: CVMetalTextureCache?
        CVMetalTextureCacheCreate(kCFAllocatorDefault, nil, device, nil, &cache)
        _metalTextureCache = cache!
        
        _blurShader = MPSImageGaussianBlur(device: device, sigma: Float(kBlurRadius))
        _blurShader.edgeMode = MPSImageEdgeMode.clamp
        _blurShader.label = "DepthColoringFilter._blurShader"
    }
    
    private class func _buildRotateAspectFitTransform(sourceWidth: Int, sourceHeight: Int,
                                                      resultWidth: Int, resultHeight: Int)
    -> simd_float3x3
    {
        // Note that the source aspect ratio is inverted. This is because the source
        // data is sideways. It makes things much, much easier to reason about if we
        // think of things in terms of how they show up on the screen
        let sourceAspectRatio = Float(sourceHeight) / Float(sourceWidth)
        let resultAspectRatio = Float(resultWidth) / Float(resultHeight)
        
        // The matrix below is derived in maxima through following steps:
        //   1. scale by the result width/height
        //   2. translate by (-0.5, -0.5)
        //   3. rotate 90 degrees and scale by the aspect ratio
        //   4. translate by (0.5, 0.5)
        //
        // In maxima, that's:
        //  A: matrix([1, 0, -1 / 2], [0, 1, -1 / 2], [0, 0, 1])
        //  C: matrix([0, xScale, 0], [yScale, 0, 0], [0, 0, 1])
        //  B: matrix([1, 0, 1 / 2], [0, 1, 1 / 2], [0, 0, 1])
        //  D: matrix([1 / resultWidth, 0, 0], [0, 1 / resultHeight, 0], [0, 0, 1])
        //
        //  expand(B . C . A . D)
        
        // This enforces either fill (cover if you're into css) by comparing the space we're
        // aiming to fill with the aspect ratio of the input.
        var imageScale: simd_float2
        if sourceAspectRatio > resultAspectRatio {
            // The source data is wider than the display
            imageScale = simd_float2(sourceAspectRatio / resultAspectRatio, 1.0)
        } else {
            // The display is wider than the source data
            imageScale = simd_float2(1.0, resultAspectRatio / sourceAspectRatio)
        }
        
        var transform = simd_float3x3(0)
        transform[1][0] = 1.0 / (imageScale[1] * Float(resultHeight))
        transform[2][0] = 0.5 * (1.0 - 1.0 / imageScale[1])
        transform[0][1] = 1.0 / (imageScale[0] * Float(resultWidth))
        transform[2][1] = 0.5 * (1.0 - 1.0 / imageScale[0])
        
        return transform
    }
    
    func encodeCommands(onto commandBuffer: MTLCommandBuffer,
                        colorBuffer: CVPixelBuffer,
                        depthBuffer: CVPixelBuffer?,
                        outputTexture: MTLTexture)
    {
        let colorTexture = _metalTexture(fromColorBuffer: colorBuffer)!
        
        _uniforms.transform = DepthColoringFilter._buildRotateAspectFitTransform(
            sourceWidth: colorTexture.width,
            sourceHeight: colorTexture.height,
            resultWidth: outputTexture.width,
            resultHeight: outputTexture.height)
        
        if let depthBuffer = depthBuffer {
            CVPixelBufferReplaceNaNs(depthBuffer, 0)
            
            let depthTexture = _metalTexture(fromDepthBuffer: depthBuffer, device: commandBuffer.device)!
            
            _uniforms.minDepth = 0
            _uniforms.maxDepth = 1.4 * CVPixelBufferAverageDepthAroundCenter(depthBuffer)
            
            _encodeDepthAndColorRenderCommands(onto: commandBuffer,
                                               colorTexture: colorTexture,
                                               depthTexture: depthTexture,
                                               outputTexture: outputTexture)
        } else {
            _encodeColorRenderCommands(onto: commandBuffer,
                                       colorTexture: colorTexture,
                                       outputTexture: outputTexture)
        }
    }
    
    private func _encodeColorRenderCommands(onto commandBuffer: MTLCommandBuffer,
                                            colorTexture: MTLTexture,
                                            outputTexture: MTLTexture)
    {
        let resultWidth = outputTexture.width
        let resultHeight = outputTexture.height
        let threadgroupCounts = MTLSize(width: 8, height: 8, depth: 1)
        let threadgroups = MTLSize( width: resultWidth  / threadgroupCounts.width  + (resultWidth  % threadgroupCounts.width  == 0 ? 0 : 1),
                                   height: resultHeight / threadgroupCounts.height + (resultHeight % threadgroupCounts.height == 0 ? 0 : 1),
                                   depth: 1)
        
        
        let commandEncoder = commandBuffer.makeComputeCommandEncoder()!
        commandEncoder.label = "DepthColoringFilter.commandEncoder"
        commandEncoder.setComputePipelineState(_colorPipelineState)
        commandEncoder.setBytes(&_uniforms, length: MemoryLayout<Uniforms>.size, index: 0)
        commandEncoder.setTexture(colorTexture, index: 0)
        commandEncoder.setTexture(outputTexture, index: 1)
        commandEncoder.dispatchThreadgroups(threadgroups, threadsPerThreadgroup: threadgroupCounts)
        commandEncoder.endEncoding()
    }
    
    private func _encodeDepthAndColorRenderCommands(onto commandBuffer: MTLCommandBuffer,
                                                    colorTexture: MTLTexture,
                                                    depthTexture: MTLTexture,
                                                    outputTexture: MTLTexture)
    {
        if _blurredDepthTexture == nil {
            let blurredDepthDescriptor = MTLTextureDescriptor.texture2DDescriptor(
                pixelFormat: MTLPixelFormat.r32Float,
                width: depthTexture.width,
                height: depthTexture.height,
                mipmapped: false)
            blurredDepthDescriptor.usage = [.shaderRead, .shaderWrite]
            
            _blurredDepthTexture = commandBuffer.device.makeTexture(descriptor: blurredDepthDescriptor)
            _blurredDepthTexture?.label = "DepthColoringFilter._blurredDepthTexture"
        }
        
        // Blur the depth texture using a Metal performance shader
        _blurShader.encode(commandBuffer: commandBuffer,
                           sourceTexture: depthTexture,
                           destinationTexture: _blurredDepthTexture!)
        
        let resultWidth = outputTexture.width
        let resultHeight = outputTexture.height
        let threadgroupCounts = MTLSize(width: 8, height: 8, depth: 1)
        let threadgroups = MTLSize( width: resultWidth  / threadgroupCounts.width  + (resultWidth  % threadgroupCounts.width  == 0 ? 0 : 1),
                                   height: resultHeight / threadgroupCounts.height + (resultHeight % threadgroupCounts.height == 0 ? 0 : 1),
                                   depth: 1)
        
        let commandEncoder = commandBuffer.makeComputeCommandEncoder()!
        commandEncoder.label = "DepthColoringFilter.commandEncoder"
        commandEncoder.setComputePipelineState(_depthColorPipelineState)
        commandEncoder.setBytes(&_uniforms, length: MemoryLayout<Uniforms>.size, index: 0)
        commandEncoder.setTexture(colorTexture, index: 0)
        commandEncoder.setTexture(_blurredDepthTexture, index: 1)
        commandEncoder.setTexture(outputTexture, index: 2)
        commandEncoder.dispatchThreadgroups(threadgroups, threadsPerThreadgroup: threadgroupCounts)
        commandEncoder.endEncoding()
    }
    
    private func _metalTexture(fromColorBuffer colorBuffer: CVPixelBuffer) -> MTLTexture? {
        let textureAttributes: [CFString: Any] = [
            kCVPixelBufferMetalCompatibilityKey: true,
            kCVMetalTextureUsage: MTLTextureUsage.shaderRead.rawValue
        ]
        
        var texture: CVMetalTexture?
        CVMetalTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                  _metalTextureCache,
                                                  colorBuffer,
                                                  textureAttributes as CFDictionary,
                                                  MTLPixelFormat.bgra8Unorm,
                                                  CVPixelBufferGetWidthOfPlane(colorBuffer, 0),
                                                  CVPixelBufferGetHeightOfPlane(colorBuffer, 0),
                                                  0,
                                                  &texture)
        
        return CVMetalTextureGetTexture(texture!)
    }
    
    private func _metalTexture(fromDepthBuffer depthBuffer: CVPixelBuffer, device: MTLDevice) -> MTLTexture? {
        let textureAttributes: [CFString: Any] = [
            kCVPixelBufferMetalCompatibilityKey: true,
            kCVMetalTextureUsage: MTLTextureUsage.shaderWrite.rawValue
        ]
        
        var texture: CVMetalTexture?
        CVMetalTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                  _metalTextureCache,
                                                  depthBuffer,
                                                  textureAttributes as CFDictionary,
                                                  MTLPixelFormat.r32Float,
                                                  CVPixelBufferGetWidth(depthBuffer),
                                                  CVPixelBufferGetHeight(depthBuffer),
                                                  0,
                                                  &texture)
        
        return CVMetalTextureGetTexture(texture!)
    }
    
    private class func _buildColorPipelineState(withDevice device: MTLDevice, library: MTLLibrary) -> MTLComputePipelineState {
        let function = library.makeFunction(name: "DrawColorTexture")!
        
        let pipelineDescriptor = MTLComputePipelineDescriptor()
        pipelineDescriptor.computeFunction = function
        pipelineDescriptor.label = "DepthColoringFilter._depthColorPipelineState"
        
        let pipelineState = try! device.makeComputePipelineState(function: function)
        
        return pipelineState
    }
    
    private class func _buildDepthColorPipelineState(withDevice device: MTLDevice, library: MTLLibrary) -> MTLComputePipelineState {
        let function = library.makeFunction(name: "DepthColoringFilter")!
        
        let pipelineDescriptor = MTLComputePipelineDescriptor()
        pipelineDescriptor.computeFunction = function
        pipelineDescriptor.label = "DepthColoringFilter._depthColorPipelineState"
        
        let pipelineState = try! device.makeComputePipelineState(function: function)
        
        return pipelineState
    }
    
    private let _blurShaderCopyAllocator: MPSCopyAllocator = { (kernel: MPSKernel, buffer: MTLCommandBuffer, texture: MTLTexture) -> MTLTexture in
        let descriptor = MTLTextureDescriptor.texture2DDescriptor(pixelFormat: texture.pixelFormat,
                                                                  width: texture.width,
                                                                  height: texture.height,
                                                                  mipmapped: false)
        descriptor.usage = .shaderWrite
        
        return buffer.device.makeTexture(descriptor: descriptor)!
    }
    
}
