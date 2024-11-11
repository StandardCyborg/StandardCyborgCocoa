//
//  SurfelFusion.cpp
//  StandardCyborgFusion
//
//  Created by eric on 2020-03-12.
//  Copyright © 2020 Standard Cyborg. All rights reserved.
//

#import <StandardCyborgFusion/PBFDefinitions.h>
#include <StandardCyborgFusion/EigenHelpers.hpp>

#import <standard_cyborg/util/DataUtils.hpp>
#include <standard_cyborg/util/IncludeEigen.hpp>

#include <standard_cyborg/util/DebugHelpers.hpp>

#include "SurfelFusion.hpp"

#include "DebugLog.h"

const std::vector<uint32_t>& SurfelFusion::getSurfelIndexLookups() const
{
    return _surfelIndexLookups;
}

SurfelFusion::SurfelFusion(std::shared_ptr<SurfelIndexMap> surfelIndexMap) :
    _surfelIndexMap(surfelIndexMap)
{
}

void SurfelFusion::cullLowConfidence(bool ignoreLifetime, int minWeight, Surfels& surfels, std::vector<int>* deletedSurfelList)
{
    size_t surfelCount = surfels.size();
    size_t compressedIndex = 0;
    
    for (size_t index = 0; index < surfelCount; ++index) {
        Surfel& surfel = surfels[index];
        
        if ((surfel.lifetime > 0 && !ignoreLifetime) || (surfel.weight >= minWeight)) {
            surfels[compressedIndex] = surfels[index];
            
            ++compressedIndex;
        } else if (deletedSurfelList != NULL) {
            deletedSurfelList->push_back((int)index);
        }
    }
    
    surfels.resize(compressedIndex);
}

void _addValuesAsNewSurfel(Vector3f position, Vector3f normal, Vector3f color, float weight, int surfelLifetime, float surfelSize, Surfels& surfels)
{
    Surfel surfel;
    surfel.position = position;
    surfel.normal = normal;
    surfel.surfelSize = surfelSize;
    
    surfel.color = color;
    surfel.weight = weight;
    surfel.lifetime = surfelLifetime;
    
    surfels.push_back(surfel);
}

void _integrateValuesIntoExistingSurfelAtIndex(size_t surfelIndex, Vector3f incomingPosition, Vector3f incomingNormal, float incomingNormalLength, Vector3f incomingColor, float incomingSurfelSize, float weight, int surfelLifetime, Surfels& surfels)
{
    Surfel& surfel = surfels[surfelIndex];
    float currentWeight = surfel.weight;
    float divTotalCount = 1.0 / (currentWeight + weight);
    
    surfel.position = (currentWeight * surfel.position + weight * incomingPosition) * divTotalCount;
    
    // Compute the length of the exising surfel normal
    Vector3f existingNormal = surfel.normal;
    
    // Average the normals to compute the new direction
    Vector3f newNormal = (currentWeight * existingNormal + weight * incomingNormal) * divTotalCount;
    newNormal.normalize();
    
    const float existingSurfelSize = surfel.surfelSize;
    
    // If the new sample normal is larger, use the original length. Otherwise average.
    float targetSurfelSize = (incomingSurfelSize > existingSurfelSize) ? existingSurfelSize : 0.5 * (existingSurfelSize + incomingSurfelSize);
    
    surfel.normal = newNormal;
    surfel.color = (currentWeight * surfel.color + weight * incomingColor) * divTotalCount;
    surfel.lifetime = surfelLifetime;
    surfel.weight += weight;
    surfel.surfelSize = targetSurfelSize;
}

