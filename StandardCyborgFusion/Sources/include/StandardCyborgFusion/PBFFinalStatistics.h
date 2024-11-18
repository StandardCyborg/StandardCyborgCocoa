//
//  PBFFinalStatistics.h
//  StandardCyborgSDK
//
//  Created by eric on 2019-10-24.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

typedef struct {
    int mergedFrameCount;
    double framerate;
    double averageICPIterations;
    int failedFrameCount;
    float averageCorrespondenceError;
} PBFFinalStatistics;
