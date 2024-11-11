//
//  MetalTextureProjection.cpp
//  StandardCyborgFusion
//
//  Created by eric on 2019-09-03.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#include "MetalTextureProjection.hpp"

using namespace standard_cyborg;

MetalTextureProjection::MetalTextureProjection(id<MTLDevice> device,
                                               int textureResolution)
{
    _device = device;

    NSError* error;

    _library = [_device newDefaultLibraryWithBundle:[NSBundle bundleWithIdentifier:@"com.standardcyborg.StandardCyborgFusion"] error:&error];
    if (_library == nil) { NSLog(@"Unable to create library: %@", error); }

    _textureResolution = textureResolution;
    _clearPassNan = [[ClearPassNan alloc] init];
    _renderUvs = [[RenderUvs alloc] initWithDevice:_device library:_library];
    _renderPositions = [[RenderPositions alloc] initWithDevice:_device library:_library];

    _commandQueue = [_device newCommandQueue];
    _computeKernel = [_library newFunctionWithName:@"uvToColor"];
    _computeKernel.label = @"uvToColor";
    _computePipelineState = [_device newComputePipelineStateWithFunction:_computeKernel error:NULL];

    if (_computePipelineState == nil) {
        // Compute pipeline State creation could fail if kernelFunction failed to load from the
        // library.  If the Metal API validation is enabled, we automatically be given more
        // information about what went wrong.  (Metal API validation is enabled by default
        // when a debug build is run from Xcode)
        NSLog(@"Failed to create compute pipeline state, error %@", error);
        exit(1);
    }
}

bool MetalTextureProjection::startProjecting(int frameWidth,
                                             int frameHeight,
                                             const sc3d::Geometry& triangleMesh,
                                             const sc3d::PerspectiveCamera& camera)
{
    _camera = camera;
    
    {
        MTLTextureDescriptor* descriptor = [MTLTextureDescriptor
            texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA32Float
                                         width:_textureResolution
                                        height:_textureResolution
                                     mipmapped:NO];

        descriptor.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
        _targetTexture = [_device newTextureWithDescriptor:descriptor];
        _targetTexture.label = @"MetalTextureProjection._targetTexture";
    }
    
    {
        MTLTextureDescriptor* descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA32Float
                                                                                              width:_textureResolution
                                                                                             height:_textureResolution
                                                                                          mipmapped:NO];

        descriptor.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite | MTLTextureUsageRenderTarget;
        _positionsTexture = [_device newTextureWithDescriptor:descriptor];
        _positionsTexture.label = @"MetalTextureProjection._positionsTexture";
    }

    {
        MTLTextureDescriptor* descriptor = [MTLTextureDescriptor
            texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA32Float
                                         width:frameWidth
                                        height:frameHeight
                                     mipmapped:NO];

        descriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
        _uvTexture = [_device newTextureWithDescriptor:descriptor];
        _uvTexture.label = @"MetalTextureProjection._uvTexture";
    }

    {
        MTLTextureDescriptor* descriptor = [MTLTextureDescriptor
            texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA32Float
                                         width:frameWidth
                                        height:frameHeight
                                     mipmapped:NO];

        descriptor.usage = MTLTextureUsageShaderRead;
        _frameTexture = [_device newTextureWithDescriptor:descriptor];
        _frameTexture.label = @"MetalTextureProjection._frameTexture";
    }

    {
        MTLTextureDescriptor* descriptor = [MTLTextureDescriptor
            texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                         width:frameWidth
                                        height:frameHeight
                                     mipmapped:NO];

        descriptor.usage = MTLTextureUsageRenderTarget;
        descriptor.storageMode = MTLStorageModePrivate;
        _depthTexture = [_device newTextureWithDescriptor:descriptor];
        _depthTexture.label = @"MetalTextureProjection.depthTexture";
    }

    {
        id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
        commandBuffer.label = @"MetalTextureProjection.commandBuffer";

        [_renderPositions encodeCommandsWithDevice:_device
                                     commandBuffer:commandBuffer
                                      triangleMesh:triangleMesh
                                       intoTexture:_positionsTexture];

        [commandBuffer commit];
        [commandBuffer waitUntilCompleted];
    }
    
    return true;
}

