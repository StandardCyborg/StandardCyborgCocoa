//
//  OfflineSurfelLandmarking.cpp
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 5/28/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#include <standard_cyborg/util/IncludeEigen.hpp>
#include <StandardCyborgFusion/PBFDefinitions.h>
#include <iostream>

#include "OfflineSurfelLandmarking.hpp"

OfflineSurfelLandmarking::OfflineSurfelLandmarking(std::shared_ptr<SurfelIndexMap> surfelIndexMap) :
    _surfelIndexMap(surfelIndexMap)
{ }

OfflineSurfelLandmarking::~OfflineSurfelLandmarking() { }

void OfflineSurfelLandmarking::placeLandmarksOnSurfels(const Surfel* surfels,
                                                       size_t surfelCount,
                                                       size_t frameWidth,
                                                       size_t frameHeight,
                                                       Eigen::Matrix4f viewProjectionMatrix,
                                                       const std::vector<ScreenSpaceLandmark>& screenSpaceLandmarks)
{
    if (_surfelIndexLookups.size() != frameWidth * frameHeight) {
        _surfelIndexLookups = std::vector<uint32_t>(frameWidth * frameHeight, 0);
    }
    
    for(int ii = 0; ii < _surfelIndexLookups.size(); ++ii) {
        _surfelIndexLookups[ii] = EMPTY_SURFEL_INDEX;
    }
    
    bool success = _surfelIndexMap->drawForColor(surfels,
                                                 surfelCount,
                                                 viewProjectionMatrix,
                                                 frameWidth,
                                                 frameHeight,
                                                 _surfelIndexLookups);
    
    if (!success) {
        std::cerr << "Failed to draw surfel index map for color" << std::endl;
        return;
    }
    
    for (auto screenSpaceLandmark : screenSpaceLandmarks) {
        // Convert (x, y) in [0, 1] x [0, 1] to a raster image position (row, column)
        off_t col = std::max(0, std::min((int)(frameWidth - 1), (int)(screenSpaceLandmark.x * frameWidth)));
        off_t row = std::max(0, std::min((int)(frameHeight - 1), (int)(screenSpaceLandmark.y * frameHeight)));
        off_t index = frameWidth * row + col;
        uint32_t surfelIndex = _surfelIndexLookups[index];
        
        // If this didn't land on a surfel *as rasterized from the current camera position
        // estimate but before this frame's surfels were assimilated*, skip it. Please note
        // this is a *major* simplifying assumption we're making so that we don't have to
        // re-rasterize the surfels a second time for this frame, after assimilating.
        if (surfelIndex == EMPTY_SURFEL_INDEX) continue;
        
        // std::cout << "Added hit for surfel index " << surfelIndex << ": landmark " << screenSpaceLandmark.landmarkId << std::endl;
        _surfelLandmarksIndex.addHit(surfelIndex, screenSpaceLandmark.landmarkId);
    }
}

std::unordered_map<int, Eigen::Vector3f> OfflineSurfelLandmarking::computeLandmarks(const Surfel* surfels)
{
    return _surfelLandmarksIndex.computeCentroids(surfels);
}

void OfflineSurfelLandmarking::reset()
{
    _surfelIndexLookups = std::vector<uint32_t>();
    _surfelLandmarksIndex.removeAllHits();
}

const std::vector<uint32_t>& OfflineSurfelLandmarking::getSurfelIndexLookups()
{
    return _surfelIndexLookups;
}
