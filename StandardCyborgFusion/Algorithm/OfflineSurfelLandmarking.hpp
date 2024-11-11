//
//  OfflineSurfelLandmarking.hpp
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 5/28/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <standard_cyborg/util/IncludeEigen.hpp>
#include <StandardCyborgFusion/Surfel.hpp>
#include <StandardCyborgFusion/ScreenSpaceLandmark.hpp>
#include <StandardCyborgFusion/SparseSurfelLandmarksIndex.hpp>
#include <StandardCyborgFusion/SurfelIndexMap.hpp>
#include <StandardCyborgFusion/ScreenSpaceLandmark.hpp>
#include <unordered_map>
#include <vector>

class OfflineSurfelLandmarking {
    
public:
    OfflineSurfelLandmarking(std::shared_ptr<SurfelIndexMap> surfelIndexMap);
    
    ~OfflineSurfelLandmarking();
    
    void placeLandmarksOnSurfels(const Surfel* surfels,
                                 size_t surfelCount,
                                 size_t frameWidth,
                                 size_t frameHeight,
                                 Eigen::Matrix4f viewProjectionMatrix,
                                 const std::vector<ScreenSpaceLandmark>& screenSpaceLandmarks);
    
    std::unordered_map<int, Eigen::Vector3f> computeLandmarks(const Surfel* surfels);
    
    void reset();
    
    const std::vector<uint32_t>& getSurfelIndexLookups(); // Exposed for debugging
    
private:
    std::shared_ptr<SurfelIndexMap> _surfelIndexMap;
    std::vector<uint32_t> _surfelIndexLookups;
    SparseSurfelLandmarksIndex _surfelLandmarksIndex;
};
