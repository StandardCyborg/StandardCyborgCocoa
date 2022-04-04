//
//  SurfelFusion.hpp
//  StandardCyborgFusion
//
//  Created by eric on 2020-03-12.
//  Copyright © 2020 Standard Cyborg. All rights reserved.
//

#ifndef SurfelFusion_hpp
#define SurfelFusion_hpp

#include "SurfelIndexMap.hpp"

#include <StandardCyborgFusion/ProcessedFrame.hpp>
#include <StandardCyborgFusion/ScreenSpaceLandmark.hpp>
#include <StandardCyborgFusion/SparseSurfelLandmarksIndex.hpp>

struct SurfelFusionConfiguration {
    float maxSurfelIncidenceThreshold = (70 * M_PI / 180);
    float surfelMergeRadiusScaleFactor = 0.1;
    float inputConfidenceThreshold = 0.9;
    float minDepth = 0;
    float maxDepth = 0.5;
    bool cullLowConfidence = true;
    int minCount = 6;
    int surfelLifetime = 20;
    bool ignoreLifetime = false;
};

class SurfelFusion {
public:
    SurfelFusion(std::shared_ptr<SurfelIndexMap> surfelIndexMap);

    bool doFusion(SurfelFusionConfiguration surfelFusionConfiguration, ProcessedFrame& frame, Surfels& surfels, math::Mat4x4 extrinsicMatrix, const std::vector<ScreenSpaceLandmark>* screenSpaceLandmarks, SparseSurfelLandmarksIndex& _surfelLandmarksIndex, std::vector<int>& deletedSurfelIndicesList);
    
    void finish(SurfelFusionConfiguration surfelFusionConfiguration, Surfels& surfels, SparseSurfelLandmarksIndex& surfelLandmarksIndex, std::vector<int>& deletedSurfelIndicesList);

    const std::vector<uint32_t>& getSurfelIndexLookups()const;
    
private:
    void cullLowConfidence(bool ignoreLifetime, int minWeight, Surfels& surfels, std::vector<int>* deletedSurfelList =NULL  );
    
    std::shared_ptr<SurfelIndexMap> _surfelIndexMap;
    std::vector<uint32_t> _surfelIndexLookups;
};
#endif /* SurfelFusion_hpp */
