//
//  DrawCorrespondences.mm
//  VisualTesterMac
//
//  Created by Aaron Thompson on 9/17/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import "DrawCorrespondences.hpp"
#import <simd/simd.h>
#import <GLKit/GLKMatrix4.h>

NS_ASSUME_NONNULL_BEGIN

@implementation DrawCorrespondences {
    id<MTLDevice> _device;
    id<MTLLibrary> _library;
    id<MTLRenderPipelineState> _pipelineState;
    id<MTLBuffer> _sharedUniformsBuffer;
    id<MTLDepthStencilState> _depthStencilState;
}

struct Vertex {
    float position[3];
    float color;
};

struct SharedUniforms {
    matrix_float4x4 projection;
    matrix_float4x4 view;
    matrix_float4x4 model;
    
    SharedUniforms(matrix_float4x4 projectionIn, matrix_float4x4 viewIn, matrix_float4x4 modelIn) :
    projection(projectionIn),
    view(viewIn),
    model(modelIn)
    { }
};

// MARK: - MetalVisualization

- (instancetype)initWithDevice:(id<MTLDevice>)device library:(id<MTLLibrary>)library
{
    self = [super init];
    if (self) {
        _device = device;
        
        id<MTLFunction> vertexFunction = [library newFunctionWithName:@"DrawCorrespondencesVertex"];
        id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"DrawCorrespondencesFragment"];
        
        MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
        vertexDescriptor.attributes[0].format = MTLVertexFormatFloat3;
        vertexDescriptor.attributes[0].offset = offsetof(Vertex, position);
        vertexDescriptor.attributes[1].format = MTLVertexFormatFloat;
        vertexDescriptor.attributes[1].offset = offsetof(Vertex, color);
        vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
        vertexDescriptor.layouts[0].stride = sizeof(Vertex);
        
        MTLRenderPipelineDescriptor *pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineDescriptor.vertexFunction = vertexFunction;
        pipelineDescriptor.fragmentFunction = fragmentFunction;
        pipelineDescriptor.vertexDescriptor = vertexDescriptor;
        pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
        pipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
        
        MTLDepthStencilDescriptor *depthStencilDescriptor = [[MTLDepthStencilDescriptor alloc] init];
        depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionLess;
        depthStencilDescriptor.depthWriteEnabled = YES;
        _depthStencilState = [_device newDepthStencilStateWithDescriptor:depthStencilDescriptor];
        
        NSError *error;
        _pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];
        if (_pipelineState == nil) { NSLog(@"Unable to create pipeline state: %@", error); }
        
        _sharedUniformsBuffer = [device newBufferWithLength:sizeof(SharedUniforms)
                                                    options:MTLResourceOptionCPUCacheModeWriteCombined];
        _sharedUniformsBuffer.label = @"DrawCorrespondences._sharedUniformsBuffer";
    }
    return self;
}

- (void)encodeCommandsWithDevice:(id<MTLDevice>)device
                   commandBuffer:(id<MTLCommandBuffer>)commandBuffer
                         surfels:(const Surfels&)surfels
                       icpResult:(ICPResult&)icpResult
                      viewMatrix:(matrix_float4x4)viewMatrix
                projectionMatrix:(matrix_float4x4)projectionMatrix
                     intoTexture:(id<MTLTexture>)texture
                    depthTexture:(id<MTLTexture>)depthTexture
{
    if (!_enabled) return;
    
    // icpResult.sourceVertices and .targetVertices is only defined and saved in debug builds
#if DEBUG
    size_t vertexCount = 0;
    id<MTLBuffer> vertexBuffer = [self _createVertexBufferFromSourceVertices:icpResult.sourceVertices
                                                              targetVertices:icpResult.targetVertices
                                                              getVertexCount:&vertexCount];
    [self _updateSharedUniformsBufferWithViewMatrix:viewMatrix
                                   projectionMatrix:projectionMatrix];
    
    MTLRenderPassDescriptor *passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    passDescriptor.colorAttachments[0].texture = texture;
    passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    passDescriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
    passDescriptor.depthAttachment.texture = depthTexture;
    passDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
    passDescriptor.depthAttachment.loadAction = MTLLoadActionLoad;
    
    id<MTLRenderCommandEncoder> commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
    commandEncoder.label = @"DrawCorrespondences.commandEncoder";
    
    if (vertexCount > 0) {
        [commandEncoder setRenderPipelineState:_pipelineState];
        [commandEncoder setViewport:(MTLViewport){ 0, 0, (double)texture.width, (double)texture.height, -1, 1 }];
        [commandEncoder setDepthStencilState:_depthStencilState];
        [commandEncoder setCullMode:MTLCullModeNone];
        [commandEncoder setVertexBuffer:vertexBuffer offset:0 atIndex:0];
        [commandEncoder setVertexBuffer:_sharedUniformsBuffer offset:0 atIndex:1];
        [commandEncoder drawPrimitives:MTLPrimitiveTypeLine vertexStart:0 vertexCount:vertexCount];
    }
    
    [commandEncoder endEncoding];
#endif // DEBUG
}

// MARK: - Private

- (id<MTLBuffer> _Nullable)_createVertexBufferFromSourceVertices:(std::shared_ptr<std::vector<math::Vec3>>)sourceVertices
                                                  targetVertices:(std::shared_ptr<std::vector<math::Vec3>>)targetVertices
                                                  getVertexCount:(size_t *)vertexCountOut
{
    size_t vertexCount = 0;
    id<MTLBuffer> result = nil;
    
    if (sourceVertices != nullptr && targetVertices != nullptr) {
        vertexCount = MIN(sourceVertices->size(), targetVertices->size());
    }
    
    if (vertexCount > 0) {
        Vertex vertices[vertexCount * 2];
        
        for (size_t i = 0; i < vertexCount; ++i) {
            math::Vec3 source = (*sourceVertices)[i];
            math::Vec3 ref = (*targetVertices)[i];
            vertices[i * 2 + 0] = { source.x, source.y, source.z, 0 };
            vertices[i * 2 + 1] = { ref.x, ref.y, ref.z, 1 };
        }
        
        result = [_device newBufferWithBytes:(void *)&vertices
                                      length:sizeof(Vertex) * vertexCount * 2
                                     options:MTLResourceCPUCacheModeWriteCombined];
    }
    
    *vertexCountOut = vertexCount * 2;
    return result;
}

- (void)_updateSharedUniformsBufferWithViewMatrix:(matrix_float4x4)view
                                 projectionMatrix:(matrix_float4x4)projection {
    matrix_float4x4 model = matrix_identity_float4x4;
    
    SharedUniforms sharedUniforms(projection, view, model);
    memcpy([_sharedUniformsBuffer contents], &sharedUniforms, sizeof(SharedUniforms));
}

@end

NS_ASSUME_NONNULL_END
