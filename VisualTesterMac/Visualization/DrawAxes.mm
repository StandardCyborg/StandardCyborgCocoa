//
//  DrawAxes.mm
//  VisualTesterMac
//
//  Created by Ricky Reusser on 8/31/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <CoreGraphics/CoreGraphics.h>
#import <GLKit/GLKMatrix4.h>
#import "DrawAxes.hpp"
#import "MathHelpers.h"

NS_ASSUME_NONNULL_BEGIN

@implementation DrawAxes {
    id<MTLDevice> _device;
    id<MTLLibrary> _library;
    id<MTLRenderPipelineState> _pipelineState;
    id<MTLBuffer> _vertexBuffer;
    id<MTLBuffer> _sharedUniformsBuffer;
    id<MTLDepthStencilState> _depthStencilState;
}

struct Vertex {
    simd_float3 position;
    simd_float3 color;
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
        
        id<MTLFunction> vertexFunction = [library newFunctionWithName:@"DrawAxesVertex"];
        id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"DrawAxesFragment"];
        
        MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
        
        vertexDescriptor.attributes[0].format = MTLVertexFormatFloat3;
        vertexDescriptor.attributes[0].offset = 0;
        vertexDescriptor.attributes[0].bufferIndex = 0;
        
        vertexDescriptor.attributes[1].format = MTLVertexFormatFloat3;
        vertexDescriptor.attributes[1].offset = offsetof(Vertex, color);
        vertexDescriptor.attributes[1].bufferIndex = 0;
        
        vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
        vertexDescriptor.layouts[0].stride = sizeof(Vertex);

        MTLDepthStencilDescriptor *depthStencilDescriptor = [[MTLDepthStencilDescriptor alloc] init];
        depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionLess;
        depthStencilDescriptor.depthWriteEnabled = YES;
        _depthStencilState = [_device newDepthStencilStateWithDescriptor:depthStencilDescriptor];

        MTLRenderPipelineDescriptor *pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineDescriptor.vertexFunction = vertexFunction;
        pipelineDescriptor.fragmentFunction = fragmentFunction;
        pipelineDescriptor.vertexDescriptor = vertexDescriptor;
        pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
        pipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;

        NSError *error;
        _pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];
        if (_pipelineState == nil) { NSLog(@"Unable to create pipeline state: %@", error); }
        
        _sharedUniformsBuffer = [device newBufferWithLength:sizeof(SharedUniforms)
                                                    options:MTLResourceOptionCPUCacheModeWriteCombined];
        
        _sharedUniformsBuffer.label = @"Shared uniforms";
        
        _vertexBuffer = [self _createVertexBuffer];
        _vertexBuffer.label = @"Vertices";
    }
    return self;
}

- (void)encodeCommandsWithDevice:(id<MTLDevice>)device
                   commandBuffer:(id<MTLCommandBuffer>)commandBuffer
                         surfels:(const std::vector<Surfel>&)surfels
                       icpResult:(ICPResult&)icpResult
                      viewMatrix:(matrix_float4x4)viewMatrix
                projectionMatrix:(matrix_float4x4)projectionMatrix
                     intoTexture:(id<MTLTexture>)texture
                    depthTexture:(id<MTLTexture>)depthTexture
{
    [self _updateSharedUniformsBufferWithViewMatrix:viewMatrix projectionMatrix:projectionMatrix];
    MTLRenderPassDescriptor *passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    passDescriptor.colorAttachments[0].texture = texture;
    passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    passDescriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
    passDescriptor.depthAttachment.texture = depthTexture;
    passDescriptor.depthAttachment.loadAction = MTLLoadActionLoad;
    passDescriptor.depthAttachment.storeAction = MTLStoreActionStore;

    id<MTLRenderCommandEncoder> commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
    commandEncoder.label = @"DrawAxes.commandEncoder";
    
    [commandEncoder setRenderPipelineState:_pipelineState];
    [commandEncoder setViewport:(MTLViewport){ 0, 0, (double)texture.width, (double)texture.height, -1, 1 }];
    [commandEncoder setDepthStencilState:_depthStencilState];
    [commandEncoder setVertexBuffer:_vertexBuffer offset:0 atIndex:0];
    [commandEncoder setVertexBuffer:_sharedUniformsBuffer offset:0 atIndex:1];
    [commandEncoder drawPrimitives:MTLPrimitiveTypeLine vertexStart:0 vertexCount:6];

    [commandEncoder endEncoding];
}

// MARK: - Private

- (id<MTLBuffer>)_createVertexBuffer {
    static const Vertex axisVertices[] = {
        { {0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 } },
        { {1.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 } },
        { {0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 } },
        { {0.0, 1.0, 0.0 }, { 0.0, 1.0, 0.0 } },
        { {0.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0 } },
        { {0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } }
    };
    
    return [_device newBufferWithBytes:axisVertices length:sizeof(axisVertices) options:0];
}

- (void)_updateSharedUniformsBufferWithViewMatrix:(matrix_float4x4)view
                                 projectionMatrix:(matrix_float4x4)projection {
    matrix_float4x4 model = matrix_identity_float4x4;
    SharedUniforms sharedUniforms(projection, view, model);
    memcpy([_sharedUniformsBuffer contents], &sharedUniforms, sizeof(SharedUniforms));
}

@end

NS_ASSUME_NONNULL_END
