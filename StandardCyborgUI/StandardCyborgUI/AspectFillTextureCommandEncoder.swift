//
//  AspectFillTextureCommandEncoder.swift
//  Capture
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import Foundation
import Metal
import simd
import StandardCyborgFusion

public class AspectFillTextureCommandEncoder {
    
    private struct Uniforms {
        var transform = simd_float3x3(0)
        var alpha: Float = 0.3
        var __memoryPadding = simd_float4(repeating: 0)
    }
    
    private let _pipelineState: MTLComputePipelineState
    private let _textureCache: CVMetalTextureCache
    private var _uniforms = Uniforms()
    
    public init(device: MTLDevice, library: MTLLibrary) {
        _pipelineState = AspectFillTextureCommandEncoder._buildColorPipelineState(withDevice: device, library: library)
        
        var cache: CVMetalTextureCache?
        CVMetalTextureCacheCreate(kCFAllocatorDefault, nil, device, nil, &cache)
        _textureCache = cache!
    }
    
    public var alpha: Float = 0.3
    
    public func encodeCommands(onto commandBuffer: MTLCommandBuffer,
                               colorBuffer: CVPixelBuffer,
                               outputTexture: MTLTexture)
    {
        guard let colorTexture = _metalTexture(fromColorBuffer: colorBuffer)
            else { return }
        
        _uniforms.alpha = alpha
        _uniforms.transform = AspectFillTextureCommandEncoder._buildRotateAspectFitTransform(
            sourceWidth: colorTexture.width,
            sourceHeight: colorTexture.height,
            resultWidth: outputTexture.width,
            resultHeight: outputTexture.height)
        
        _encodeColorRenderCommands(onto: commandBuffer,
                                   colorTexture: colorTexture,
                                   outputTexture: outputTexture)
    }
    
    // MARK: - Private
    
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
    
    private func _encodeColorRenderCommands(onto commandBuffer: MTLCommandBuffer,
                                            colorTexture: MTLTexture,
                                            outputTexture: MTLTexture)
    {
        let resultWidth = outputTexture.width
        let resultHeight = outputTexture.height
        let threadgroupCounts = MTLSize(width: 8, height: 8, depth: 1)
        let threadgroups = MTLSize(width:  resultWidth  / threadgroupCounts.width  + (resultWidth  % threadgroupCounts.width  == 0 ? 0 : 1),
                                   height: resultHeight / threadgroupCounts.height + (resultHeight % threadgroupCounts.height == 0 ? 0 : 1),
                                   depth: 1)
        
        
        let commandEncoder = commandBuffer.makeComputeCommandEncoder()!
        commandEncoder.label = "DrawColorTexture.commandEncoder"
        commandEncoder.setComputePipelineState(_pipelineState)
        commandEncoder.setBytes(&_uniforms, length: MemoryLayout<Uniforms>.size, index: 0)
        commandEncoder.setTexture(colorTexture, index: 0)
        commandEncoder.setTexture(outputTexture, index: 1)
        commandEncoder.dispatchThreadgroups(threadgroups, threadsPerThreadgroup: threadgroupCounts)
        commandEncoder.endEncoding()
    }
    
    private func _metalTexture(fromColorBuffer colorBuffer: CVPixelBuffer) -> MTLTexture? {
        let textureAttributes = [kCVPixelBufferMetalCompatibilityKey: NSNumber(booleanLiteral: true),
                                 kCVMetalTextureUsage: NSNumber(integerLiteral: Int(MTLTextureUsage.shaderRead.rawValue))]
        
        var texture: CVMetalTexture?
        CVMetalTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                  _textureCache,
                                                  colorBuffer,
                                                  textureAttributes as CFDictionary,
                                                  MTLPixelFormat.bgra8Unorm,
                                                  CVPixelBufferGetWidthOfPlane(colorBuffer, 0),
                                                  CVPixelBufferGetHeightOfPlane(colorBuffer, 0),
                                                  0,
                                                  &texture)
        
        return texture == nil ? nil : CVMetalTextureGetTexture(texture!)
    }
    
    private class func _buildColorPipelineState(withDevice device: MTLDevice, library: MTLLibrary) -> MTLComputePipelineState {
        let function = library.makeFunction(name: "DrawColorTexture")!
        
        let pipelineDescriptor = MTLComputePipelineDescriptor()
        pipelineDescriptor.computeFunction = function
        pipelineDescriptor.label = "DrawColorTexture._depthColorPipelineState"
        
        let pipelineState = try! device.makeComputePipelineState(function: function)
        
        return pipelineState
    }
    
}
