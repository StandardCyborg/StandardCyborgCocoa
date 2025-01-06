//
//  DepthProcessor.hpp
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 8/14/18.
//

#pragma once

#import "ProcessedFrame.hpp"
#import "RawFrame.hpp"

class DepthProcessor {
public:
    virtual void computeFrameValues(ProcessedFrame &frameOut,
                                    const RawFrame &rawFrame,
                                    bool smoothPoints = false) = 0;
};