std::vector<float> MetalTextureProjection::finishProjecting(const sc3d::Geometry& triangleMesh)
{
#if TARGET_OS_OSX
    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    commandBuffer.label = @"MetalTextureProjection.commandBuffer";

    if ([_device isLowPower] == NO) {
        id<MTLBlitCommandEncoder> blitEncoder = [commandBuffer blitCommandEncoder];
        [blitEncoder synchronizeTexture:_targetTexture slice:0 level:0];
        [blitEncoder synchronizeTexture:_positionsTexture slice:0 level:0];

        [blitEncoder endEncoding];

        [commandBuffer commit];
        [commandBuffer waitUntilCompleted];
    }
#endif
    std::vector<float> textureData(4 * _textureResolution * _textureResolution);
    [_targetTexture getBytes:textureData.data()
                 bytesPerRow:_textureResolution * sizeof(float) * 4
                  fromRegion:MTLRegionMake2D(0, 0, _textureResolution, _textureResolution)
                 mipmapLevel:0];
    
    // Nil out the target texture to free up memory now
    _targetTexture = nil;
    
    std::vector<float> positionsTextureData(4 * _textureResolution * _textureResolution);
    [_positionsTexture getBytes:positionsTextureData.data()
                    bytesPerRow:_textureResolution * sizeof(float) * 4
                     fromRegion:MTLRegionMake2D(0, 0, _textureResolution, _textureResolution)
                    mipmapLevel:0];
    
    // Nil out the positions texture to free up memory now
    _positionsTexture = nil;
    
    // Divide all colors by total weight, thus normalizing the weights
    for (int row = 0; row < _textureResolution; ++row) {
        for (int col = 0; col < _textureResolution * 4; col += 4) {
            size_t textureIndex = _textureResolution * row * 4 + col;
            
            float r = textureData[textureIndex + 0];
            float g = textureData[textureIndex + 1];
            float b = textureData[textureIndex + 2];
            float a = textureData[textureIndex + 3];

            if (r == 0.0f && g == 0.0f && b == 0.0f && a == 0.0f) {
                // this pixel was never projected to. nan. we will deal with it later.
                textureData[textureIndex + 0] = std::nan("a");
                textureData[textureIndex + 1] = 0.0f;
                textureData[textureIndex + 2] = 0.0f;
                textureData[textureIndex + 3] = 1.0f;

            } else {
                textureData[textureIndex + 0] = r / a;
                textureData[textureIndex + 1] = g / a;
                textureData[textureIndex + 2] = b / a;
                textureData[textureIndex + 3] = 1.0f;
            }
        }
    }
    
    std::vector<math::Vec3> filteredPositions;
    std::vector<math::Vec2> filteredTexCoords;

    {
        const std::vector<math::Vec2>& originalTexCoords = triangleMesh.getTexCoords();
        const std::vector<math::Vec3>& originalPositions = triangleMesh.getPositions();

        for (int iv = 0; iv < originalPositions.size(); ++iv) {
            int uvCol = originalTexCoords[iv].x * _textureResolution;
            int uvRow = originalTexCoords[iv].y * _textureResolution;

            math::Vec3 col;

            col.x = textureData[_textureResolution * uvRow * 4 + uvCol * 4 + 0];
            col.y = textureData[_textureResolution * uvRow * 4 + uvCol * 4 + 1];
            col.z = textureData[_textureResolution * uvRow * 4 + uvCol * 4 + 2];

            if (std::isnan(col.x)) {
            } else {
                filteredPositions.push_back(originalPositions[iv]);
                filteredTexCoords.push_back(originalTexCoords[iv]);
            }
        }
    }
    
    sc3d::Geometry filteredGeo(filteredPositions);
    filteredGeo.setTexCoords(filteredTexCoords);
    
    /// Fill in holes where nothing was projected to
    _fillTextureHoles(filteredGeo, positionsTextureData, textureData);
    
    // To make sure texture seams aren't visible, pad all charts by one pixel.
    // We repeat this padding several iterations.
    // Otherwise, the seams will still be visible when mipmaps are used.
    _padTextureEdges(textureData);
    
    // Flip texture horizonally
    for (int row = 0; row < _textureResolution / 2; ++row) {
        for (int col = 0; col < _textureResolution * 4; col += 1) {
            float temp = textureData[_textureResolution * row * 4 + col + 0];
            textureData[_textureResolution * row * 4 + col + 0] = textureData[_textureResolution * (_textureResolution - 1 - row) * 4 + col + 0];
            textureData[_textureResolution * (_textureResolution - 1 - row) * 4 + col + 0] = temp;
        }
    }

    return textureData;
}

