//
//  OutlierDetector.cpp
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 8/13/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#include "OutlierDetector.hpp"
#include <cmath>

using namespace StandardCyborg;

OutlierDetector::OutlierDetector (float _meanDecayTime, float _varianceDecayTime, float _outlierCutoff, bool _filterOutliers)
    : meanDecayTime(_meanDecayTime),
      varianceDecayTime(_varianceDecayTime),
      outlierCutoff(_outlierCutoff),
      filterOutliers(_filterOutliers)
{
    mean = 0.0;
    tPrev = 0.0;
    count = 0;
    M2n = 1.0;
}

OutlierDetectorResult OutlierDetector::assimilateSample (float t, float y) {
    OutlierDetectorResult result;
    result.t = t;
    
    float dt = t - tPrev;
    float varianceDecayFactor = count < 2.0 ? 1.0 : std::exp(-dt / varianceDecayTime * std::log(2.0));

    count++;
    n = n * varianceDecayFactor + 1;
    
    switch(count) {
      case 1:
        mean = y;
        M2n = 1.0;

        result.mean = mean;
        result.deviationsFromMean = 0.0;
        result.isOutlier = false;

        break;

      case 2:
        mean = (mean + y) * 0.5;
        M2n = 1.0;

        result.mean = mean;
        result.deviationsFromMean = 0.0;
        result.isOutlier = false;
        break;

      default:
        float meanDecayFactor = std::exp(-dt / meanDecayTime * std::log(2.0));
        float deviationsFromMean = std::abs(y - mean) / std::sqrt(M2n / n);

        if (!filterOutliers || (M2n == 0.0 || deviationsFromMean <= outlierCutoff)) {
          float prevMean = mean;
          mean = mean * meanDecayFactor + y * (1.0 - meanDecayFactor);
          M2n = M2n * varianceDecayFactor + (y - prevMean) * (y - mean);
        }

        result.mean = mean;
        result.isOutlier = true;

    }
    
    result.y = y;
    result.variance = M2n / n;
    result.deviationsFromMean = std::abs(result.y - result.mean) / std::sqrt(result.variance);
    result.outlierScore = result.deviationsFromMean / outlierCutoff;
    result.isOutlier = result.outlierScore > 1.0;
    
    tPrev = t;
    
    return result;
}
