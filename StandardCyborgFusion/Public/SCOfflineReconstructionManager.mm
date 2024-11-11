//
//  SCOfflineReconstructionManager.mm
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 12/13/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreVideo/CoreVideo.h>

#include <nlohmann/json.hpp>

#import <StandardCyborgFusion/DepthProcessor.hpp>
#import <StandardCyborgFusion/GeometryHelpers.hpp>
#import <StandardCyborgFusion/MetalDepthProcessor.hpp>
#import <StandardCyborgFusion/MetalSurfelIndexMap.hpp>
#import <StandardCyborgFusion/PBFModel.hpp>
#import <StandardCyborgFusion/SurfelFusion.hpp>
#import <StandardCyborgFusion/PointCloudIO.hpp>
#import <StandardCyborgFusion/SCAssimilatedFrameMetadata.h>
#import <StandardCyborgFusion/SCAssimilatedFrameMetadata_Private.h>
#import <StandardCyborgFusion/SCOfflineReconstructionManager.h>
#import <StandardCyborgFusion/SCOfflineReconstructionManager_Private.h>
#import <StandardCyborgFusion/SCPointCloud_Private.h>

#import <standard_cyborg/util/DataUtils.hpp>

#import "GravityEstimator.hpp"

#include <fstream>

using JSON = nlohmann::json;
using namespace standard_cyborg;

@implementation SCOfflineReconstructionManager {
    id<MTLDevice> _metalDevice;
    id<MTLCommandQueue> _metalCommandQueue;
    std::shared_ptr<DepthProcessor> _depthProcessor;
    std::shared_ptr<MetalSurfelIndexMap> _surfelIndexMap;
    std::shared_ptr<PBFModel> _pbfModel;
    std::shared_ptr<RawFrame> _lastRawFrame;

    GravityEstimator _gravityEstimator;

    PBFConfiguration _pbfConfig;
    ICPConfiguration _icpConfig;
    SurfelFusionConfiguration _surfelFusionConfig;
    __weak id<SCOfflineReconstructionManagerDelegate> _delegate;
    BOOL _hasFinalized;
}

// MARK: SCReconstructionManagerParameters

- (float)icpDownsampleFraction
{
    return _pbfConfig.icpDownsampleFraction;
}

- (void)setICPDownsampleFraction:(float)fraction
{
    _pbfConfig.icpDownsampleFraction = fraction;
}

- (int)maxThreadCount
{
    return _pbfConfig.icpDownsampleFraction;
}

- (void)setMaxThreadCount:(int)count
{
    _icpConfig.threadCount = count;
}

- (float)icpOutlierDeviationsThreshold
{
    return _icpConfig.outlierDeviationsThreshold;
}

- (void)setICPOutlierDeviationsThreshold:(float)threshold
{
    _icpConfig.outlierDeviationsThreshold = threshold;
}

- (float)icpTolerance
{
    return _icpConfig.tolerance;
}

- (void)setICPTolerance:(float)fraction
{
    _icpConfig.tolerance = fraction;
}

- (int)maxICPIterations
{
    return _icpConfig.maxIterations;
}

- (void)setMaxICPIterations:(int)maxIterations
{
    _icpConfig.maxIterations = maxIterations;
}

- (float)minDepth
{
    return _surfelFusionConfig.minDepth;
}

- (void)setMinDepth:(float)minDepth
{
    _surfelFusionConfig.minDepth = minDepth;
}

- (float)maxDepth
{
    return _surfelFusionConfig.maxDepth;
}

- (void)setMaxDepth:(float)maxDepth
{
    _surfelFusionConfig.maxDepth = maxDepth;
}

// MARK: - Public

- (instancetype)initWithDevice:(id<MTLDevice>)device
                  commandQueue:(id<MTLCommandQueue>)commandQueue
                maxThreadCount:(int)maxThreadCount
{
    self = [super init];
    if (self) {
        _metalDevice = device;
        _metalCommandQueue = commandQueue;
        _icpConfig.threadCount = maxThreadCount;

        _depthProcessor = std::shared_ptr<DepthProcessor>(new MetalDepthProcessor(_metalDevice, _metalCommandQueue));

        [self _reinitializePBFModel];
    }
    return self;
}

