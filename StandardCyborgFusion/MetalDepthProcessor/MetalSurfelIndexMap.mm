//
//  MetalSurfelIndexMap.mm
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 7/26/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#include <CoreGraphics/CoreGraphics.h>
#include <iostream>
#include <standard_cyborg/util/DataUtils.hpp>

#include "crc32.hpp"
#include "DebugLog.h"
#include "GeometryHelpers.hpp"
#include "MathHelpers.h"
#include "MetalSurfelIndexMap.hpp"
#include "PBFDefinitions.h"

using namespace Eigen;

NS_ASSUME_NONNULL_BEGIN

#if !TARGET_OS_OSX
static inline size_t __roundUpToMultiple(size_t value, size_t multiple) {
    return ((value + multiple - 1) / multiple) * multiple;
}
#endif

struct SurfelIndexMapVertex {
    simd_packed_float2 vertices;
    
    SurfelIndexMapVertex(float x, float y) {
        vertices = (simd_packed_float2){ x, y };
    }
};

struct SurfelIndexMapSharedUniforms {
    simd_float4x4 projectionViewMatrix;
    simd_float4 inverseLensDistortionConstants;
    simd_float2 opticalImageSize;
    simd_float2 opticalImageCenter;
    float maxLensCalibrationRadius;
    float surfelAliasingSafetyFactor;
    
    SurfelIndexMapSharedUniforms(const RawFrame& frame, const Matrix4f& cameraPoseMatrix) {
        const sc3d::PerspectiveCamera& camera = frame.camera;
        
        projectionViewMatrix = toSimdFloat4x4(camera.getProjectionViewMatrix() * toMat4x4(cameraPoseMatrix));
        
        inverseLensDistortionConstants = toSimdFloat4(camera.getInverseLensDistortionCurveFit());
        opticalImageSize = toSimdFloat2(camera.getIntrinsicMatrixReferenceSize());
        opticalImageCenter = toSimdFloat2(camera.getOpticalImageCenter());
        maxLensCalibrationRadius = camera.getOpticalImageMaxRadius();
        
        surfelAliasingSafetyFactor = 1.3;
    }
    
    SurfelIndexMapSharedUniforms(Matrix4f projectionView) {
        projectionViewMatrix = toSimdFloat4x4(projectionView);
    }
};

MetalSurfelIndexMap::MetalSurfelIndexMap(id<MTLDevice> device, id<MTLCommandQueue> commandQueue, bool forColor) :
    _device(device),
    _commandQueue(commandQueue)
{
    NSError *error;
    _library = [_device newDefaultLibraryWithBundle:[NSBundle bundleWithIdentifier:@"com.standardcyborg.StandardCyborgFusion"] error:&error];
    if (_library == nil) { NSLog(@"Unable to create library: %@", error); }
    
    id<MTLFunction> vertexFunction = [_library newFunctionWithName:forColor ? @"SurfelIndexMapForColorVertex" : @"SurfelIndexMapVertex"];
    id<MTLFunction> fragmentFunction = [_library newFunctionWithName:@"SurfelIndexMapFragment"];
    
    MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
    vertexDescriptor.attributes[0].format = MTLVertexFormatFloat2;
    vertexDescriptor.attributes[0].offset = 0;
    vertexDescriptor.attributes[0].bufferIndex = 0;
    vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
    vertexDescriptor.layouts[0].stride = sizeof(SurfelIndexMapVertex);
    
    MTLRenderPipelineDescriptor *pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineDescriptor.vertexFunction = vertexFunction;
    pipelineDescriptor.fragmentFunction = fragmentFunction;
    pipelineDescriptor.vertexDescriptor = vertexDescriptor;
    pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatR32Uint;
    pipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
    pipelineDescriptor.label = @"SurfelIndexMap._pipelineState";
    _pipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];
    if (_pipelineState == nil) { NSLog(@"Unable to create pipeline state: %@", error); }
    
    MTLDepthStencilDescriptor *depthStencilDescriptor = [[MTLDepthStencilDescriptor alloc] init];
    depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionLess;
    depthStencilDescriptor.depthWriteEnabled = YES;
    depthStencilDescriptor.label = @"SurfelIndexMap._depthStencilState";
    _depthStencilState = [_device newDepthStencilStateWithDescriptor:depthStencilDescriptor];
    
    _sharedUniformsBuffer = [_device newBufferWithLength:sizeof(SurfelIndexMapSharedUniforms) options:MTLResourceOptionCPUCacheModeWriteCombined];
    _sharedUniformsBuffer.label = @"SurfelIndexMap shared uniforms";
    _vertexBuffer = this->_createVertexBuffer();
    _vertexBuffer.label = @"SurfelIndexMap._vertexBuffer";
}

