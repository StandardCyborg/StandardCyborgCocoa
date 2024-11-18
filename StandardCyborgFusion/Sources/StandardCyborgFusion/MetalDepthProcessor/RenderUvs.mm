//
//  RenderUvs.mm
//  VisualTesterMac
//
//  Created by Eric on 8/31/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <CoreGraphics/CoreGraphics.h>
#import <GLKit/GLKMatrix4.h>
#import <standard_cyborg/math/Vec4.hpp>

#import "GeometryHelpers.hpp"
#import "RenderUvs.hpp"

NS_ASSUME_NONNULL_BEGIN

using namespace standard_cyborg;


static inline size_t __roundUpToMultiple(size_t value, size_t multiple) {
    return ((value + multiple - 1) / multiple) * multiple;
}

@implementation RenderUvs {
    id<MTLDevice> _device;
    id<MTLLibrary> _library;
    id<MTLRenderPipelineState> _pipelineState;
    id<MTLBuffer> _sharedUniformsBuffer;
    id<MTLDepthStencilState> _depthStencilState;
}

struct Vertex {
    math::Vec4 pos;
    math::Vec4 color;
    math::Vec4 texCoord;
    math::Vec4 normal;
    
    Vertex(math::Vec4 pos_, math::Vec4 color_, math::Vec4 texCoord_, math::Vec4 normal_)
    {
        pos = pos_;
        color = color_;
        texCoord = texCoord_;
        normal = normal_;
    }
};

struct SharedUniforms {
    simd_float4x4 projectionView; // precomputed projection * view
    simd_float4x4 orientationView; // precomputed orientation * view
    
    simd_float4 inverseLensDistortionConstants;
    simd_float2 opticalImageSize;
    simd_float2 opticalImageCenter;
    float maxLensCalibrationRadius;
    
    SharedUniforms(simd_float4x4 projection,
                   simd_float4x4 orientation,
                   simd_float4x4 view,
                   const sc3d::PerspectiveCamera& camera)
    {
        projectionView = simd_mul(projection, view);
        orientationView = simd_mul(orientation, view);
        
        inverseLensDistortionConstants = toSimdFloat4(camera.getInverseLensDistortionCurveFit());
        opticalImageSize = toSimdFloat2(camera.getIntrinsicMatrixReferenceSize());
        opticalImageCenter = toSimdFloat2(camera.getOpticalImageCenter());
        maxLensCalibrationRadius = camera.getOpticalImageMaxRadius();
    }
};

#pragma mark - MetalVisualization

- (instancetype)initWithDevice:(id<MTLDevice>)device library:(id<MTLLibrary>)library
{
    self = [super init];
    if (self) {
        _device = device;
        
        id<MTLFunction> vertexFunction = [library newFunctionWithName:@"RenderUvsVertex"];
        id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"RenderUvsFragment"];
        
        MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
        vertexDescriptor.attributes[0].format = MTLVertexFormatFloat4;
        vertexDescriptor.attributes[0].offset = 0;
        vertexDescriptor.attributes[0].bufferIndex = 0;
        
        vertexDescriptor.attributes[1].format = MTLVertexFormatFloat4;
        vertexDescriptor.attributes[1].offset = sizeof(math::Vec4);
        vertexDescriptor.attributes[1].bufferIndex = 0;
        
        vertexDescriptor.attributes[2].format = MTLVertexFormatFloat4;
        vertexDescriptor.attributes[2].offset = sizeof(math::Vec4) + sizeof(math::Vec4);
        vertexDescriptor.attributes[2].bufferIndex = 0;
        
        vertexDescriptor.attributes[3].format = MTLVertexFormatFloat4;
        vertexDescriptor.attributes[3].offset = sizeof(math::Vec4) + sizeof(math::Vec4) + sizeof(math::Vec4);
        vertexDescriptor.attributes[3].bufferIndex = 0;
        
        vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
        vertexDescriptor.layouts[0].stride = sizeof(Vertex);
        
        MTLRenderPipelineDescriptor *pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineDescriptor.vertexFunction = vertexFunction;
        pipelineDescriptor.fragmentFunction = fragmentFunction;
        pipelineDescriptor.vertexDescriptor = vertexDescriptor;
        pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatRGBA32Float;
        pipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
        
        MTLDepthStencilDescriptor *depthStencilDescriptor = [[MTLDepthStencilDescriptor alloc] init];
        depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionLess;
        depthStencilDescriptor.depthWriteEnabled = YES;
        _depthStencilState = [_device newDepthStencilStateWithDescriptor:depthStencilDescriptor];
        
        NSError *error;
        _pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];
        if (_pipelineState == nil) { NSLog(@"Unable to create pipeline state: %@", error); }
        
        _sharedUniformsBuffer = [device newBufferWithLength:sizeof(SharedUniforms) options:MTLResourceOptionCPUCacheModeWriteCombined];
        _sharedUniformsBuffer.label = @"RenderUvs.sharedUniforms";
    }
    return self;
}