struct Uniforms {
    float textureResolution;

    unsigned int frameWidth;
    unsigned int frameHeight;
};

bool MetalTextureProjection::projectSingleTexture(const matrix_float4x4& viewMatrix, const matrix_float4x4& projectionMatrix, const sc3d::ColorImage& frame, const sc3d::Geometry& triangleMesh)
{
    if (triangleMesh.getTexCoords().size() == 0) {
        return false;
    }

    const std::vector<math::Vec2>& texCoords = triangleMesh.getTexCoords();
    for (const math::Vec2& texCoord : texCoords) {
        float eps = 0.0000001f;
        if (texCoord.x < 0.0 - eps || texCoord.x > 1.0 + eps || texCoord.y < 0.0 - eps || texCoord.y > 1.0 + eps) {
            printf("// (%f, %f) out of range\n", texCoord.x, texCoord.y);
            return false;
        }
    }

    MTLRegion region = MTLRegionMake2D(0, 0, frame.getWidth(), frame.getHeight());

    [_frameTexture replaceRegion:region
                     mipmapLevel:0
                       withBytes:frame.getData().data()
                     bytesPerRow:frame.getWidth() * 4 * sizeof(float)];

    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    commandBuffer.label = @"MetalTextureProjection.commandBuffer";

    [_clearPassNan encodeCommandsWithDevice:_device
                              commandBuffer:commandBuffer
                                intoTexture:_uvTexture
                               depthTexture:_depthTexture];

    [_renderUvs encodeCommandsWithDevice:_device
                           commandBuffer:commandBuffer
                                  camera:_camera
                            triangleMesh:triangleMesh
                              viewMatrix:viewMatrix
                        projectionMatrix:projectionMatrix
                             intoTexture:_uvTexture
                            depthTexture:_depthTexture];


    /*
     With that, we have rendered the triangle mesh to uvTexture.
     As a result, uvTexture will, in each pixel, contain the uv coords
     of the fragment that was rendered into that pixel.
     Plus, it also contains a weight value, in the alpha channel,
     based on the angle between the camera viewing direction, and
     the normal of the fragment.
     
     If a pixel was never rendered to, it will just contain NaN instead.
     */

    MTLSize threadgroupCounts = MTLSizeMake(8, 8, 1);
    MTLSize threadgroups = MTLSizeMake(frame.getWidth() / threadgroupCounts.width,
                                       frame.getHeight() / threadgroupCounts.height,
                                       1);

    Uniforms uniforms;
    uniforms.textureResolution = _textureResolution;
    uniforms.frameWidth = frame.getWidth();
    uniforms.frameHeight = frame.getHeight();
    
    id<MTLComputeCommandEncoder> computeCommandEncoder = [commandBuffer computeCommandEncoder];
    
    [computeCommandEncoder setComputePipelineState:_computePipelineState];
    [computeCommandEncoder setTexture:_uvTexture atIndex:0];
    [computeCommandEncoder setTexture:_frameTexture atIndex:1];
    [computeCommandEncoder setTexture:_targetTexture atIndex:2];
    [computeCommandEncoder setBytes:&uniforms length:sizeof(Uniforms) atIndex:0];
    [computeCommandEncoder dispatchThreadgroups:threadgroups threadsPerThreadgroup:threadgroupCounts];
    [computeCommandEncoder endEncoding];

    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];

    return true;
}