bool MetalSurfelIndexMap::draw(const std::vector<Surfel>& surfels,
                               const Matrix4f& modelMatrix,
                               const RawFrame& rawFrame,
                               std::vector<uint32_t>& indexLookups)
{
    bool success = true;
    
    this->_updateSharedUniformsBuffer(rawFrame, modelMatrix);
    
    size_t surfelCount = surfels.size();
    if (surfelCount == 0) { return success; }
    
    if (_indexTexture == nil || _indexTexture.width != rawFrame.width) {
        MTLTextureDescriptor *descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatR32Uint
                                                                                              width:rawFrame.width
                                                                                             height:rawFrame.height
                                                                                          mipmapped:NO];
        
        // On iOS devices, we write directly into indexLookups via a shared memory buffer
        // On Mac devices, we have to copy it after the fact
#if TARGET_OS_OSX
        descriptor.usage = MTLResourceUsageWrite|MTLTextureUsageRenderTarget;
        _indexTexture = [_device newTextureWithDescriptor:descriptor];
#else
        descriptor.usage = MTLTextureUsageRenderTarget;
        descriptor.storageMode = MTLStorageModeShared;
        size_t length = __roundUpToMultiple(rawFrame.width * rawFrame.height * sizeof(uint32_t), 4096);
        id<MTLBuffer> indexTextureBuffer = [_device newBufferWithBytesNoCopy:(void *)indexLookups.data()
                                                                      length:length
                                                                     options:MTLResourceStorageModeShared
                                                                 deallocator:^(void * _Nonnull pointer, NSUInteger length) {}];
        // Create a texture backed by this buffer
        _indexTexture = [indexTextureBuffer newTextureWithDescriptor:descriptor
                                                              offset:0
                                                         bytesPerRow:rawFrame.width * sizeof(uint32_t)];
#endif
        _indexTexture.label = @"SurfelIndexMap._indexTexture";
    }
    
    if (_depthTexture == nil || _depthTexture.width != rawFrame.width) {
        MTLTextureDescriptor *descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                                                              width:rawFrame.width
                                                                                             height:rawFrame.height
                                                                                          mipmapped:NO];
        descriptor.usage = MTLTextureUsageRenderTarget;
        descriptor.storageMode = MTLStorageModePrivate;
        _depthTexture = [_device newTextureWithDescriptor:descriptor];
        _depthTexture.label = @"SurfelIndexMap._depthTexture";
    }
    
    id<MTLBuffer> surfelsBuffer = nil;
    if (surfelCount > 0) {
        surfelsBuffer = [_device newBufferWithBytesNoCopy:(void *)&surfels[0]
                                                   length:roundUpToMultiple(sizeof(Surfel) * surfels.size(), 4096)
                                                  options:0
                                              deallocator:NULL];
        surfelsBuffer.label = @"SurfelIndexMap.surfelsBuffer";
    }
    
    MTLRenderPassDescriptor *passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    passDescriptor.colorAttachments[0].texture = _indexTexture;
    passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(EMPTY_SURFEL_INDEX, 0, 0, 0);
    passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    
    passDescriptor.depthAttachment.texture = _depthTexture;
    passDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
    passDescriptor.depthAttachment.storeAction = MTLStoreActionDontCare;
    passDescriptor.depthAttachment.clearDepth = 1.0;
    
    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    id<MTLRenderCommandEncoder> commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
    commandBuffer.label = @"SurfelIndexMap.commandBuffer";
    commandEncoder.label = @"SurfelIndexMap.commandEncoder";
    
    if (surfelCount > 0 && surfelsBuffer != nil) {
        [commandEncoder setRenderPipelineState:_pipelineState];
        [commandEncoder setViewport:(MTLViewport){ 0, 0, (double)rawFrame.width, (double)rawFrame.height, -1, 1 }];
        [commandEncoder setDepthStencilState:_depthStencilState];
        [commandEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
        [commandEncoder setCullMode:MTLCullModeBack];
        [commandEncoder setVertexBuffer:_vertexBuffer offset:0 atIndex:0];
        [commandEncoder setVertexBuffer:_sharedUniformsBuffer offset:0 atIndex:1];
        [commandEncoder setVertexBuffer:surfelsBuffer offset:0 atIndex:2];
        [commandEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:6 instanceCount:surfelCount];
    }
    
    if (surfelCount > 0 && surfelsBuffer == nil) {
        DEBUG_LOG("Failed to render surfel cloud! %zu surfels in buffer %lx, but surfelsBuffer was nil", surfelCount, (unsigned long)&surfels);
        success = false;
    }
    
    [commandEncoder endEncoding];
    
    // Using the blit encoder on MacOS only seems necessary for GPUs with discrete memory (e.g. ATI),
    // and doing so on shared memory GPU architectures (e.g. Intel) breaks things.
    // Although there's no direct "is shared memory" flag, isLowPower does the trick well enough.
#if TARGET_OS_OSX
    if ([_device isLowPower] == NO) {
        id<MTLBlitCommandEncoder> blitEncoder = [commandBuffer blitCommandEncoder];
        [blitEncoder synchronizeTexture:_indexTexture slice:0 level:0];
        [blitEncoder endEncoding];
    }
#endif
    
    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];
    
    // On iOS devices, we've written directly into indexLookups, so no need to copy the result back out
