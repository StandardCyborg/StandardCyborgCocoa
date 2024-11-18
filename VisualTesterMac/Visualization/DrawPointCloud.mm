//
//  DrawPointCloud.mm
//  VisualTesterMac
//
//  Created by Ricky Reusser on 8/31/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <CoreGraphics/CoreGraphics.h>
#import <GLKit/GLKMatrix4.h>
#import "DrawPointCloud.hpp"

NS_ASSUME_NONNULL_BEGIN

static inline size_t __roundUpToMultiple(size_t value, size_t multiple) {
    return ((value + multiple - 1) / multiple) * multiple;
}

@implementation DrawPointCloud {
    id<MTLDevice> _device;
    id<MTLLibrary> _library;
    id<MTLRenderPipelineState> _pipelineState;
    id<MTLBuffer> _vertexBuffer;
    id<MTLBuffer> _sharedUniformsBuffer;
    id<MTLDepthStencilState> _depthStencilState;
}

struct Vertex {
    simd_float2 vertices;
    
    Vertex(float x, float y) {
        vertices = (simd_float2){ x, y };
    }
};

struct SharedUniforms {
    matrix_float4x4 projection;
    matrix_float4x4 view;
    matrix_float4x4 model;
    bool colorByNormals;
    
    SharedUniforms(matrix_float4x4 projectionIn, matrix_float4x4 viewIn, matrix_float4x4 modelIn, bool colorByNormalsIn) :
    projection(projectionIn),
    view(viewIn),
    model(modelIn),
    colorByNormals(colorByNormalsIn)
    { }
};

// MARK: - MetalVisualization

- (instancetype)initWithDevice:(id<MTLDevice>)device library:(id<MTLLibrary>)library
{
    self = [super init];
    if (self) {
        _device = device;
        
        id<MTLFunction> vertexFunction = [library newFunctionWithName:@"DrawPointCloudVertex"];
        id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"DrawPointCloudFragment"];
        
        MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
        vertexDescriptor.attributes[0].format = MTLVertexFormatFloat2;
        vertexDescriptor.attributes[0].offset = 0;
        vertexDescriptor.attributes[0].bufferIndex = 0;
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
        
        _sharedUniformsBuffer = [device newBufferWithLength:sizeof(SharedUniforms) options:MTLResourceOptionCPUCacheModeWriteCombined];
        _sharedUniformsBuffer.label = @"DrawPointCloud.sharedUniforms";
        _vertexBuffer = [self _createVertexBuffer];
        _vertexBuffer.label = @"DrawPointCloud._vertexBuffer";
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
    size_t surfelCount = surfels.size();
    if (surfelCount == 0) { return; }
    
    [self _updateSharedUniformsBufferWithViewMatrix:viewMatrix
                                   projectionMatrix:projectionMatrix];
    id<MTLBuffer> surfelsBuffer = [self _createSurfelsBufferFromSurfels:surfels];
    
    MTLRenderPassDescriptor *passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    passDescriptor.colorAttachments[0].texture = texture;
    passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    passDescriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
    passDescriptor.depthAttachment.texture = depthTexture;
    passDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
    passDescriptor.depthAttachment.loadAction = MTLLoadActionLoad;

    id<MTLRenderCommandEncoder> commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
    commandEncoder.label = @"DrawPointCloud.commandEncoder";
    
    if (surfelCount > 0) {
        [commandEncoder setRenderPipelineState:_pipelineState];
        [commandEncoder setViewport:(MTLViewport){ 0, 0, (double)texture.width, (double)texture.height, -1, 1 }];
        [commandEncoder setDepthStencilState:_depthStencilState];
        [commandEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
        [commandEncoder setCullMode:MTLCullModeNone];
        [commandEncoder setVertexBuffer:_vertexBuffer offset:0 atIndex:0];
        [commandEncoder setVertexBuffer:_sharedUniformsBuffer offset:0 atIndex:1];
        [commandEncoder setVertexBuffer:surfelsBuffer offset:0 atIndex:2];
        [commandEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:6 instanceCount:surfelCount];
    }
    
    [commandEncoder endEncoding];
}

// MARK: - Private

- (id<MTLBuffer>)_createVertexBuffer {
    static const float INV_RT3 = 1.0f / sqrtf(3.0f);
    
    static const Vertex kHexVertices[] = {
        Vertex(-2.0f * INV_RT3,  0.0),
        Vertex(       -INV_RT3, -1.0),
        Vertex(       -INV_RT3,  1.0),
        Vertex(        INV_RT3, -1.0),
        Vertex(        INV_RT3,  1.0),
        Vertex( 2.0f * INV_RT3,  0.0),
    };
    
    return [_device newBufferWithBytes:kHexVertices length:sizeof(kHexVertices) options:0];
}

- (id<MTLBuffer>)_createSurfelsBufferFromSurfels:(const std::vector<Surfel>&)surfels {
    return [_device newBufferWithBytesNoCopy:(void *)&surfels[0]
                                      length:__roundUpToMultiple(sizeof(Surfel) * surfels.size(), 4096)
                                     options:0
                                 deallocator:NULL];
}

- (void)_updateSharedUniformsBufferWithViewMatrix:(matrix_float4x4)view
                                 projectionMatrix:(matrix_float4x4)projection
{
    matrix_float4x4 model = matrix_identity_float4x4;
    
    SharedUniforms sharedUniforms(projection, view, model, _colorByNormals);
    memcpy([_sharedUniformsBuffer contents], &sharedUniforms, sizeof(SharedUniforms));
}

@end

NS_ASSUME_NONNULL_END