- (void)encodeCommandsWithDevice:(id<MTLDevice>)device
                   commandBuffer:(id<MTLCommandBuffer>)commandBuffer
                          camera:(sc3d::PerspectiveCamera)camera
                    triangleMesh:(const sc3d::Geometry&)triangleMesh
                      viewMatrix:(simd_float4x4)viewMatrix
                projectionMatrix:(simd_float4x4)projectionMatrix

                     intoTexture:(id<MTLTexture>)texture
                    depthTexture:(id<MTLTexture>)depthTexture
{
    [self _updateSharedUniformsBufferWithViewMatrix:viewMatrix
                                   projectionMatrix:projectionMatrix
                                             camera:camera];
    
    id<MTLBuffer> vertexBuffer;
    __block std::shared_ptr<std::vector<Vertex>> vertices(new std::vector<Vertex>());
    {
        const std::vector<math::Vec3>& positions = triangleMesh.getPositions();
        const std::vector<math::Vec3>& colors = triangleMesh.getColors();
        const std::vector<math::Vec2>& texCoords = triangleMesh.getTexCoords();
        const std::vector<math::Vec3>& normals = triangleMesh.getNormals();
        
        vertices->reserve(triangleMesh.vertexCount());
        
        for (int iv = 0; iv < triangleMesh.vertexCount(); ++iv) {
            vertices->push_back(Vertex{
                {positions[iv].x, positions[iv].y, positions[iv].z, 0.0f},
                {colors[iv].x, colors[iv].y, colors[iv].z, 0.0f},
                {texCoords[iv].x, texCoords[iv].y, 0.0, 0.0},
                {normals[iv].x, normals[iv].y, normals[iv].z, 0.0f},
            });
        }
        
        vertexBuffer = [_device newBufferWithBytesNoCopy:(void *)vertices->data()
                                                  length:__roundUpToMultiple(sizeof(Vertex) * vertices->size(), 4096)
                                                 options:0
                                             deallocator:^(void *pointer, NSUInteger length) {
                                                 vertices = nullptr;
                                             }];
    }
    
    id<MTLBuffer> indexBuffer;
    __block std::shared_ptr<std::vector<int>> indices(new std::vector<int>());
    {
        indices->reserve(triangleMesh.faceCount());
        const std::vector<sc3d::Face3>& faces = triangleMesh.getFaces();
        
        for (const sc3d::Face3 &face : faces) {
            indices->push_back(face[0]);
            indices->push_back(face[1]);
            indices->push_back(face[2]);
        }
        
        indexBuffer = [_device newBufferWithBytesNoCopy:(void *)indices->data()
                                                 length:__roundUpToMultiple(sizeof(int) * indices->size(), 4096)
                                                options:0
                                            deallocator:^(void *pointer, NSUInteger length) {
                                                indices = nullptr;
                                            }];
    }
    
    
    MTLRenderPassDescriptor *passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    passDescriptor.colorAttachments[0].texture = texture;
    passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    passDescriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
    passDescriptor.depthAttachment.texture = depthTexture;
    passDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
    passDescriptor.depthAttachment.loadAction = MTLLoadActionLoad;
    
    id<MTLRenderCommandEncoder> commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
    commandEncoder.label = @"RenderUvs.commandEncoder";
    
    [commandEncoder setRenderPipelineState:_pipelineState];
    [commandEncoder setViewport:(MTLViewport){0, 0, (double)texture.width, (double)texture.height, -1, 1}];
    [commandEncoder setDepthStencilState:_depthStencilState];
    [commandEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
    [commandEncoder setCullMode:MTLCullModeNone];
    [commandEncoder setVertexBuffer:vertexBuffer offset:0 atIndex:0];
    [commandEncoder setVertexBuffer:_sharedUniformsBuffer offset:0 atIndex:1];
    [commandEncoder setFragmentBuffer:_sharedUniformsBuffer offset:0 atIndex:1];
    
    [commandEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                               indexCount:3 * triangleMesh.faceCount()
                                indexType:MTLIndexTypeUInt32
                              indexBuffer:indexBuffer
                        indexBufferOffset:0];
    
    [commandEncoder endEncoding];
}

#pragma mark - Private

- (void)_updateSharedUniformsBufferWithViewMatrix:(simd_float4x4)viewMatrix
                                 projectionMatrix:(simd_float4x4)projectionMatrix
                                           camera:(sc3d::PerspectiveCamera)camera
{
    simd_float4x4 orientationMatrix = toSimdFloat4x4(camera.getViewMatrix());
    
    SharedUniforms sharedUniforms(projectionMatrix, orientationMatrix, viewMatrix, camera);
    memcpy([_sharedUniformsBuffer contents], &sharedUniforms, sizeof(SharedUniforms));
}

@end

NS_ASSUME_NONNULL_END
