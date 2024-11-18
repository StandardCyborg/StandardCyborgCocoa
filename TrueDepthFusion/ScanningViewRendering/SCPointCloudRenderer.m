//
//  SCPointCloudRenderer.m
//  TrueDepthFusion
//
//  Created by Aaron Thompson on 9/23/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <AVFoundation/AVFoundation.h>
#import <MetalKit/MetalKit.h>
#import <StandardCyborgFusion/StandardCyborgFusion.h>

#import "SCPointCloudRenderer.h"

@implementation SCPointCloudRenderer {
    id<MTLDevice> _device;
    id<MTLLibrary> _library;
    id<MTLRenderPipelineState> _pipelineState;
    id<MTLDepthStencilState> _depthStencilState;
    id<MTLBuffer> _sharedUniformsBuffer;
    id<MTLTexture> _depthTexture;
    id<MTLTexture> _matcapTexture;
    MTKTextureLoader *_textureLoader;
}

typedef struct {
    simd_float3x3 viewNormalMatrix;
    simd_float4x4 viewMatrix;
    simd_float4x4 viewProjectionMatrix;
    float pointSize;
} SharedUniforms;

// MARK: - MetalVisualization

+ (MTLVertexDescriptor *)pointCloudVertexDescriptor
{
    MTLVertexDescriptor *descriptor = [MTLVertexDescriptor vertexDescriptor];
    
    descriptor.layouts[0].stride = [SCPointCloud pointStride];
    descriptor.attributes[0].format = MTLVertexFormatFloat3;
    descriptor.attributes[0].offset = [SCPointCloud positionOffset];
    descriptor.attributes[1].format = MTLVertexFormatFloat3;
    descriptor.attributes[1].offset = [SCPointCloud normalOffset];
    descriptor.attributes[2].format = MTLVertexFormatFloat3;
    descriptor.attributes[2].offset = [SCPointCloud colorOffset];
    descriptor.attributes[3].format = MTLVertexFormatFloat;
    descriptor.attributes[3].offset = [SCPointCloud weightOffset];
    
    return descriptor;
}

- (instancetype)initWithDevice:(id<MTLDevice>)device library:(id<MTLLibrary>)library
{
    self = [super init];
    if (self) {
        _device = device;
        
        _textureLoader = [[MTKTextureLoader alloc] initWithDevice:device];
        _matcapTexture = [_textureLoader newTextureWithName:@"matcap" scaleFactor:1.0 bundle:nil options:@{MTKTextureLoaderOptionSRGB: @NO} error:NULL];
        
        id<MTLFunction> vertexFunction = [library newFunctionWithName:@"RenderSCPointCloudVertex"];
        id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"RenderSCPointCloudFragment"];
        
        MTLRenderPipelineDescriptor *pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineDescriptor.vertexFunction = vertexFunction;
        pipelineDescriptor.fragmentFunction = fragmentFunction;
        pipelineDescriptor.vertexDescriptor = [[self class] pointCloudVertexDescriptor];
        pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
        pipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
        pipelineDescriptor.label = @"SCPointCloudRenderer._pipelineState";
        
        MTLDepthStencilDescriptor *depthStencilDescriptor = [[MTLDepthStencilDescriptor alloc] init];
        depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionLess;
        depthStencilDescriptor.depthWriteEnabled = YES;
        depthStencilDescriptor.label = @"SCPointCloudRenderer._depthStencilState";
        _depthStencilState = [_device newDepthStencilStateWithDescriptor:depthStencilDescriptor];
        
        NSError *error;
        _pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];
        if (_pipelineState == nil) { NSLog(@"Unable to create pipeline state: %@", error); }
        
        _sharedUniformsBuffer = [device newBufferWithLength:sizeof(SharedUniforms)
                                                    options:MTLResourceOptionCPUCacheModeWriteCombined];
        _sharedUniformsBuffer.label = @"SCPointCloudRenderer._sharedUniformsBuffer";
    }
    return self;
}

