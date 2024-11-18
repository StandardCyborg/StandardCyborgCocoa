//
//  RenderPositions.mm
//  StandardCyborgFusion
//
//  Created by Eric on 8/31/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <CoreGraphics/CoreGraphics.h>
#import <GLKit/GLKMatrix4.h>
#import <standard_cyborg/sc3d/Geometry.hpp>
#import <standard_cyborg/math/Vec4.hpp>

#import "RenderPositions.hpp"

namespace math = standard_cyborg::math;

NS_ASSUME_NONNULL_BEGIN

static inline size_t __roundUpToMultiple(size_t value, size_t multiple) {
    return ((value + multiple - 1) / multiple) * multiple;
}

struct Vertex {
    math::Vec4 position;
    math::Vec4 color;
    math::Vec4 texCoord;
    math::Vec4 normal;
    
    Vertex(math::Vec4 position_, math::Vec4 color_, math::Vec4 texCoord_, math::Vec4 normal_) {
        position = position_;
        color = color_;
        texCoord = texCoord_;
        normal = normal_;
    }
};

@implementation RenderPositions {
    id<MTLDevice> _device;
    id<MTLLibrary> _library;
    id<MTLRenderPipelineState> _pipelineState;
}

#pragma mark - MetalVisualization

- (instancetype)initWithDevice:(id<MTLDevice>)device library:(id<MTLLibrary>)library
{
    self = [super init];
    if (self) {
        _device = device;
        
        id<MTLFunction> vertexFunction = [library newFunctionWithName:@"RenderPositionsVertex"];
        id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"RenderPositionsFragment"];
        
        MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
        vertexDescriptor.attributes[0].format = MTLVertexFormatFloat4;
        vertexDescriptor.attributes[0].offset = offsetof(Vertex, position);
        vertexDescriptor.attributes[0].bufferIndex = 0;
        
        vertexDescriptor.attributes[1].format = MTLVertexFormatFloat4;
        vertexDescriptor.attributes[1].offset = offsetof(Vertex, color);
        vertexDescriptor.attributes[1].bufferIndex = 0;
        
        vertexDescriptor.attributes[2].format = MTLVertexFormatFloat4;
        vertexDescriptor.attributes[2].offset = offsetof(Vertex, texCoord);
        vertexDescriptor.attributes[2].bufferIndex = 0;

        vertexDescriptor.attributes[3].format = MTLVertexFormatFloat4;
        vertexDescriptor.attributes[3].offset = offsetof(Vertex, normal);
        vertexDescriptor.attributes[3].bufferIndex = 0;
        
        vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
        vertexDescriptor.layouts[0].stride = sizeof(Vertex);
        
        MTLRenderPipelineDescriptor *pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineDescriptor.vertexFunction = vertexFunction;
        pipelineDescriptor.fragmentFunction = fragmentFunction;
        pipelineDescriptor.vertexDescriptor = vertexDescriptor;
        pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatRGBA32Float;
         
        NSError *error;
        _pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];
        if (_pipelineState == nil) { NSLog(@"Unable to create pipeline state: %@", error); }
        
    }
    return self;
}

- (void)encodeCommandsWithDevice:(id<MTLDevice>)device
                   commandBuffer:(id<MTLCommandBuffer>)commandBuffer
                    triangleMesh:(const sc3d::Geometry&)triangleMesh
                     intoTexture:(id<MTLTexture>)texture
{
    const std::vector<math::Vec3>& positions = triangleMesh.getPositions();
    const std::vector<math::Vec3>& colors = triangleMesh.getColors();
    const std::vector<math::Vec2>& texCoords = triangleMesh.getTexCoords();
    const std::vector<math::Vec3>& normals = triangleMesh.getNormals();
    const std::vector<sc3d::Face3>& faces = triangleMesh.getFaces();
    
    assert(normals.size() > 0);
    
    __block std::shared_ptr<std::vector<Vertex>> vertices(new std::vector<Vertex>());
    __block std::shared_ptr<std::vector<int>> indices(new std::vector<int>());
    
    vertices->reserve(triangleMesh.vertexCount());
    indices->reserve(triangleMesh.faceCount());
    
    for (int iv = 0; iv < triangleMesh.vertexCount(); ++iv) {
        vertices->push_back(Vertex{
            {positions[iv].x, positions[iv].y, positions[iv].z, 0.0f,},
            {colors[iv].x, colors[iv].y, colors[iv].z, 0.0f},
            {texCoords[iv].x, texCoords[iv].y, 0.0, 0.0},
            {normals[iv].x, normals[iv].y, normals[iv].z, 0.0f},
        });
    }
    
    for (const sc3d::Face3& face : faces) {
        indices->push_back(face[0]);
        indices->push_back(face[1]);
        indices->push_back(face[2]);
    }
    
    id<MTLBuffer> _vertexBuffer =
    [_device newBufferWithBytesNoCopy:(void *)vertices->data()
                               length:__roundUpToMultiple(sizeof(Vertex) * vertices->size(), 4096)
                              options:0
                          deallocator:^(void *pointer, NSUInteger length) {
        vertices = nullptr;
    }];
    
    id<MTLBuffer> indexBuffer =
    [_device newBufferWithBytesNoCopy:(void *)indices->data()
                               length:__roundUpToMultiple(sizeof(int) * indices->size(), 4096)
                              options:0
                          deallocator:^(void *pointer, NSUInteger length) {
        indices = nullptr;
    }];
    
    MTLRenderPassDescriptor *passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    passDescriptor.colorAttachments[0].texture = texture;
    passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(std::nan("1"), std::nan("1"), std::nan("1"), std::nan("1"));
    
    id<MTLRenderCommandEncoder> commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
    commandEncoder.label = @"RenderPositions.commandEncoder";
    
    [commandEncoder setRenderPipelineState:_pipelineState];
    [commandEncoder setViewport:(MTLViewport){ 0, 0, (double)texture.width, (double)texture.height, -1, 1 }];
    [commandEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
    [commandEncoder setCullMode:MTLCullModeNone];
    [commandEncoder setVertexBuffer:_vertexBuffer offset:0 atIndex:0];

    [commandEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                               indexCount:faces.size() * 3
                                indexType:MTLIndexTypeUInt32
                              indexBuffer:indexBuffer
                        indexBufferOffset:0];
    
    [commandEncoder endEncoding];
}

@end

NS_ASSUME_NONNULL_END