- (void)_reinitializePBFModel
{
    _surfelIndexMap = std::shared_ptr<MetalSurfelIndexMap>(new MetalSurfelIndexMap(_metalDevice, _metalCommandQueue));
    _pbfModel = std::shared_ptr<PBFModel>(new PBFModel(_surfelIndexMap));

    __weak SCOfflineReconstructionManager *weakSelf = self;
    _pbfModel->setICPIterationCallback([weakSelf](ICPResult result) {
        [weakSelf _ICPIteratedWithResult:result];
    });
}


- (void)setMotionDataPath:(NSString *)filePath
{
    JSON json;
    std::string fp = [filePath UTF8String];

    std::ifstream inputStream(fp.c_str());
    inputStream >> json;

    size_t gravityFieldsPresent = json[0].count("gravity_x") + json[0].count("gravity_y") + json[0].count("gravity_z") + json[0].count("attitude_x") + json[0].count("attitude_y") + json[0].count("attitude_z") + json[0].count("attitude_w");

    // Ensure all entries are present, otherwise don't accumulate the values.
    if (gravityFieldsPresent == 7) {
        _gravityEstimator.accumulate(simd_make_float3(json[0]["gravity_x"], json[0]["gravity_y"], json[0]["gravity_z"]),
                                     simd_make_float4(json[0]["attitude_x"], json[0]["attitude_y"], json[0]["attitude_z"], json[0]["attitude_w"]));
    }

}

- (simd_float3)gravity
{
    return _gravityEstimator.getGravity();
}

- (simd_float3x3)gravityAlignedAxes
{
    return _gravityEstimator.getCoordinateFrame();
}

- (SCAssimilatedFrameMetadata)accumulateFromBPLYWithPath:(NSString *)filePath
{
    auto rawFrame = [self readBPLYWithPath:filePath];
    
    auto result = [self accumulateFromRawFrame:*rawFrame];
    
    _lastRawFrame = std::move(rawFrame);
    
    return result;
}

- (SCPointCloud *)reconstructRawFrameFromBPLYAtPath:(NSString *)bplyPath
{
    std::unique_ptr<RawFrame> rawFrame([self readBPLYWithPath:bplyPath]);

    ProcessedFrame processedFrame(*rawFrame);

    _depthProcessor->computeFrameValues(processedFrame, *rawFrame);

    size_t depthCount = processedFrame.positions.size();

    std::vector<Surfel> surfels;
    surfels.reserve(depthCount);

    for (size_t i = 0; i < depthCount; ++i) {
        float depth = rawFrame->depths[i];
        if (!isnan(depth) && depth > 0.0 && depth < 0.5) { // Hard-coded max depth
            Surfel surfel;
            surfel.position = standard_cyborg::toVector3f(processedFrame.positions[i]);
            surfel.color = standard_cyborg::toVector3f(rawFrame->colors[i]);
            surfel.normal = standard_cyborg::toVector3f(processedFrame.normals[i]);
            surfel.weight = processedFrame.weights[i];
            surfels.push_back(surfel);
        }
    }

    size_t resultSize = surfels.size() * sizeof(Surfel);
    Surfel *result;
    posix_memalign((void **)&result, METAL_REQUIRED_ALIGNMENT, resultSize);
    memcpy(result, surfels.data(), resultSize);

    NSData *surfelsData = [NSData dataWithBytesNoCopy:result length:resultSize freeWhenDone:YES];
    simd_float3 gravity = _gravityEstimator.getGravity();

    SCPointCloud *pointCloud = [[SCPointCloud alloc] initWithSurfelData:surfelsData gravity:gravity];

    const sc3d::PerspectiveCamera& camera = rawFrame->camera;
    
    pointCloud.intrinsicMatrix = toSimdFloat3x3(camera.getIntrinsicMatrix());
    pointCloud.intrinsicMatrixReferenceDimensions = toSimdFloat2(camera.getIntrinsicMatrixReferenceSize());
    pointCloud.depthFrameSize = simd_make_float2(rawFrame->width, rawFrame->height);
    //toSimdFloat2(Vec2(camera.getLegacyImageSize()));
    
    _lastRawFrame = std::move(rawFrame);

    return pointCloud;
}

