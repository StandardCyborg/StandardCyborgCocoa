//
//  MetalDepthProcessor.mm
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 8/1/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <vector>
#import <StandardCyborgFusion/ProcessedFrame.hpp>
#import <StandardCyborgFusion/RawFrame.hpp>
#import <StandardCyborgFusion/MathHelpers.h>
#import <StandardCyborgFusion/EigenHelpers.hpp>
#import <Metal/Metal.h>

#import "MetalComputeEngine.h"
#import "MetalDepthProcessor.hpp"
#import "MetalDepthProcessorData.hpp"
#import "ComputePointsKernel.h"
#import "InitializeDepthConfidenceKernel.h"
#import "ComputeNormalsKernel.h"
#import "ComputeWeightsKernel.h"
#import "SmoothDepthKernel.h"
#import "MetalDepthProcessorData.hpp"


MetalDepthProcessor::MetalDepthProcessor(id<MTLDevice> device, id<MTLCommandQueue> commandQueue) {
    assert(device != nil);
    id<MTLLibrary> library = [device newDefaultLibraryWithBundle:[NSBundle bundleWithIdentifier:@"com.standardcyborg.StandardCyborgFusion"] error:NULL];

    NSArray *kernels = @[
                         [[InitializeDepthConfidenceKernel alloc] initWithDevice:device library:library],
                         [[SmoothDepthKernel alloc] initWithDevice:device library:library],
                         [[ComputePointsKernel alloc] initWithDevice:device library:library],
                         [[ComputeNormalsKernel alloc] initWithDevice:device library:library],
                         [[ComputeWeightsKernel alloc] initWithDevice:device library:library],
                         ];
    
    _computeEngine = [[MetalComputeEngine alloc] initWithDevice:device commandQueue:commandQueue library:library computeKernels:kernels];
}


void MetalDepthProcessor::computeFrameValues(ProcessedFrame &frame,
                                             const RawFrame &rawFrame,
                                             bool smoothPoints)
{
    const sc3d::PerspectiveCamera& camera = rawFrame.camera;
    size_t width = rawFrame.width;
    size_t height = rawFrame.height;
    
    size_t pointCount = width * height;
    assert(pointCount == rawFrame.depths.size());
    assert(pointCount == rawFrame.colors.size());
    
    
    InitializeDepthConfidenceKernel *initializeDepthConfidenceKernel = (InitializeDepthConfidenceKernel *)[[_computeEngine kernels] objectAtIndex:0];
    SmoothDepthKernel *smoothDepthKernel = (SmoothDepthKernel *)[[_computeEngine kernels] objectAtIndex:1];
    ComputePointsKernel *pointsKernel = (ComputePointsKernel *)[[_computeEngine kernels] objectAtIndex:2];
    ComputeNormalsKernel *normalsKernel = (ComputeNormalsKernel *)[[_computeEngine kernels] objectAtIndex:3];
    ComputeWeightsKernel *weightsKernel = (ComputeWeightsKernel *)[[_computeEngine kernels] objectAtIndex:4];
    
    [pointsKernel setPerspectiveCamera:camera
                           frameWidth:width
                          frameHeight:height];
    
    [pointsKernel setUseSmoothedDepth:smoothPoints];
    
    [normalsKernel setPerspectiveCamera:camera
                            frameWidth:width
                           frameHeight:height];
    
    [weightsKernel setPerspectiveCamera:camera
                            frameWidth:width
                           frameHeight:height];
     
    [smoothDepthKernel setPerspectiveCamera:camera
                                frameWidth:width
                               frameHeight:height];
    
    [initializeDepthConfidenceKernel setPerspectiveCamera:camera
                                              frameWidth:width
                                             frameHeight:height];
    
    _depthProcessorData.fill([_computeEngine device],
                             width, height,
                             rawFrame.depths,
                             frame.positions,                
                             frame.normals,
                             frame.surfelSizes,
                             frame.weights,
                             frame.inputConfidences);
    
    [_computeEngine runWithDepthProcessorData:_depthProcessorData];
}
