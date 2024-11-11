//
//  MetalDepthProcessorData.mm
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 9/24/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import "MetalDepthProcessorData.hpp"
#import <Foundation/Foundation.h>
#include <iostream>
#import <StandardCyborgFusion/MathHelpers.h>

#include <vector>

using namespace Eigen;

using namespace standard_cyborg;

MetalDepthProcessorData::MetalDepthProcessorData() {}

MetalDepthProcessorData::~MetalDepthProcessorData() {}

void MetalDepthProcessorData::fill(
    id<MTLDevice> device,
    size_t widthIn,
    size_t heightIn,
    
    const std::vector<float> &depths,
    std::vector<math::Vec3> &points,
    std::vector<math::Vec3> &normals,
    std::vector<float> &surfelSizes,
    std::vector<float> &weights,
    std::vector<float> &inputConfidences)
{
    width = widthIn;
    height = heightIn;
    
    MTLRegion region = MTLRegionMake2D(0, 0, width, height);
    NSUInteger bytesPerRow = width * sizeof(float);

    // Cache the depth texture object just for efficiency, reusing the buffer but filling it anew each time
    if (depthTexture == nil || depthTexture.width != width || depthTexture.height != height) {
        MTLTextureDescriptor *depthTextureDescriptor = [MTLTextureDescriptor
                                                        texture2DDescriptorWithPixelFormat:MTLPixelFormatR32Float
                                                        width:width
                                                        height:height
                                                        mipmapped:NO];
        depthTextureDescriptor.usage = MTLTextureUsageShaderRead;
        depthTexture = [device newTextureWithDescriptor:depthTextureDescriptor];
        depthTexture.label = @"depthTexture";
    }
    
    [depthTexture replaceRegion:region
                    mipmapLevel:0
                      withBytes:depths.data()
                    bytesPerRow:bytesPerRow];
    
    if (workTexture == NULL || workTexture.width != width || workTexture.height != height) {
        MTLTextureDescriptor *workTextureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatR32Float width:width height:height mipmapped:NO];
        workTextureDescriptor.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
        workTexture = [device newTextureWithDescriptor:workTextureDescriptor];
    }
    
    if (smoothedDepthTexture == NULL || smoothedDepthTexture.width != width || smoothedDepthTexture.height != height) {
        MTLTextureDescriptor *smoothedDepthTextureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatR32Float width:width height:height mipmapped:NO];
        smoothedDepthTextureDescriptor.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
        smoothedDepthTexture = [device newTextureWithDescriptor:smoothedDepthTextureDescriptor];
    }

    size_t pointsBufferSize = roundUpToMultiple(points.size() * sizeof(math::Vec3), 4096);
    pointsBuffer = [device newBufferWithBytesNoCopy:points.data() length:pointsBufferSize options:0 deallocator:NULL];
    pointsBuffer.label = @"pointsBuffer";

    size_t normalsBufferSize = roundUpToMultiple(normals.size() * sizeof(math::Vec3), 4096);
    normalsBuffer = [device newBufferWithBytesNoCopy:normals.data() length:normalsBufferSize options:0 deallocator:NULL];
    normalsBuffer.label = @"normalsBuffer";
     
    size_t weightsBufferSize = roundUpToMultiple(weights.size() * sizeof(float), 4096);
    weightsBuffer = [device newBufferWithBytesNoCopy:weights.data() length:weightsBufferSize options:0 deallocator:NULL];
    weightsBuffer.label = @"weightsBuffer";

    size_t inputConfidencesSize = roundUpToMultiple(inputConfidences.size() * sizeof(float), 4096);
    inputConfidencesBuffer = [device newBufferWithBytesNoCopy:inputConfidences.data() length:inputConfidencesSize options:0 deallocator:NULL];
    inputConfidencesBuffer.label = @"inputConfidencesBuffer";
    
    size_t surfelSizesSize = roundUpToMultiple(surfelSizes.size() * sizeof(float), 4096);
    surfelSizesBuffer = [device newBufferWithBytesNoCopy:surfelSizes.data() length:surfelSizesSize options:0 deallocator:NULL];
    surfelSizesBuffer.label = @"surfelSizesBuffer";
    
    if (workBuffer == NULL || workBuffer.length != inputConfidencesSize) {
        workBuffer = [device newBufferWithLength:inputConfidencesSize options:0];
        workBuffer.label = @"workBuffer";
    }
}