#if TARGET_OS_OSX
    [_indexTexture getBytes:indexLookups.data()
                bytesPerRow:rawFrame.width * sizeof(uint32_t)
                 fromRegion:MTLRegionMake2D(0, 0, rawFrame.width, rawFrame.height)
                mipmapLevel:0];
#endif

//    uint32_t checksum = crc32((void*)indexLookups.data(), indexLookups.size() * sizeof(float));
//    std::cout << "\tsurfel index lookup checksum = " << std::hex << checksum << std::dec << std::endl;
    
    return success;
}

bool MetalSurfelIndexMap::drawForColor(const Surfel* surfels,
                                       size_t surfelCount,
                                       Matrix4f projectionViewMatrix,
                                       size_t frameWidth,
                                       size_t frameHeight,
                                       std::vector<uint32_t>& indexLookups)
{
    // [[MTLCaptureManager sharedCaptureManager] startCaptureWithDevice:_device];
    
    bool success = true;
    
    if (surfelCount == 0 || frameWidth == 0 || frameHeight == 0) { return success; }
    
    if (_indexTexture == nil || _indexTexture.width != frameWidth) {
        MTLTextureDescriptor *descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatR32Uint
                                                                                              width:frameWidth
                                                                                             height:frameHeight
                                                                                          mipmapped:NO];
        descriptor.usage = MTLResourceUsageWrite|MTLTextureUsageRenderTarget;
        _indexTexture = [_device newTextureWithDescriptor:descriptor];
        _indexTexture.label = @"SurfelIndexMap._indexTexture";
    }
    
    if (_depthTexture == nil || _depthTexture.width != frameWidth) {
        MTLTextureDescriptor *descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                                                              width:frameWidth
                                                                                             height:frameHeight
                                                                                          mipmapped:NO];
        descriptor.usage = MTLTextureUsageRenderTarget;
        descriptor.storageMode = MTLStorageModePrivate;
        _depthTexture = [_device newTextureWithDescriptor:descriptor];
        _depthTexture.label = @"SurfelIndexMap._depthTexture";
    }
    
    this->_updateSharedUniformsBufferForColor(projectionViewMatrix);
    
    id<MTLBuffer> surfelsBuffer = nil;
    if (surfelCount > 0) {
        surfelsBuffer = [_device newBufferWithBytesNoCopy:(void *)surfels
                                                   length:roundUpToMultiple(sizeof(Surfel) * surfelCount, 4096)
                                                  options:0
                                              deallocator:NULL];
        surfelsBuffer.label = @"SurfelIndexMap.surfelsBuffer";
    }
    
    MTLRenderPassDescriptor *passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    passDescriptor.colorAttachments[0].texture = _indexTexture;
    passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(EMPTY_SURFEL_INDEX, 0, 0, 0);
    passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    
    passDescriptor.depthAttachment.texture = _depthTexture;
    passDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
    passDescriptor.depthAttachment.storeAction = MTLStoreActionDontCare;
    passDescriptor.depthAttachment.clearDepth = 1.0;
    
    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    id<MTLRenderCommandEncoder> commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
    commandBuffer.label = @"SurfelIndexMap.commandBuffer";
    commandEncoder.label = @"SurfelIndexMap.commandEncoder";
    
    if (surfelCount > 0 && surfelsBuffer != nil) {
        [commandEncoder setRenderPipelineState:_pipelineState];
        [commandEncoder setViewport:(MTLViewport){ 0, 0, (double)frameWidth, (double)frameHeight, -1, 1 }];
        [commandEncoder setDepthStencilState:_depthStencilState];
        [commandEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
        [commandEncoder setCullMode:MTLCullModeBack];
        [commandEncoder setVertexBuffer:_vertexBuffer offset:0 atIndex:0];
        [commandEncoder setVertexBuffer:_sharedUniformsBuffer offset:0 atIndex:1];
        [commandEncoder setVertexBuffer:surfelsBuffer offset:0 atIndex:2];
        [commandEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:6 instanceCount:surfelCount];
    }
    
    if (surfelCount > 0 && surfelsBuffer == nil) {
        DEBUG_LOG("Failed to render surfel cloud! %zu surfels in buffer %lx, but surfelsBuffer was nil", surfelCount, (unsigned long)&surfels);
        success = false;
    }
    
    [commandEncoder endEncoding];
    
    // Using the blit encoder on MacOS only seems necessary for GPUs with discrete memory (e.g. ATI),
    // and doing so on shared memory GPU architectures (e.g. Intel) breaks things.
    // Although there's no direct "is shared memory" flag, isLowPower does the trick well enough.
#if TARGET_OS_OSX
    if ([_device isLowPower] == NO) {
        id<MTLBlitCommandEncoder> blitEncoder = [commandBuffer blitCommandEncoder];
        [blitEncoder synchronizeTexture:_indexTexture slice:0 level:0];
        [blitEncoder endEncoding];
    }
#endif
    
    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];
    
    [_indexTexture getBytes:indexLookups.data()
                bytesPerRow:frameWidth * sizeof(uint32_t)
                 fromRegion:MTLRegionMake2D(0, 0, frameWidth, frameHeight)
                mipmapLevel:0];
    
    // [[MTLCaptureManager sharedCaptureManager] stopCapture];
    
    return success;
}

