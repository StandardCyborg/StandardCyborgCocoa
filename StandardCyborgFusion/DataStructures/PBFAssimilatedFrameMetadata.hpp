//
//  PBFAssimilatedFrameMetadata.hpp
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 9/28/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#pragma once

#include <standard_cyborg/util/IncludeEigen.hpp>

struct PBFAssimilatedFrameMetadata {
    /**
     @brief View matrix for the assimilated frame
     @discussion View matrix here is equivalent to the extrinsic matrix but named as such since after applying near and far clipping planes to the projection (perspective) matrix, the combination corresponds to normal computer graphics notation.
     */
    Eigen::Matrix4f viewMatrix;
    
    /**
     @brief Projection matrix for the assimilated frame
     */
    Eigen::Matrix4f projectionMatrix;
    
    double timestamp;
    
    /**
     @brief True if this frame was merged, false if it was rejected
     */
    bool isMerged = false;
    
    /**
     @brief Total surfel count after this frame was merged
     */
    size_t surfelCount = 0;
    
    float correspondenceError = 0.0f;
    
    float icpUnusedIterationFraction = 0.0;
    int icpIterationCount = 0;
};