bool SurfelFusion::doFusion(SurfelFusionConfiguration surfelFusionConfiguration,
                            ProcessedFrame& frame,
                            Surfels& surfels,
                            math::Mat4x4 extrinsicMatrix,
                            const std::vector<ScreenSpaceLandmark>* screenSpaceLandmarks,
                            SparseSurfelLandmarksIndex& surfelLandmarksIndex,
                            std::vector<int>& deletedSurfelIndicesList)
{
    const Eigen::Matrix3f extrinsicNormalMatrix = NormalMatrixFromMat4(toMatrix4f(extrinsicMatrix));
    
    const RawFrame& rawFrame = frame.rawFrame;
    const size_t width = rawFrame.width;
    const size_t height = rawFrame.height;
    
    // The first step is to render the existing surfel map into the perspective of
    // the incoming depth information. This will allow us to answer the question:
    // does an incoming depth sample land on top of an existing surfel?
    
    if (_surfelIndexLookups.size() != width * height) {
        _surfelIndexLookups = std::vector<uint32_t>(width * height, 0);
    }
    
    for (int ii = 0; ii < _surfelIndexLookups.size(); ++ii) {
        _surfelIndexLookups[ii] = EMPTY_SURFEL_INDEX;
    }
    
    bool surfelIndexMapDrawSuccess = _surfelIndexMap->draw(surfels, toMatrix4f(extrinsicMatrix).inverse(), rawFrame, _surfelIndexLookups);

    if (surfelIndexMapDrawSuccess == false) {
        return false;
    }

    // The next step is to iterate through the incoming depths and assimilate the
    // information.
    //
    // The counter is the *next* surfel index that will be written into for new
    // surfels. It starts at the end of the existing surfels, which means we need
    // to compress down our representation at each frame to remove any deleted
    // surfels. Fortunately that's a single sweep through the whole model.
    size_t assimilatedCount = 0;
    size_t newCount = 0;

#if DETAILED_PBF_MERGE_STATS
    size_t angleOfIncidenceRejections = 0;
    size_t maxDepthRejections = 0;
    size_t minDepthRejections = 0;
    size_t inputConfidenceRejections = 0;
    size_t mergeRadiusRejections = 0;
    size_t mergeRadiusSuccesses = 0;
#endif

    float cosAngleOfIncidenceThreshold = cos(surfelFusionConfiguration.maxSurfelIncidenceThreshold);
    float surfelMergeRadiusScaleFactorSquared = surfelFusionConfiguration.surfelMergeRadiusScaleFactor * surfelFusionConfiguration.surfelMergeRadiusScaleFactor;


    for (size_t row = 0; row < height; row++) {
        size_t baseIndex = width * row;
        for (size_t col = 0; col < width; col++) {
            size_t index = baseIndex + col;

            float depth = rawFrame.depths[index];

            // If it's an invalid depth, we don't have to do anything
            if (depth <= surfelFusionConfiguration.minDepth || depth > surfelFusionConfiguration.maxDepth) {
#if DETAILED_PBF_MERGE_STATS
                if (depth <= surfelFusionConfiguration.minDepth) minDepthRejections++;
                if (depth > surfelFusionConfiguration.maxDepth) maxDepthRejections++;
#endif
                continue;
            }

            float inputConfidence = frame.inputConfidences[index];
            if (inputConfidence < surfelFusionConfiguration.inputConfidenceThreshold) {
#if DETAILED_PBF_MERGE_STATS
                inputConfidenceRejections++;
#endif
                continue;
            }

            uint32_t surfelIndex = _surfelIndexLookups[index];
            assert(surfelIndex < surfels.size() || surfelIndex == EMPTY_SURFEL_INDEX);

            // If there's no surfel here, add it
            const Vector3f incomingPosition = standard_cyborg::toVector3f(frame.positions[index]);
            const Vector3f incomingNormal =  standard_cyborg::toVector3f(frame.normals[index]);
            const float incomingSurfelSize = frame.surfelSizes[index];
            
            // *Before transforming*, check the angle of incidence since this is much easier
            // if we don't have to consider transforms here

            // Since the camera is (by definition, at the time of the first frame) fixed at the origin,
            // the dot product of a surfel's position with its normal defines its angle of incidence
            // with the camera, allowing to compute the angle of incidence at which it was observed.
            // The negative makes observed angles positive, for simplicity.
            float cosAngleOfIncidence = -incomingPosition.dot(incomingNormal) / incomingPosition.norm();
            
            if (cosAngleOfIncidence < cosAngleOfIncidenceThreshold) {
#if DETAILED_PBF_MERGE_STATS
                angleOfIncidenceRejections++;
#endif
                continue;
            }

            // Scale by the square of the cosine. For no particular reason than because it weights glancing angles
            // a bit less than simply the cosine.
            float angleWeighting = cosAngleOfIncidence * cosAngleOfIncidence;

            // Now transform the incoming position into the frame of reference of the surfels
            // This is really just = extrinsicMatrix * incomingPosition
            Vector3f incomingPositionInModelFrame = Vec3TransformMat4(incomingPosition, toMatrix4f(extrinsicMatrix));
            Vector3f incomingNormalInModelFrame = extrinsicNormalMatrix * incomingNormal;

            // If the incoming point is too far away from the surfel it landed on, trigger a new surfel creation
            if (surfelIndex != EMPTY_SURFEL_INDEX) {
                Surfel& surfel = surfels[surfelIndex];
                Vector3f positionDelta = incomingPositionInModelFrame - surfel.position;

                // Uncertainty scales with the *square* of depth, so we normalize by the square of
                // depth in order to get a depth-corrected merge tolerance
                float depth4 = depth * depth;
                depth4 *= depth4;
                
                float depthCorrectedMergeRadiusFactorSquared = positionDelta.squaredNorm() / depth4;

                if (depthCorrectedMergeRadiusFactorSquared > surfelMergeRadiusScaleFactorSquared) {
#if DETAILED_PBF_MERGE_STATS
                    mergeRadiusRejections++;
#endif
                    surfelIndex = EMPTY_SURFEL_INDEX;
                } else {
#if DETAILED_PBF_MERGE_STATS
                    mergeRadiusSuccesses++;
#endif
                }
            }

            Vector3f incomingColor = standard_cyborg::toVector3f(frame.rawFrame.colors[index]);

            if (surfelIndex == EMPTY_SURFEL_INDEX) {
                _addValuesAsNewSurfel(incomingPositionInModelFrame,
                                      incomingNormalInModelFrame,
                                      incomingColor,
                                      angleWeighting,
                                      surfelFusionConfiguration.surfelLifetime,
                                      incomingSurfelSize,
                                      surfels);
                newCount++;
            } else {
                _integrateValuesIntoExistingSurfelAtIndex(surfelIndex,
                                                          incomingPositionInModelFrame,
                                                          incomingNormalInModelFrame,
                                                          1.0,
                                                          incomingColor,
                                                          incomingSurfelSize,
                                                          angleWeighting,
                                                          surfelFusionConfiguration.surfelLifetime,
                                                          surfels);
                assimilatedCount++;
            }
        }
    }


    if (screenSpaceLandmarks != NULL) {
        for (auto screenSpaceLandmark : *screenSpaceLandmarks) {
            // Convert (x, y) in [0, 1] x [0, 1] to a raster image position (row, column)
            off_t col = std::max(0, std::min((int)(width - 1), (int)(screenSpaceLandmark.x * width)));
            off_t row = std::max(0, std::min((int)(height - 1), (int)(screenSpaceLandmark.y * height)));
            off_t index = width * row + col;
            uint32_t surfelIndex = _surfelIndexLookups[index];

            // If this didn't land on a surfel *as rasterized from the current camera position
            // estimate but before this frame's surfels were assimilated*, skip it. Please note
            // this is a *major* simplifying assumption we're making so that we don't have to
            // re-rasterize the surfels a second time for this frame, after assimilating.
            if (surfelIndex == EMPTY_SURFEL_INDEX) continue;

            surfelLandmarksIndex.addHit(surfelIndex, screenSpaceLandmark.landmarkId);
        }
    }

    if (surfelFusionConfiguration.cullLowConfidence) {
        if (screenSpaceLandmarks == NULL && surfelLandmarksIndex.size() == 0) {
            this->cullLowConfidence(surfelFusionConfiguration.ignoreLifetime, surfelFusionConfiguration.minCount, surfels);
        } else {
            // If there are landmarks, we have to track deleted surfels so we can renumber the
            // landmarks index.

            // Flush any previously-deleted surfels from this list without deallocating the memory.
            // This is just a microoptimization to avoid constantly allocating and deallocating a
            // ~4000 element vector in favor of just storing the high-water mark and overwriting.
            deletedSurfelIndicesList.clear();

            // Cull low confidence surfels and store the deleted surfels in a list for the sake
            // of renumbering the surfel landmark index
            this->cullLowConfidence(surfelFusionConfiguration.ignoreLifetime, surfelFusionConfiguration.minCount, surfels, &deletedSurfelIndicesList);

            // Delete and renumber sparse surfel landmark storage
            surfelLandmarksIndex.deleteSurfelLandmarksAndRenumber(deletedSurfelIndicesList);
        }
    }
 
    // Decay the lifetimes by one step
    for (auto& surfel : surfels) {
        surfel.lifetime--;
    }

    #if DETAILED_PBF_MERGE_STATS
        std::cout << "\tsurfel count = " << surfels.size() << std::endl;
        std::cout << "\tindex lookup count = " << _surfelIndexLookups.size() << std::endl;
        std::cout << "\tsurfel rejections:\n"
                  << "\t             min depth:" << minDepthRejections << "\n"
                  << "\t             max depth:" << maxDepthRejections << "\n"
                  << "\t    angle of incidence:" << angleOfIncidenceRejections << "\n"
                  << "\t      input confidence:" << inputConfidenceRejections << "\n"
                  << "\t          merge radius:" << mergeRadiusRejections << "\n\n"
                  << "\tmerge radius successes:" << mergeRadiusSuccesses << "\n"
                  << "\t   assimilated surfels:" << assimilatedCount << "\n"
                  << "\t           new surfels:" << newCount << std::endl;
    #endif
    
    return true;
}