id<MTLTexture> MetalSurfelIndexMap::getDepthTexture() {
    return _depthTexture;
}

id<MTLTexture> MetalSurfelIndexMap::getIndexTexture() {
    return _indexTexture;
}

Matrix4f MetalSurfelIndexMap::getViewProjectionMatrix() {
    return toMatrix4f(_lastViewProjectionMatrix);
}

// MARK: - Private

id<MTLBuffer> MetalSurfelIndexMap::_createVertexBuffer() {
    static const float INV_RT3 = 1.0f / sqrtf(3.0f);
    
    static const SurfelIndexMapVertex kHexVertices[] = {
        SurfelIndexMapVertex(-2.0f * INV_RT3,  0.0),
        SurfelIndexMapVertex(       -INV_RT3, -1.0),
        SurfelIndexMapVertex(       -INV_RT3,  1.0),
        SurfelIndexMapVertex(        INV_RT3, -1.0),
        SurfelIndexMapVertex(        INV_RT3,  1.0),
        SurfelIndexMapVertex( 2.0f * INV_RT3,  0.0),
    };
    
    return [_device newBufferWithBytes:kHexVertices length:sizeof(kHexVertices) options:0];
}

void MetalSurfelIndexMap::_updateSharedUniformsBuffer(const RawFrame& frame, const Matrix4f& modelMatrix) {
    SurfelIndexMapSharedUniforms sharedUniforms(frame, modelMatrix);
    memcpy([_sharedUniformsBuffer contents], &sharedUniforms, sizeof(SurfelIndexMapSharedUniforms));
    
    _lastViewProjectionMatrix = sharedUniforms.projectionViewMatrix;
}

void MetalSurfelIndexMap::_updateSharedUniformsBufferForColor(Matrix4f viewProjection) {
    SurfelIndexMapSharedUniforms sharedUniforms(viewProjection);
    sharedUniforms.surfelAliasingSafetyFactor = 1.5;
    memcpy([_sharedUniformsBuffer contents], &sharedUniforms, sizeof(SurfelIndexMapSharedUniforms));
}

NS_ASSUME_NONNULL_END
