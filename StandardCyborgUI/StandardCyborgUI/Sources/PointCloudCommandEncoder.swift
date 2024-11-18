//
//  PointCloudCommandEncoder.swift
//  TrueDepthFusion
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import AVFoundation
import Metal
import MetalKit
import StandardCyborgFusion

private func _roundUpToMultiple(value: Int, multiple: Int) -> Int {
    return ((value + multiple - 1) / multiple) * multiple
}

private struct SharedUniforms {
    let modelViewMatrix: simd_float4x4
    let normalMatrix: simd_float3x3
    let modelViewProjection: simd_float4x4
    let pointSize: Float
    let __memoryPadding: simd_float2
}

/** Submits Metal commands onto a command buffer to render a point cloud */
public class PointCloudCommandEncoder {
    private let _device: MTLDevice
    private let _pipelineState: MTLRenderPipelineState
    private let _depthStencilState: MTLDepthStencilState
    private let _sharedUniformsBuffer: MTLBuffer
    private var _depthTexture: MTLTexture?
    private let _matcapTexture: MTLTexture

    // MARK: - MetalVisualization

    private class func pointCloudVertexDescriptor() -> MTLVertexDescriptor {
        let descriptor = MTLVertexDescriptor()
        
        descriptor.layouts[0].stride = SCPointCloud.pointStride()
        descriptor.attributes[0].format = SCPointCloud.positionMTLVertexFormat()
        descriptor.attributes[0].offset = SCPointCloud.positionOffset()
        descriptor.attributes[1].format = SCPointCloud.normalMTLVertexFormat()
        descriptor.attributes[1].offset = SCPointCloud.normalOffset()
        descriptor.attributes[2].format = SCPointCloud.colorMTLVertexFormat()
        descriptor.attributes[2].offset = SCPointCloud.colorOffset()
        descriptor.attributes[3].format = SCPointCloud.weightMTLVertexFormat()
        descriptor.attributes[3].offset = SCPointCloud.weightOffset()
        
        return descriptor
    }
    
    /** The designated initializer. Pass the same MTLDevice instance as is used
        for the rest of your rendering and your CAMetalLayer */
    public init(device: MTLDevice, library: MTLLibrary) {
        _device = device
        
        let textureLoader = MTKTextureLoader(device: _device)
        let bundle = Bundle.scuiResourcesBundle
        /*
        guard let matcapURL = bundle.url(forResource: "matcap", withExtension: "png") else {
            fatalError("Couldn't find matcap.png in \(bundle)")
        }
        
        _matcapTexture = try! textureLoader.newTexture(URL: matcapURL, options: [MTKTextureLoader.Option.SRGB: NSNumber(booleanLiteral: false)])
         */
        _matcapTexture = try! textureLoader.newTexture(name: "matcap",
                                                       scaleFactor: 1,
                                                       bundle: bundle,
                                                       options: [MTKTextureLoader.Option.SRGB: NSNumber(booleanLiteral: false)])

        
        let vertexFunction = library.makeFunction(name: "RenderSCPointCloudVertex")
        let fragmentFunction = library.makeFunction(name: "RenderSCPointCloudFragment")
        
        let pipelineDescriptor = MTLRenderPipelineDescriptor()
        pipelineDescriptor.vertexFunction = vertexFunction
        pipelineDescriptor.fragmentFunction = fragmentFunction
        pipelineDescriptor.vertexDescriptor = PointCloudCommandEncoder.pointCloudVertexDescriptor()
        pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormat.bgra8Unorm
        pipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormat.depth32Float
        pipelineDescriptor.label = "PointCloudCommandEncoder._pipelineState"
        
        let depthStencilDescriptor = MTLDepthStencilDescriptor()
        depthStencilDescriptor.depthCompareFunction = MTLCompareFunction.less
        depthStencilDescriptor.isDepthWriteEnabled = true
        depthStencilDescriptor.label = "PointCloudCommandEncoder._depthStencilState"
        _depthStencilState = _device.makeDepthStencilState(descriptor: depthStencilDescriptor)!
        
        _pipelineState = try! device.makeRenderPipelineState(descriptor: pipelineDescriptor)
        
        _sharedUniformsBuffer = device.makeBuffer(length: MemoryLayout<SharedUniforms>.size,
                                                  options: [MTLResourceOptions.cpuCacheModeWriteCombined])!
        _sharedUniformsBuffer.label = "PointCloudCommandEncoder._sharedUniformsBuffer"
    }

