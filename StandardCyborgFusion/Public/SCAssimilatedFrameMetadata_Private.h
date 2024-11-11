//
//  SCAssimilatedFrameMetadata_Private.h
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 12/20/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <StandardCyborgFusion/PBFAssimilatedFrameMetadata.hpp>
#import <StandardCyborgFusion/SCAssimilatedFrameMetadata.h>
#import <StandardCyborgFusion/EigenHelpers.hpp>

static SCAssimilatedFrameMetadata
SCAssimilatedFrameMetadataFromPBFAssimilatedFrameMetadata(PBFAssimilatedFrameMetadata pbfMetadata,
                                                          NSInteger consecutiveFailedFrameCount)
{
    static const float kPoorTrackingQualityThreshold = 0.1;
    static const NSInteger kMaxConsecutiveFailedFrameCount = 8;
    
    SCAssimilatedFrameMetadata metadata;
    metadata.viewMatrix = toSimdFloat4x4(pbfMetadata.viewMatrix);

    metadata.projectionMatrix = toSimdFloat4x4(pbfMetadata.projectionMatrix);
    metadata.colorBuffer = NULL;
    metadata.depthBuffer = NULL;
    
    if (pbfMetadata.isMerged == false && consecutiveFailedFrameCount + 1 >= kMaxConsecutiveFailedFrameCount) {
        metadata.result = SCAssimilatedFrameResultFailed;
    } else if (pbfMetadata.isMerged == false) {
        metadata.result = SCAssimilatedFrameResultLostTracking;
    } else if (pbfMetadata.icpUnusedIterationFraction < kPoorTrackingQualityThreshold) {
        metadata.result = SCAssimilatedFrameResultPoorTracking;
    } else {
        metadata.result = SCAssimilatedFrameResultSucceeded;
    }
    
    return metadata;
}
