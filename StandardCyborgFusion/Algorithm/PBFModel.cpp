//
//  PBFModel.mm
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 7/25/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#include "PBFModel.hpp"
#include "crc32.hpp"
#include "DebugLog.h"
#include "EigenHelpers.hpp"
#include "GeometryHelpers.hpp"
#include "MathHelpers.h"

#include <iostream>
#include "crc32.hpp"
#include <StandardCyborgFusion/PBFDefinitions.h>
#include <standard_cyborg/util/DataUtils.hpp>
#include <cmath>

#import <StandardCyborgFusion/PBFFinalStatistics.h>

using namespace Eigen;

#define DETAILED_PBF_MERGE_STATS 0
#if DETAILED_PBF_MERGE_STATS
#warning "Expensive PBF merge stats enabled; disable before committing"
#endif

struct CameraVelocity {
    Vector3f angularVelocity;
    Vector3f velocity;
};


PBFModel::PBFModel(std::shared_ptr<SurfelIndexMap> surfelIndexMap, unsigned int randomSeed) :
    _surfelFusion(surfelIndexMap)
{
    _fastRNG.seed(randomSeed);
}

PBFModel::~PBFModel() {}

const Surfels& PBFModel::getSurfels() const
{
    return _surfels;
}

const std::vector<uint32_t>& PBFModel::getSurfelIndexMap() const
{
    return _surfelFusion.getSurfelIndexLookups();
}

const SparseSurfelLandmarksIndex& PBFModel::getSurfelLandmarksIndex() const
{
    return _surfelLandmarksIndex;
}

void PBFModel::setICPIterationCallback(ICPIterationCallback callback)
{
    _icpCallback = callback;
}

Matrix4f PBFModel::getCurrentExtrinsicMatrix()
{
    return _extrinsicMatrix;
}

const std::vector<PBFAssimilatedFrameMetadata> PBFModel::getAssimilatedFrameMetadata() const
{
    return _assimilatedFrameMetadatas;
}

std::shared_ptr<sc3d::Geometry> PBFModel::buildPointCloud(float downsampledFraction)
{
    assert(downsampledFraction > 0.0f);
    assert(downsampledFraction <= 1.0f);

    size_t surfelCount = _surfels.size();
    size_t surfelStride = (size_t)(1.0f / downsampledFraction);
    size_t resultCount = surfelCount / surfelStride;

    std::vector<math::Vec3> vertices;
    vertices.reserve(resultCount);
   
    std::vector<math::Vec3> normals;
    normals.reserve(resultCount);
    
    std::vector<math::Vec3> colors;
    colors.reserve(resultCount);
    
    // Randomly select an index within each stride interval
    std::vector<off_t> indexes(resultCount);
    for (int i = 0; i < resultCount; ++i) {
        indexes[i] = i * surfelStride + _fastRNG.sample((int)surfelStride);
    }

    for (size_t i = 0; i < resultCount; ++i) {
        off_t surfelIndex = indexes[i];
        
        Surfel& surfel = _surfels[surfelIndex];
        
        vertices.push_back(toVec3(surfel.position));
        normals.push_back(toVec3(surfel.normal));
        colors.push_back(toVec3(surfel.color));
    }

    return std::shared_ptr<sc3d::Geometry>(new sc3d::Geometry(vertices, normals, colors));
}

PBFAssimilatedFrameMetadata* PBFModel::_nthMostRecentValidFrameMetadata(size_t offset)
{
    size_t frameMetaCount = _assimilatedFrameMetadatas.size();
    size_t validCount = 0;
    for (off_t i = frameMetaCount - 1; i >= 0; i--) {
        if (_assimilatedFrameMetadatas[i].isMerged) {
            if (validCount == offset) {
                return &(_assimilatedFrameMetadatas[i]);
            }

            validCount++;
        }
    }

    return nullptr;
}