- (SCPointCloud *)buildPointCloud
{
    const Surfels &surfels = [self surfels];
    NSData *surfelData = [NSData dataWithBytes:surfels.data() length:surfels.size() * sizeof(Surfel)];
    simd_float3 g = _gravityEstimator.getGravity();

    return [[SCPointCloud alloc] initWithSurfelData:surfelData gravity:g];
}

- (BOOL)writePointCloudToPLYFile:(NSString *)plyPath
{
    std::string plyPathString = std::string([plyPath UTF8String]);

    const Surfels &surfels = [self surfels];
    size_t surfelCount = surfels.size();
    simd_float3 gravity = _gravityEstimator.getGravity();

    return PointCloudIO::WriteSurfelsToPLYFile(surfels.data(),
                                               surfelCount,
                                               Vector3f(gravity.x, gravity.y, gravity.z),
                                               plyPathString);
}

- (BOOL)writePointCloudToUSDAFile:(NSString *)USDAPath
{
    NSString *texturePath = [[USDAPath stringByDeletingPathExtension] stringByAppendingPathExtension:@"png"];

    const Surfels &surfels = _pbfModel->getSurfels();
    size_t surfelCount = surfels.size();

    return PointCloudIO::WriteSurfelsToUSDAFile(surfels.data(),
                                                surfelCount,
                                                std::string([USDAPath UTF8String]),
                                                std::string([texturePath UTF8String]));
}

- (PBFFinalStatistics)finalize
{
    PBFFinalStatistics finalStatistics = _pbfModel->finishAssimilating(_surfelFusionConfig);

    _hasFinalized = YES;

    return finalStatistics;
}

- (void)reset
{
    [self _reinitializePBFModel];
    _hasFinalized = NO;
}

// MARK: - Internal

- (void)_ICPIteratedWithResult:(ICPResult)result
{
    dispatch_sync(dispatch_get_global_queue(0, 0), ^{
        if ([_delegate respondsToSelector:@selector(reconstructionManager:didIterateICPWithResult:)]) {
            [_delegate reconstructionManager:self didIterateICPWithResult:result];
        }
    });
}

@end

// MARK: -

@implementation SCOfflineReconstructionManager (Private)

- (void)setDelegate:(id<SCOfflineReconstructionManagerDelegate>)delegate
{
    _delegate = delegate;
}

- (id<SCOfflineReconstructionManagerDelegate>)delegate
{
    return _delegate;
}

- (const std::vector<PBFAssimilatedFrameMetadata>)assimilatedFrameMetadata
{
    return _pbfModel->getAssimilatedFrameMetadata();
}

- (const Surfels &)surfels
{
    return _pbfModel->getSurfels();
}

- (const std::vector<uint32_t> &)surfelIndexMap
{
    return _pbfModel->getSurfelIndexMap();
}

- (id<MTLTexture>)surfelIndexMapTexture
{
    return _surfelIndexMap->getIndexTexture();
}

- (const std::shared_ptr<RawFrame>)lastRawFrame
{
    return _lastRawFrame;
}

- (std::unique_ptr<RawFrame>)readBPLYWithPath:(NSString *)bplyPath
{
    return PointCloudIO::ReadRawFrameFromBPLYFile([bplyPath UTF8String]);
}


- (SCAssimilatedFrameMetadata)accumulateFromRawFrame:(const RawFrame &)rawFrame
{
    NSAssert(!_hasFinalized, @"Told to accumulate another JSON file after having finalized; you must reset after finalizing to accumulate more");

    CFAbsoluteTime startTime = CFAbsoluteTimeGetCurrent();

    ProcessedFrame processedFrame(rawFrame);

    _depthProcessor->computeFrameValues(processedFrame, rawFrame);
    
    auto pbfMetadata = _pbfModel->assimilate(processedFrame, _pbfConfig, _icpConfig, _surfelFusionConfig, rawFrame.timestamp);

    CFAbsoluteTime endTime = CFAbsoluteTimeGetCurrent();

    SCAssimilatedFrameMetadata metadata = SCAssimilatedFrameMetadataFromPBFAssimilatedFrameMetadata(pbfMetadata, 0);
    metadata.assimilationTime = endTime - startTime;
    return metadata;
}

@end