- (void)encodeCommandsOntoBuffer:(id<MTLCommandBuffer>)commandBuffer
                      pointCloud:(SCPointCloud *)pointCloud
      depthCameraCalibrationData:(AVCameraCalibrationData *)depthCameraCalibrationData
                      viewMatrix:(matrix_float4x4)viewMatrix
                   outputTexture:(id<MTLTexture>)outputTexture
                  depthFrameSize:(CGSize)depthFrameSize
          flipsInputHorizontally:(BOOL)flipsInputHorizontally
{
    if ([pointCloud pointCount] == 0) { return; }
    
    if (_depthTexture == nil) {
        MTLTextureDescriptor *descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                                                              width:outputTexture.width
                                                                                             height:outputTexture.height
                                                                                          mipmapped:NO];
        descriptor.usage = MTLTextureUsageRenderTarget;
        descriptor.storageMode = MTLStorageModePrivate;
        _depthTexture = [_device newTextureWithDescriptor:descriptor];
        _depthTexture.label = @"SCPointCloudRenderer._depthTexture";
    }
    
    id<MTLBuffer> pointsBuffer = [pointCloud buildPointsMTLBufferWithDevice:_device];
    pointsBuffer.label = @"SCPointCloudRenderer.pointsBuffer";
    
    // BEWARE! Middle return
    if (pointsBuffer == nil) { return; }
    
    [self _updateSharedUniformsBufferWithIntrinsicMatrix:depthCameraCalibrationData.intrinsicMatrix
                                     referenceDimensions:depthCameraCalibrationData.intrinsicMatrixReferenceDimensions
                                              viewMatrix:viewMatrix
                                             resultWidth:outputTexture.width
                                            resultHeight:outputTexture.height
                                          depthFrameSize:depthFrameSize
                                  flipsInputHorizontally:flipsInputHorizontally];
    
    MTLRenderPassDescriptor *passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    passDescriptor.colorAttachments[0].texture = outputTexture;
    passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    passDescriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
    passDescriptor.depthAttachment.texture = _depthTexture;
    passDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
    passDescriptor.depthAttachment.storeAction = MTLStoreActionDontCare;
    passDescriptor.depthAttachment.clearDepth = 1.0;
    
    id<MTLRenderCommandEncoder> commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
    commandEncoder.label = @"SCPointCloudRenderer.commandEncoder";
    
    [commandEncoder setRenderPipelineState:_pipelineState];
    [commandEncoder setViewport:(MTLViewport){ 0, 0, (double)outputTexture.width, (double)outputTexture.height, -1, 1 }];
    [commandEncoder setDepthStencilState:_depthStencilState];
    [commandEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
    [commandEncoder setCullMode:MTLCullModeBack];
    [commandEncoder setVertexBuffer:pointsBuffer offset:0 atIndex:0];
    [commandEncoder setVertexBuffer:_sharedUniformsBuffer offset:0 atIndex:1];
    [commandEncoder setFragmentTexture:_matcapTexture atIndex:0];
    [commandEncoder drawPrimitives:MTLPrimitiveTypePoint vertexStart:0 vertexCount:[pointCloud pointCount]];
    
    [commandEncoder endEncoding];
}

// MARK: - Private