// Return the angular velocity (in radians per second) of the x, y, and z axes of the camera
// as well as the velocity of the camera position
static CameraVelocity _cameraVelocity(PBFAssimilatedFrameMetadata* previousFrameMeta, PBFAssimilatedFrameMetadata* currentFrameMeta)
{
    if (currentFrameMeta == nullptr || previousFrameMeta == nullptr || currentFrameMeta->timestamp < 0.0 || previousFrameMeta->timestamp < 0.0 || currentFrameMeta->timestamp == previousFrameMeta->timestamp) {
        return CameraVelocity{
            Vector3f(NAN, NAN, NAN),
            Vector3f(NAN, NAN, NAN)};
    }

    Matrix4f extrinsicMatrixInverse = currentFrameMeta->viewMatrix.inverse();
    Matrix4f previousExtrinsicMatrixInverse = previousFrameMeta->viewMatrix.inverse();

    Vector3f currentX = extrinsicMatrixInverse.col(0).head<3>();
    Vector3f currentY = extrinsicMatrixInverse.col(1).head<3>();
    Vector3f currentZ = extrinsicMatrixInverse.col(2).head<3>();
    Vector3f currentP = extrinsicMatrixInverse.col(3).head<3>();

    Vector3f previousX = previousExtrinsicMatrixInverse.col(0).head<3>();
    Vector3f previousY = previousExtrinsicMatrixInverse.col(1).head<3>();
    Vector3f previousZ = previousExtrinsicMatrixInverse.col(2).head<3>();
    Vector3f previousP = previousExtrinsicMatrixInverse.col(3).head<3>();

#if DEBUG
    // This is going to be running VERY slowly, so force-override the delta time to 1/30s
    double deltaT = 1.0 / 30.0;
#else
    double deltaT = (double)(currentFrameMeta->timestamp - previousFrameMeta->timestamp);
#endif
    
    return CameraVelocity{
        Vector3f(
            acosf(currentX.dot(previousX)),
            acosf(currentY.dot(previousY)),
            acosf(currentZ.dot(previousZ)))
            / deltaT,
        (currentP - previousP) / deltaT};
}

PBFAssimilatedFrameMetadata PBFModel::assimilate(ProcessedFrame& frame,
                                                 PBFConfiguration pbfConfig,
                                                 ICPConfiguration icpConfig,
                                                 SurfelFusionConfiguration surfelFusionConfiguration,
                                                 double currentTime,
                                                 const std::vector<ScreenSpaceLandmark>* screenSpaceLandmarks)
{
    // Summary of algorithm:
    // The first frame is defined to be identity for the world coordinates
    // Incoming frames are transformed to world coordinates when un-projecting to point clouds for ICP
    // ICP finds a transform from incoming point cloud back to world coordinates
    // That resulting transform is multiplied into the world transform
    // The surfel index map is drawn from the point of view of the incoming frame (inverse world transform)
    // Points in the new frame are un-projected into 3D based on the world transform
    
    // Get the current most recent metadata (haven't pushed this frame's metadata yet)
    PBFAssimilatedFrameMetadata* previousFrameMeta = _nthMostRecentValidFrameMetadata(0);

    PBFAssimilatedFrameMetadata frameMeta;
    frameMeta.isMerged = false;
    frameMeta.viewMatrix = Matrix4f::Identity();
    frameMeta.timestamp = currentTime;
    frameMeta.icpUnusedIterationFraction = 1.0f;
    frameMeta.projectionMatrix = toMatrix4f(frame.rawFrame.camera.getProjectionViewMatrix());

    const RawFrame& rawFrame = frame.rawFrame;
    const size_t width = rawFrame.width;
    const size_t height = rawFrame.height;
    
    if (_surfels.size() > 0) {
        ICPResult icpResult = _runICP(frame, surfelFusionConfiguration, icpConfig, pbfConfig);

        Matrix4f extrinsicMatrixTmp = toMatrix4f(icpResult.sourceTransform) * _extrinsicMatrix;
        // Store this whether or not we end up using it since we also store information about whether
        // the frame was assimilated or not
        frameMeta.viewMatrix = extrinsicMatrixTmp;
        frameMeta.icpIterationCount = icpResult.iterationCount;
        frameMeta.correspondenceError = icpResult.rmsCorrespondenceError;
        
        if (!icpResult.succeeded) {
            DEBUG_LOG("ICP rejected due to bad convergence after %d/%d iterations", icpResult.iterationCount, icpConfig.maxIterations);
            frameMeta.icpUnusedIterationFraction = 0;
        } else {
            frameMeta.icpUnusedIterationFraction = 1.0f - (float)icpResult.iterationCount / (float)icpConfig.maxIterations;
            
            CameraVelocity cv = _cameraVelocity(previousFrameMeta, &frameMeta);
            
            if (cv.angularVelocity.hasNaN() || cv.angularVelocity.norm() > pbfConfig.maxCameraAngularVelocity) {
                DEBUG_LOG("Rejecting ICP due to bad fit with angular velocity %f", cv.angularVelocity.norm());
                frameMeta.icpUnusedIterationFraction = 0;
            }
            
            else if (cv.velocity.hasNaN() || cv.velocity.norm() > pbfConfig.maxCameraVelocity) {
                DEBUG_LOG("Rejecting ICP due to bad fit with linear velocity %f", cv.velocity.norm());
                frameMeta.icpUnusedIterationFraction = 0;
            }
        }
        
        if (frameMeta.icpUnusedIterationFraction > 0) {
            _extrinsicMatrix = extrinsicMatrixTmp;
        } else {
            // It didn't converge in time, so bail out
            DEBUG_LOG("ICP didn't converge with enough quality (%f) after %d/%d iterations. Ignoring frame.", frameMeta.icpUnusedIterationFraction, icpResult.iterationCount, icpConfig.maxIterations);
            _assimilatedFrameMetadatas.push_back(frameMeta);
            return frameMeta;
        }
    }

    if (_surfels.size() == 0) {
        // Initialize the _surfels vector to a realistic eventual size
        _surfels.reserve(width * height);
    }
    
    if (!_surfelFusion.doFusion(surfelFusionConfiguration,
                                frame,
                                _surfels,
                                toMat4x4(_extrinsicMatrix),
                                screenSpaceLandmarks,
                                _surfelLandmarksIndex,
                                _deletedSurfelIndicesList)
    ) {
        DEBUG_LOG("Frame couldn't be fused.");
        frameMeta.icpUnusedIterationFraction = 0;
    } else {
        frameMeta.isMerged = true;
        frameMeta.surfelCount = _surfels.size();
    }
    
    _assimilatedFrameMetadatas.push_back(frameMeta);

    return frameMeta;
}


