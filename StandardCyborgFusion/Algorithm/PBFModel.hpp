//
//  PBFModel.hpp
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 7/25/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#pragma once

#include <vector>

#include <StandardCyborgFusion/FastRand.hpp>
#include <standard_cyborg/util/IncludeEigen.hpp>
#include <standard_cyborg/sc3d/PerspectiveCamera.hpp>
#include <StandardCyborgFusion/ICP.hpp>
#include <StandardCyborgFusion/Surfel.hpp>
#include <StandardCyborgFusion/PBFFinalStatistics.h>
#include <StandardCyborgFusion/SurfelFusion.hpp>


#include <standard_cyborg/sc3d/Geometry.hpp>

#include <StandardCyborgFusion/ScreenSpaceLandmark.hpp>
#include <StandardCyborgFusion/SparseSurfelLandmarksIndex.hpp>
#include "PBFAssimilatedFrameMetadata.hpp"
#include "ProcessedFrame.hpp"
#include "PBFConfiguration.hpp"

using namespace standard_cyborg;

class PBFModel {
public:
    PBFModel(std::shared_ptr<SurfelIndexMap> surfelIndexMap, unsigned int randomSeed = 0);
    ~PBFModel();

    void setICPIterationCallback(ICPIterationCallback callback);

    PBFAssimilatedFrameMetadata assimilate(ProcessedFrame& frame,
                                           PBFConfiguration pbfConfig,
                                           ICPConfiguration icpConfig,
                                           SurfelFusionConfiguration surfelFusionConfiguration,
                                           double currentTime,
                                           const std::vector<ScreenSpaceLandmark>* screenSpaceLandmarks = NULL);

    PBFFinalStatistics finishAssimilating(SurfelFusionConfiguration surfelFusionConfiguration);

    void reset(unsigned int randomSeed = 0);
    
    std::shared_ptr<sc3d::Geometry> buildPointCloud(float downsampledFraction = 1.0f);
    Eigen::Matrix4f getCurrentExtrinsicMatrix();
    const Surfels& getSurfels() const;
    const std::vector<uint32_t>& getSurfelIndexMap() const;
    const SparseSurfelLandmarksIndex& getSurfelLandmarksIndex() const;
    const std::vector<PBFAssimilatedFrameMetadata> getAssimilatedFrameMetadata() const;

private:
    ICPIterationCallback _icpCallback;
    std::vector<PBFAssimilatedFrameMetadata> _assimilatedFrameMetadatas;
    FastRand _fastRNG;

    Surfels _surfels;
    std::shared_ptr<sc3d::Geometry> _ICPTargetCloud;
    
    SparseSurfelLandmarksIndex _surfelLandmarksIndex;
    std::vector<int> _deletedSurfelIndicesList;
    
    SurfelFusion _surfelFusion;

    Eigen::Matrix4f _extrinsicMatrix = Eigen::Matrix4f::Identity();

    void _cullLowConfidence(bool ignoreLifetime, int minWeight, std::vector<int>* deletedSurfelList = NULL);
    ICPResult _runICP(ProcessedFrame& frame, SurfelFusionConfiguration surfelFusionConfiguration, ICPConfiguration icpConfig, PBFConfiguration pbfConfig);
    
    PBFAssimilatedFrameMetadata* _nthMostRecentValidFrameMetadata(size_t offset = 0);
    PBFFinalStatistics _calcFinalStatistics();

    // Prohibit copying and assignment
    PBFModel(const PBFModel&) = delete;
    PBFModel& operator=(const PBFModel&) = delete;
};