    public func encodeCommands(onto commandBuffer: MTLCommandBuffer,
                               pointCloud: SCPointCloud,
                               depthCameraCalibrationData: AVCameraCalibrationData,
                               viewMatrix: simd_float4x4,
                               pointSize: Float,
                               flipsInputHorizontally: Bool = false,
                               outputTexture: MTLTexture)
    {
        guard pointCloud.pointCount > 0 else { return }
        
        if _depthTexture == nil {
            let descriptor = MTLTextureDescriptor.texture2DDescriptor(pixelFormat: MTLPixelFormat.depth32Float,
                                                                      width: outputTexture.width,
                                                                      height: outputTexture.height,
                                                                      mipmapped: false)
            descriptor.usage = MTLTextureUsage.renderTarget
            descriptor.storageMode = MTLStorageMode.private
            _depthTexture = _device.makeTexture(descriptor: descriptor)
            _depthTexture?.label = "PointCloudCommandEncoder._depthTexture"
        }
        
        let pointsBuffer = pointCloud.buildPointsMTLBuffer(with: _device)
        pointsBuffer.label = "PointCloudCommandEncoder.pointsBuffer"
        
        self._updateSharedUniformsBuffer(calibrationData: depthCameraCalibrationData,
                                         viewMatrix: viewMatrix,
                                         resultWidth: outputTexture.width,
                                         resultHeight: outputTexture.height,
                                         pointSize: pointSize,
                                         flipsInputHorizontally: flipsInputHorizontally)
        
        let passDescriptor = MTLRenderPassDescriptor()
        passDescriptor.colorAttachments[0].texture = outputTexture
        passDescriptor.colorAttachments[0].storeAction = MTLStoreAction.store
        passDescriptor.colorAttachments[0].loadAction = MTLLoadAction.load
        passDescriptor.depthAttachment.texture = _depthTexture
        passDescriptor.depthAttachment.loadAction = MTLLoadAction.clear
        passDescriptor.depthAttachment.storeAction = MTLStoreAction.dontCare
        passDescriptor.depthAttachment.clearDepth = 1
        
        let commandEncoder = commandBuffer.makeRenderCommandEncoder(descriptor: passDescriptor)!
        commandEncoder.label = "PointCloudCommandEncoder.commandEncoder"
        
        commandEncoder.setRenderPipelineState(_pipelineState)
        commandEncoder.setViewport(MTLViewport(originX: 0, originY: 0,
                                               width: Double(outputTexture.width),
                                               height: Double(outputTexture.height),
                                               znear: -1, zfar: 1))
        commandEncoder.setDepthStencilState(_depthStencilState)
        commandEncoder.setFrontFacing(MTLWinding.counterClockwise)
        commandEncoder.setCullMode(MTLCullMode.back)
        commandEncoder.setVertexTexture(_matcapTexture, index: 0)
        commandEncoder.setVertexBuffer(pointsBuffer, offset: 0, index: 0)
        commandEncoder.setVertexBuffer(_sharedUniformsBuffer, offset: 0, index: 1)
        commandEncoder.drawPrimitives(type: MTLPrimitiveType.point, vertexStart: 0, vertexCount: pointCloud.pointCount)
        
        commandEncoder.endEncoding()
    }
    
    // MARK: - Private
    