PBFFinalStatistics PBFModel::_calcFinalStatistics()
{
    double startTime = -1, endTime = -1;
    int mergedFrameCount = 0;
    int failedFrameCount = 0;
    int accumulatedICPIterationCount = 0;
    
    float sumCorrespondenceError = 0;

    for (auto meta : _assimilatedFrameMetadatas) {
        if (startTime == -1) {
            startTime = meta.timestamp;
        }

        endTime = meta.timestamp;

        if (meta.isMerged) {
            ++mergedFrameCount;
            sumCorrespondenceError += meta.correspondenceError;
        } else {
            ++failedFrameCount;
        }

        accumulatedICPIterationCount += meta.icpIterationCount;
    }

    double duration = endTime - startTime;
    double framerate = mergedFrameCount / duration;
    double averageICPIterations = (double)accumulatedICPIterationCount / (double)mergedFrameCount;

    PBFFinalStatistics finalStatistics;
    finalStatistics.mergedFrameCount = mergedFrameCount;
    finalStatistics.framerate = framerate;
    finalStatistics.averageICPIterations = averageICPIterations;
    finalStatistics.failedFrameCount = failedFrameCount;
    finalStatistics.averageCorrespondenceError = sumCorrespondenceError / (float)mergedFrameCount;
    
    return finalStatistics;
}

PBFFinalStatistics PBFModel::finishAssimilating(SurfelFusionConfiguration surfelFusionConfiguration)
{
    PBFFinalStatistics finalStatistics = _calcFinalStatistics();

    DEBUG_LOG("Finished assimilating %d frames\n\tAverage framerate: %.2f FPS\n\tAverage ICP iterations: %.2f\n\tRejected frames: %d",
              finalStatistics.mergedFrameCount,
              finalStatistics.framerate,
              finalStatistics.averageICPIterations,
              finalStatistics.failedFrameCount);
    
    if (surfelFusionConfiguration.cullLowConfidence == false) { return finalStatistics; }

    const int minFinalCullFrameCount = surfelFusionConfiguration.surfelLifetime * 2;
    if (_assimilatedFrameMetadatas.size() < minFinalCullFrameCount) {
        DEBUG_LOG("Not applying final cull for only %ld frames", _assimilatedFrameMetadatas.size());
        return finalStatistics;
    }

    _surfelFusion.finish(surfelFusionConfiguration, _surfels, _surfelLandmarksIndex, _deletedSurfelIndicesList);

    return finalStatistics;
}

