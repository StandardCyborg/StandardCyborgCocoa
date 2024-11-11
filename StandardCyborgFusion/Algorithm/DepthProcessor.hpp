//
//  DepthProcessor.hpp
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 8/14/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#pragma once

#include "ProcessedFrame.hpp"
#include <StandardCyborgFusion/RawFrame.hpp>

class DepthProcessor {
public:
    virtual void computeFrameValues(ProcessedFrame &frameOut,
                                    const RawFrame &rawFrame,
                                    bool smoothPoints = false) = 0;
};