    private func _updateSharedUniformsBuffer(calibrationData: AVCameraCalibrationData,
                                             viewMatrix: simd_float4x4,
                                             resultWidth: Int,
                                             resultHeight: Int,
                                             pointSize: Float,
                                             flipsInputHorizontally: Bool)
    {
        // The full perpective matrix based given intrinsic matrix K,
        //
        //        [ fx  gamma  x0 ]
        //    K = [  0     fy  y0 ]
        //        [  0      0   1 ]
        //
        // reference dimensions W and H, and near and far limits n and f,
        // respectively, is:
        //
        //        [ -2 fx / W           0      1 - 2 x0 / W                 0 ]
        //    P = [         0   -2 fy / H      1 - 2 y0 / H                 0 ]
        //        [         0           0   (f + n)/(n - f)   2 f n / (n - f) ]
        //        [         0           0                -1                 0 ]
        //
        // P(0, 2) and P(1, 2) are both very nearly zero such that for the sake of
        // a preview (if not for the real thing, though we could almost certainly
        // get away with it), we can simply neglect the entries in order to focus
        // on the aspect ratio. The different is just a tiny, tiny skew.
        let intrinsicMatrix = calibrationData.intrinsicMatrix
        let fx = intrinsicMatrix.columns.0.x
        let fy = intrinsicMatrix.columns.1.y
        let sourceWidth = calibrationData.intrinsicMatrixReferenceDimensions.width
        let sourceHeight = calibrationData.intrinsicMatrixReferenceDimensions.height
        
        // These don't have to be perfect. We don't expect anything closer than
        // a millimeter or farther than ten meters given conceivable technological
        // limitations.
        let near: Float = 0.001 // meters
        let far: Float  = 10.0 // meters
        
        // NB: the source aspect ratio is inverted because the incoming frame is sideways
        // relative to what we display. It's much, much, much easier to reason about if
        // we just flip it.
        let resultAspectRatio = Float(resultWidth) / Float(resultHeight)
        let sourceAspectRatio = Float(sourceHeight) / Float(sourceWidth)
        
        var imageScale = simd_float2()
        var referenceSize: Float
        if sourceAspectRatio > resultAspectRatio {
            // The source data is wider than the result display
            imageScale.x = 1 / resultAspectRatio
            imageScale.y = 1
            referenceSize = Float(sourceWidth)
        } else {
            imageScale.x = 1
            imageScale.y = resultAspectRatio
            referenceSize = Float(sourceHeight)
        }
        
        let projection = simd_float4x4([
            simd_float4([ 2 * fx / referenceSize * imageScale[0], 0, 0, 0]),
            simd_float4([ 0,  2 * fy / referenceSize * imageScale[1], 0, 0]),
            simd_float4([ 0,  0, (far + near) / (near - far), -1]),
            simd_float4([ 0,  0, 2 * far * near / (near - far), 0])
        ])
        
        let viewInverse = viewMatrix.inverse
        
        // Construct an intrinsic matrix which flips the image vertically on the screen, swaps
        // the horizontal and vertical axes, and also performs a self-flip.
        var extrinsic = simd_float4x4([
            simd_float4([  1,  0,  0,  0 ]),
            simd_float4([  0,  1,  0,  0 ]),
            simd_float4([  0,  0,  -1,  0 ]),
            simd_float4([  0,  0,  0,  1 ]),
        ])
        if flipsInputHorizontally {
            extrinsic.columns.1.x = -1
        }
        
        let modelView = matrix_multiply(extrinsic, viewInverse)
        
        let truncatedModelView = simd_float3x3([
            simd_float3([modelView.columns.0[0], modelView.columns.0[1], modelView.columns.0[2]]),
            simd_float3([modelView.columns.1[0], modelView.columns.1[1], modelView.columns.1[2]]),
            simd_float3([modelView.columns.2[0], modelView.columns.2[1], modelView.columns.2[2]])
        ])
        
        let normalMatrix = truncatedModelView.inverse.transpose
        
        let modelViewProjection = matrix_multiply(projection, matrix_multiply(extrinsic, viewInverse))
        
        var sharedUniforms = SharedUniforms(modelViewMatrix: modelView,
                                            normalMatrix: normalMatrix,
                                            modelViewProjection: modelViewProjection,
                                            pointSize: pointSize,
                                            __memoryPadding: simd_float2(repeating: 0))
        
        memcpy(_sharedUniformsBuffer.contents(), &sharedUniforms, MemoryLayout<SharedUniforms>.size)
    }
    
}