void PBFModel::reset(unsigned int randomSeed)
{
    DEBUG_LOG("Resetting");

    _extrinsicMatrix.setIdentity();

    _fastRNG.seed(randomSeed);

    _surfelLandmarksIndex.removeAllHits();
    
    _surfels.clear();
    _assimilatedFrameMetadatas.clear();
    _ICPTargetCloud = nullptr;
}

// MARK: - Private

ICPResult PBFModel::_runICP(ProcessedFrame& frame, SurfelFusionConfiguration surfelFusionConfiguration, ICPConfiguration icpConfig, PBFConfiguration pbfConfig)
{
    bool shouldRebuildPointCloud = false;
    if (_assimilatedFrameMetadatas.size() < pbfConfig.kdTreeRebuildInterval / 2 || _ICPTargetCloud == nullptr) {
        // If we've scanned fewer frames than half the rebuild interval, always rebuild the point cloud
        shouldRebuildPointCloud = true;
    } else if (_assimilatedFrameMetadatas.size() < 20) {
        // If we have a few more frames worth of data to work with, slow down the rebuilds halfway
        shouldRebuildPointCloud = _assimilatedFrameMetadatas.size() % (pbfConfig.kdTreeRebuildInterval / 2) == 0;
    } else {
        shouldRebuildPointCloud = _assimilatedFrameMetadatas.size() % pbfConfig.kdTreeRebuildInterval == 0;
    }
    
    if (shouldRebuildPointCloud) {
        _ICPTargetCloud = buildPointCloud(pbfConfig.icpDownsampleFraction);
    }
    
    // Create a downsampled copy of the points for running ICP,
    // using the transform mapping the existing points and normals into the most recent frame of reference
    
    std::vector<math::Vec3> downsampledVertices;
    std::vector<math::Vec3> downsampledNormals;
    std::vector<math::Vec3> downsampledColors;
    {
        downsampledColors.reserve(frame.rawFrame.colors.size());
        downsampledNormals.reserve(frame.normals.size());
        downsampledVertices.reserve(frame.positions.size());
        
        float downsampledFraction = pbfConfig.icpDownsampleFraction;
        
        Matrix3f normalTransform = NormalMatrixFromMat4(_extrinsicMatrix);
        
        // Filter by depth
        size_t pointCount = frame.positions.size();
        size_t filteredCount = 0;
        size_t maxCount = (size_t)((float)pointCount * downsampledFraction);
        
        for (off_t i = 0; i < pointCount && filteredCount < maxCount; ++i) {
            // Add in a factor of four because otherwise the feature and center weighting-based sampling
            // will actually select far fewer than the requested number. To get the correct number of samples
            // we'd need to sort and take the correct percentile so that this is a bit of a shot in the dark
            // which will change based on the particular depth  map, but hopefully it's a reasonable guess.
            if (downsampledFraction < 1 && _fastRNG.sample(1000) > downsampledFraction * 1000.0f * 4.0) continue;
            
            float depth = frame.rawFrame.depths[i];
            
            if (_fastRNG.sample(1000) > frame.weights[i] * 1000.0f) continue;
            if (depth < surfelFusionConfiguration.minDepth || depth > surfelFusionConfiguration.maxDepth) continue;
          
            downsampledVertices.push_back(standard_cyborg::toVec3(  Vec3TransformMat4( toVector3f(frame.positions[i]), _extrinsicMatrix)  ) );
            
            downsampledNormals.push_back(standard_cyborg::toVec3(normalTransform * standard_cyborg::toVector3f(frame.normals[i])));
            
            downsampledColors.push_back(frame.rawFrame.colors[i]);
            
            ++filteredCount;
        }

    }

    sc3d::Geometry downsampledSourceCloud(downsampledVertices,
                                      downsampledNormals,
                                      downsampledColors);
    
    ICPResult icpResult = ICP::run(icpConfig,
                                   downsampledSourceCloud,
                                   *_ICPTargetCloud,
                                   _icpCallback);

#if DETAILED_PBF_MERGE_STATS
    DEBUG_LOG("ICP took %d iterations, resulting in RMS source-target error %f",
              icpResult.iterationCount,
              icpResult.rmsCorrespondenceError);
#endif

    return icpResult;
}