void SurfelFusion::finish(SurfelFusionConfiguration surfelFusionConfiguration,
                          Surfels& surfels,
                          SparseSurfelLandmarksIndex& surfelLandmarksIndex,
                          std::vector<int>& deletedSurfelIndicesList)
{
    size_t preCulledCount = surfels.size();
    
    if (surfelFusionConfiguration.cullLowConfidence) {
        if (surfelLandmarksIndex.size() == 0) {
            this->cullLowConfidence(true, surfelFusionConfiguration.minCount, surfels);
        } else {
            // If there are landmarks, we have to track deleted surfels so we can renumber the
            // landmarks index.
            
            // Flush any previously-deleted surfels from this list without deallocating the memory.
            // This is just a microoptimization to avoid constantly allocating and deallocating a
            // ~4000 element vector in favor of just storing the high-water mark and overwriting.
            deletedSurfelIndicesList.clear();
            
            // Cull low confidence surfels and store the deleted surfels in a list for the sake
            // of renumbering the surfel landmark index
            this->cullLowConfidence(true, surfelFusionConfiguration.minCount, surfels, &deletedSurfelIndicesList);
            
            // Delete and renumber sparse surfel landmark storage
            surfelLandmarksIndex.deleteSurfelLandmarksAndRenumber(deletedSurfelIndicesList);
        }
    }
    
    size_t postCulledCount = surfels.size();
    
    DEBUG_LOG("Culled %lu points from the last %d frames; %zu remain",
              preCulledCount - postCulledCount, surfelFusionConfiguration.surfelLifetime, postCulledCount);
    
}
