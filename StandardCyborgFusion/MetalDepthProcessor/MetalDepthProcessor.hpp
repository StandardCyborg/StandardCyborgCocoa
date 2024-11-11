//
//  MetalDepthProcessor.hpp
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 8/1/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#pragma once

#import "DepthProcessor.hpp"
#import "MetalDepthProcessorData.hpp"
#import "ProcessedFrame.hpp"
#import <StandardCyborgFusion/RawFrame.hpp>

@class MetalComputeEngine;
@protocol MTLCommandQueue;
@protocol MTLDevice;
@protocol MTLTexture;

class MetalDepthProcessor: public DepthProcessor {
public:
    MetalDepthProcessor(id<MTLDevice> device, id<MTLCommandQueue> commandQueue);

    virtual void computeFrameValues(ProcessedFrame &frameOut, const RawFrame &rawFrame, bool smoothPoints = false);
    
private:
    MetalComputeEngine *_computeEngine;
    MetalDepthProcessorData _depthProcessorData;
};