- (void)_updateSharedUniformsBufferWithIntrinsicMatrix:(matrix_float3x3)intrinsicMatrix
                                   referenceDimensions:(CGSize)intrinsicMatrixReferenceDimensions
                                            viewMatrix:(matrix_float4x4)viewMatrix
                                           resultWidth:(NSUInteger)resultWidth
                                          resultHeight:(NSUInteger)resultHeight
                                        depthFrameSize:(CGSize)depthFrameSize
                                flipsInputHorizontally:(bool)flipsInputHorizontally
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
    float fx = intrinsicMatrix.columns[0][0];
    float fy = intrinsicMatrix.columns[1][1];
    float sourceWidth = intrinsicMatrixReferenceDimensions.width;
    float sourceHeight = intrinsicMatrixReferenceDimensions.height;
    
    // These don't have to be perfect. We don't expect anything closer than
    // a millimeter or farther than ten meters given conceivable technological
    // limitations.
    float near = 0.001; // meters
    float far = 10.0; // meters
    
    // NB: the source aspect ratio is inverted because the incoming frame is sideways
    // relative to what we display. It's much, much, much easier to reason about if
    // we just flip it.
    float resultAspectRatio = (float)resultWidth / (float)resultHeight;
    float sourceAspectRatio = (float)sourceHeight / (float)sourceWidth;
    
    // These magic numbers are chosen to anchor a nominal point size with the initial setup,
    // then to scale it correctly with the shape of the input and output.
    float nominalPointSize = 8.0;
    float nominalFrameWidth = 640.0;
    float nominalResultHeight = 1556.0;
    float pointSize = nominalPointSize * (nominalFrameWidth / depthFrameSize.width) * ((float)resultHeight / nominalResultHeight);
    
    simd_float2 imageScale;
    float referenceSize;
    if (sourceAspectRatio > resultAspectRatio) {
        // The source data is wider than the result display
        imageScale[0] = 1.0 / resultAspectRatio;
        imageScale[1] = 1.0;
        referenceSize = sourceWidth;
    } else {
        imageScale[0] = 1.0;
        imageScale[1] = resultAspectRatio;
        referenceSize = sourceHeight;
    }

    simd_float4x4 projection = {
        .columns[0] = {2.0f * fx / referenceSize * imageScale[0], 0.0f, 0.0f, 0.0f},
        .columns[1] = {0.0f, 2.0f * fy / referenceSize * imageScale[1], 0.0f, 0.0f},
        .columns[2] = {0.0f, 0.0f, (far + near) / (near - far), -1.0f},
        .columns[3] = {0.0f, 0.0f, 2.0f * far * near / (near - far), 0.0f}
    };
    
    simd_float4x4 viewInverse = matrix_invert(viewMatrix);
    
    // According to Aaron, the TrueDepth camera is rotated 90degrees inside the phone,
    // so we need to swap the x and y axes to counteract this.
    simd_float4x4 orientationTransform;
    if (flipsInputHorizontally) {
        // if we flip input horizontally, then the rendered point cloud will also be
        // flipped horizontally, and not match the selfie image shown in the background,
        // which looks weird. So, we flip horizontally, before applying the projection matrix.
        orientationTransform = (simd_float4x4){
            .columns[0] = { -1.0, 0.0,  0.0, 0.0 },
            .columns[1] = {  0.0, 1.0,  0.0, 0.0 },
            .columns[2] = {  0.0, 0.0, -1.0, 0.0 },
            .columns[3] = {  0.0, 0.0,  0.0, 1.0 },
        };
    } else {
        orientationTransform = (simd_float4x4){
            .columns[0] = { 1.0, 0.0,  0.0, 0.0 },
            .columns[1] = { 0.0, 1.0,  0.0, 0.0 },
            .columns[2] = { 0.0, 0.0, -1.0, 0.0 },
            .columns[3] = { 0.0, 0.0,  0.0, 1.0 },
        };
    }
    
    
    simd_float4x4 view = matrix_multiply(orientationTransform, viewInverse);
    
    simd_float3x3 truncatedView = {
        .columns[0] = { view.columns[0][0], view.columns[0][1], view.columns[0][2] },
        .columns[1] = { view.columns[1][0], view.columns[1][1], view.columns[1][2] },
        .columns[2] = { view.columns[2][0], view.columns[2][1], view.columns[2][2] }
    };
    simd_float3x3 viewNormalMatrix = matrix_transpose(matrix_invert(truncatedView));
    
    simd_float4x4 viewProjectionMatrix = matrix_multiply(projection, matrix_multiply(orientationTransform, viewInverse));
    
    SharedUniforms sharedUniforms;
    sharedUniforms.viewMatrix = view;
    sharedUniforms.viewNormalMatrix = viewNormalMatrix;
    sharedUniforms.viewProjectionMatrix = viewProjectionMatrix;
    sharedUniforms.pointSize = pointSize;
    
    memcpy([_sharedUniformsBuffer contents], &sharedUniforms, sizeof(SharedUniforms));
}

@end