void MetalTextureProjection::_fillTextureHoles(const sc3d::Geometry &filteredGeo,
                                               std::vector<float> &positionsTextureData,
                                               std::vector<float> &textureData)
{
    if (filteredGeo.vertexCount() == 0) { return; }
    
    std::vector<float> tempTextureData = textureData;
    
    for (int row = 0; row < _textureResolution; ++row) {
        for (int col = 0; col < _textureResolution; ++col) {
            size_t tempTextureIndex = 4 * (_textureResolution * row + col);
            
            math::Vec4 color = _getColor(col, row, textureData);
            
            if (std::isnan(color.x)) {
                math::Vec4 unfilledColor(1.0f, 0.0f, 0.0f, 1.0f);
                
                math::Vec3 p(positionsTextureData[_textureResolution * (_textureResolution - 1 - row) * 4 + col * 4 + 0],
                                       positionsTextureData[_textureResolution * (_textureResolution - 1 - row) * 4 + col * 4 + 1],
                                       positionsTextureData[_textureResolution * (_textureResolution - 1 - row) * 4 + col * 4 + 2]);
                
                if (!std::isnan(p.x) && !std::isnan(p.y) && !std::isnan(p.z)) {
                    int closestIndex = filteredGeo.getClosestVertexIndex(p);
                    
                    int uvCol = filteredGeo.getTexCoords()[closestIndex].x * _textureResolution;
                    int uvRow = filteredGeo.getTexCoords()[closestIndex].y * _textureResolution;
                    
                    unfilledColor.x = textureData[_textureResolution * uvRow * 4 + uvCol * 4 + 0];
                    unfilledColor.y = textureData[_textureResolution * uvRow * 4 + uvCol * 4 + 1];
                    unfilledColor.z = textureData[_textureResolution * uvRow * 4 + uvCol * 4 + 2];
                    
                    tempTextureData[tempTextureIndex + 0] = unfilledColor.x;
                    tempTextureData[tempTextureIndex + 1] = unfilledColor.y;
                    tempTextureData[tempTextureIndex + 2] = unfilledColor.z;
                    tempTextureData[tempTextureIndex + 3] = unfilledColor.w;
                } else {
                    tempTextureData[tempTextureIndex + 0] = color.x;
                    tempTextureData[tempTextureIndex + 1] = color.y;
                    tempTextureData[tempTextureIndex + 2] = color.z;
                    tempTextureData[tempTextureIndex + 3] = color.w;
                }
            } else {
                tempTextureData[tempTextureIndex + 0] = color.x;
                tempTextureData[tempTextureIndex + 1] = color.y;
                tempTextureData[tempTextureIndex + 2] = color.z;
                tempTextureData[tempTextureIndex + 3] = color.w;
            }
        }
    }
    
    textureData = tempTextureData;
}

void MetalTextureProjection::_padTextureEdges(std::vector<float> &textureData)
{
    const int ITERATIONS = 16;
    
    for (int iteration = 0; iteration < ITERATIONS; ++iteration) {
        std::vector<float> tempTextureData = textureData;
        
        for (int row = 0; row < _textureResolution; ++row) {
            for (int col = 0; col < _textureResolution; col += 1) {
                size_t textureIndex = _textureResolution * row * 4 + col * 4;
                
                math::Vec4 color = _getColor(col, row, textureData);
                
                if (std::isnan(color.x)) {
                    math::Vec4 upColor = _getColor(col, row - 1, textureData);
                    math::Vec4 leftColor = _getColor(col + 1, row, textureData);
                    math::Vec4 rightColor = _getColor(col - 1, row, textureData);
                    math::Vec4 downColor = _getColor(col, row + 1, textureData);
                    
                    if (!std::isnan(upColor.x)) {
                        color = upColor;
                    } else if (!std::isnan(downColor.x)) {
                        color = downColor;
                    } else if (!std::isnan(leftColor.x)) {
                        color = leftColor;
                    } else if (!std::isnan(rightColor.x)) {
                        color = rightColor;
                    } else if (iteration + 1 != ITERATIONS) {
                        // A no-op
                        // color = color;
                    }
                }
                
                tempTextureData[textureIndex + 0] = color.x;
                tempTextureData[textureIndex + 1] = color.y;
                tempTextureData[textureIndex + 2] = color.z;
                tempTextureData[textureIndex + 3] = color.w;
            }
        }
        
        textureData = tempTextureData;
    }
}

inline math::Vec4 MetalTextureProjection::_getColor(int col, int row, const std::vector<float>& textureData)
{
    int r = row;
    r = MAX(0, row);
    r = MIN(r, _textureResolution - 1);
    
    int c = col;
    c = MAX(0, c);
    c = MIN(c, _textureResolution - 1);
    
    size_t colorIndex = 4 * (_textureResolution * r + c);
    
    math::Vec4 color(
        textureData[colorIndex + 0],
        textureData[colorIndex + 1],
        textureData[colorIndex + 2],
        textureData[colorIndex + 3]);

    return color;
}
