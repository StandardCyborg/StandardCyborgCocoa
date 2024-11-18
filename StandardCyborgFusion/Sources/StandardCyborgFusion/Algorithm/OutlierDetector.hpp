//
//  OutlierDetector.hpp
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 8/13/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

namespace StandardCyborg {

struct OutlierDetectorResult {
    float t;
    float y;
    float mean;
    float variance;
    float deviationsFromMean;
    float outlierScore;
    bool isOutlier;
};

class OutlierDetector {
    float mean;
    float tPrev;
    float n;
    int count;
    float M2n;

    float meanDecayTime, varianceDecayTime, outlierCutoff;
    bool filterOutliers;
    
public:
    OutlierDetector(float meanDecayTime, float varianceDecayTime, float outlierCutoff, bool filterOutliers);
    
    OutlierDetectorResult assimilateSample (float t, float y);
};

} // namespace StandardCyborg
