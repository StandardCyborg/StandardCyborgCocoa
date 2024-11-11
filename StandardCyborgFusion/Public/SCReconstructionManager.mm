//
//  SCReconstructionManager.mm
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 7/5/18.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//


#import <Foundation/Foundation.h>
#if !TARGET_OS_OSX

#import <AVFoundation/AVFoundation.h>
#import <CoreMotion/CoreMotion.h>
#import <StandardCyborgFusion/CVPixelBufferHelpers.h>
#import <StandardCyborgFusion/GeometryHelpers.hpp>
#import <StandardCyborgFusion/MathHelpers.h>
#import <StandardCyborgFusion/MetalDepthProcessor.hpp>
#import <StandardCyborgFusion/MetalSurfelIndexMap.hpp>
#import <StandardCyborgFusion/PBFModel.hpp>
#import <StandardCyborgFusion/PerspectiveCamera+AVFoundation.hpp>
#import <StandardCyborgFusion/PointCloudIO.hpp>
#import <StandardCyborgFusion/SCAssimilatedFrameMetadata_Private.h>
#import <StandardCyborgFusion/SCPointCloud.h>
#import <StandardCyborgFusion/SCPointCloud_Private.h>
#import <StandardCyborgFusion/SurfelFusion.hpp>
#import <standard_cyborg/util/DataUtils.hpp>
#import <StandardCyborgFusion/SCReconstructionManagerParameters_Private.h>
#import <StandardCyborgFusion/SCReconstructionManager_Private.h>

#import <iostream>
#import <objc/runtime.h>

#import "GravityEstimator.hpp"
#import "SCReconstructionManager.h"

using namespace standard_cyborg;

NS_ASSUME_NONNULL_BEGIN

@interface _IncomingFrameData : NSObject
@property (nonatomic, readonly) int sequence;
@property (nonatomic, readonly) CVPixelBufferRef depthBuffer;
@property (nonatomic, readonly) CVPixelBufferRef colorBuffer;
@property (nonatomic, readonly) AVCameraCalibrationData *calibrationData;

- (instancetype)initWithSequence:(int)sequence
                     depthBuffer:(CVPixelBufferRef)depthBuffer
                     colorBuffer:(CVPixelBufferRef)colorBuffer
                 calibrationData:(AVCameraCalibrationData *)calibrationData;

@end

@implementation _IncomingFrameData

- (instancetype)initWithSequence:(int)sequence
                     depthBuffer:(CVPixelBufferRef)depthBuffer
                     colorBuffer:(CVPixelBufferRef)colorBuffer
                 calibrationData:(AVCameraCalibrationData *)calibrationData
{
    self = [super init];
    if (self) {
        NSParameterAssert(depthBuffer);
        NSParameterAssert(colorBuffer);
        NSParameterAssert(calibrationData);
        _sequence = sequence;
        _depthBuffer = CVPixelBufferRetain(depthBuffer);
        _colorBuffer = CVPixelBufferRetain(colorBuffer);
        _calibrationData = calibrationData;
    }
    return self;
}

- (void)dealloc
{
    CVPixelBufferRelease(_depthBuffer);
    CVPixelBufferRelease(_colorBuffer);
}

@end

@interface _WeakObjectDeathNotifier : NSObject
@property (nonatomic) dispatch_block_t handler;
@end
@implementation _WeakObjectDeathNotifier
- (void)dealloc
{
    if (_handler != nil) { _handler(); }
}
@end

// MARK: -

@implementation SCReconstructionManager {
    PBFConfiguration _pbfConfig;
    ICPConfiguration _icpConfig;
    SurfelFusionConfiguration _surfelFusionConfig;
    
    dispatch_queue_t _inputQueue;
    dispatch_semaphore_t _incomingFrameDataSemaphore;
    _IncomingFrameData *_inputQueue_incomingFrameData;
    int _inputQueue_incomingFrameSequence;
    SCReconstructionManagerStatistics _inputQueue_statistics;
    BOOL _inputQueue_stopped;
    BOOL _inputQueue_shuttingDown;
    
    dispatch_queue_t _modelQueue;
    PBFModel *_modelQueue_model;
    MetalDepthProcessor *_modelQueue_depthProcessor;
    ProcessedFrame *_modelQueue_frame;
    float _modelQueue_maxDepth;
    BOOL _userSetMaxDepth;
    BOOL _modelQueue_hasCalculatedModelConfig;
    BOOL _finalized;
    BOOL _wroteIntrinsicsToFile;
    
    GravityEstimator _gravityEstimator;
}

- (instancetype)initWithDevice:(id<MTLDevice>)device
                  commandQueue:(id<MTLCommandQueue>)commandQueue
                maxThreadCount:(int)maxThreadCount
{
    self = [super init];
    if (self) {
        _wroteIntrinsicsToFile = NO;
        _inputQueue_stopped = YES;

        _modelQueue_maxDepth = _surfelFusionConfig.maxDepth;
        _userSetMaxDepth = NO;
        
        std::shared_ptr<SurfelIndexMap> surfelIndexMap(new MetalSurfelIndexMap(device, commandQueue));
        _modelQueue_model = new PBFModel(surfelIndexMap);
        
        _icpConfig.maxIterations = (int)[[NSUserDefaults standardUserDefaults] integerForKey:@"icp_max_iteration_count"] ?: _icpConfig.maxIterations;
        _icpConfig.tolerance = [[NSUserDefaults standardUserDefaults] floatForKey:@"icp_tolerance"] ?: _icpConfig.tolerance;
        
        _modelQueue_depthProcessor = new MetalDepthProcessor(device, commandQueue);
        
        _modelQueue = dispatch_queue_create("SCReconstructionManager._modelQueue",
                                            dispatch_queue_attr_make_with_qos_class(NULL, QOS_CLASS_USER_INTERACTIVE, 0));
        _inputQueue = dispatch_queue_create("SCReconstructionManager._inputQueue",
                                            dispatch_queue_attr_make_with_qos_class(NULL, QOS_CLASS_USER_INTERACTIVE, 0));
        _incomingFrameDataSemaphore = dispatch_semaphore_create(0);
        
        _normalizedFrameClipRegion = CGRectZero;
        
        __weak SCReconstructionManager *weakSelf = self;
        dispatch_async(_modelQueue, ^{
            [weakSelf _modelQueueMain];
        });
    }
    return self;
}

- (void)dealloc
{
    dispatch_sync(_inputQueue, ^{
        _inputQueue_stopped = YES;
        _inputQueue_incomingFrameData = nil;
        _inputQueue_shuttingDown = YES;
    });
    dispatch_semaphore_signal(_incomingFrameDataSemaphore);
    
    delete _modelQueue_frame;
    delete _modelQueue_depthProcessor;
    delete _modelQueue_model;
}

// MARK: - SCReconstructionManagerParameters

- (float)icpDownsampleFraction
{
    return _pbfConfig.icpDownsampleFraction;
}

- (void)setICPDownsampleFraction:(float)fraction
{
    NSParameterAssert(fraction >= 0 && fraction <= 1);
    _pbfConfig.icpDownsampleFraction = fraction;
}

- (int)maxThreadCount
{
    return _icpConfig.threadCount;
}

- (void)setMaxThreadCount:(int)count
{
    NSParameterAssert(count >= 1);
    _icpConfig.threadCount = count;
}

- (float)icpOutlierDeviationsThreshold
{
    return _icpConfig.outlierDeviationsThreshold;
}

- (void)setICPOutlierDeviationsThreshold:(float)threshold
{
    NSParameterAssert(threshold >= 0);
    _icpConfig.outlierDeviationsThreshold = threshold;
}

- (float)icpTolerance
{
    return _icpConfig.tolerance;
}

- (void)setICPTolerance:(float)fraction
{
    NSParameterAssert(fraction >= 0);
    _icpConfig.tolerance = fraction;
}

- (int)maxICPIterations
{
    return _icpConfig.maxIterations;
}

- (void)setMaxICPIterations:(int)maxIterations
{
    NSParameterAssert(maxIterations >= 1);
    _icpConfig.maxIterations = maxIterations;
}

- (float)minDepth
{
    return _surfelFusionConfig.minDepth;
}

- (void)setMinDepth:(float)minDepth
{
    NSParameterAssert(minDepth >= 0);
    _surfelFusionConfig.minDepth = minDepth;
}

- (float)maxDepth
{
    return _surfelFusionConfig.maxDepth;
}

- (void)setMaxDepth:(float)maxDepth
{
    NSParameterAssert(maxDepth >= 0);
    _surfelFusionConfig.maxDepth = maxDepth;
    _userSetMaxDepth = YES;
}
- (void)clearMaxDepth
{
    _userSetMaxDepth = NO;
}

// MARK: -

- (void)setDelegate:(_Nullable id<SCReconstructionManagerDelegate>)delegate
{
    _delegate = delegate;
    
    if (_delegate != nil) {
        // When the delegate is deallocated, we don't get notified via setDelegate that it became nil,
        // but we can track that using this notifier.
        // When that occurs, signal the semaphore so we can spin our main loop and clean up if needed
        dispatch_semaphore_t incomingFrameDataSemaphore = _incomingFrameDataSemaphore;
        _WeakObjectDeathNotifier* notifier = [[_WeakObjectDeathNotifier alloc] init];
        notifier.handler = ^{
            dispatch_semaphore_signal(incomingFrameDataSemaphore);
        };
        objc_setAssociatedObject(delegate, "SCReconstructionManager.delegate", notifier, OBJC_ASSOCIATION_RETAIN);
    }
}

- (SCReconstructionManagerStatistics)currentStatistics
{
    __block SCReconstructionManagerStatistics stats;
    dispatch_sync(_inputQueue, ^{
        stats = _inputQueue_statistics;
    });
    return stats;
}

- (simd_float3)gravity
{
    return _gravityEstimator.getGravity();
}

- (SCPointCloud *)reconstructSingleDepthBuffer:(CVPixelBufferRef)depthBuffer
                                   colorBuffer:(_Nullable CVPixelBufferRef)colorBuffer
                           withCalibrationData:(AVCameraCalibrationData *)calibrationData
                               smoothingPoints:(BOOL)smoothPoints
{
    size_t width = CVPixelBufferGetWidth(depthBuffer);
    size_t height = CVPixelBufferGetHeight(depthBuffer);
    size_t depthCount = width * height;
    
    _latestCameraCalibrationData = calibrationData;
    _latestCameraCalibrationFrameWidth = (int)width;
    _latestCameraCalibrationFrameHeight = (int)height;
    
    sc3d::PerspectiveCamera camera = PerspectiveCameraFromAVCameraCalibrationData(calibrationData, width, height);
    RawFrame rawFrame(camera, width, height);
    
    if (colorBuffer == NULL) {
        [[self class] _fillFloatVector:rawFrame.depths
                       fromPixelBuffer:depthBuffer
                     replacingNaNsWith:9999
                flipsInputHorizontally:_flipsInputHorizontally];
    } else {
        [[self class] _fillDepthVector:rawFrame.depths
                           colorMatrix:rawFrame.colors
                       fromDepthBuffer:depthBuffer
                           colorBuffer:colorBuffer
                     replacingNaNsWith:9999
                flipsInputHorizontally:_flipsInputHorizontally
                  normalizedClipRegion:_normalizedFrameClipRegion];
    }
    
    ProcessedFrame frame(camera, width, height);
    
    float maxDepth;
    if (_userSetMaxDepth == YES) {
        // respect users maxDepth
        maxDepth = _surfelFusionConfig.maxDepth;
    } else {
        // center weighting strategy
        maxDepth = kCenterDepthExpansionRatio * CVPixelBufferAverageDepthAroundCenter(depthBuffer);
    }
    
    _modelQueue_depthProcessor->computeFrameValues(frame, rawFrame, smoothPoints);
    
    // This is the maximum size it can possibly be, but it will likely be less than this
    size_t resultSize = depthCount * sizeof(Surfel);
    Surfel *result;
    const int kMetalRequiredAlignment = 4096;
    posix_memalign((void **)&result, kMetalRequiredAlignment, roundUpToMultiple(resultSize, kMetalRequiredAlignment));
    size_t surfelCount = 0;
    
    for (size_t i = 0; i < depthCount; ++i) {
        float depth = rawFrame.depths[i];
        if (!isnan(depth) && depth < maxDepth) {
            Surfel surfel;
            surfel.position = toVector3f(frame.positions[i]);
            surfel.color = toVector3f(rawFrame.colors[i]);
            surfel.normal = toVector3f(frame.normals[i]);
            surfel.weight = 20;
            result[surfelCount] = surfel;
            ++surfelCount;
        }
    }
    
    NSData *surfelsData = [NSData dataWithBytesNoCopy:result length:surfelCount * sizeof(Surfel) freeWhenDone:YES];
    simd_float3 g = [self gravity];
    
    SCPointCloud *pointCloud = [[SCPointCloud alloc] initWithSurfelData:surfelsData gravity:g];
    pointCloud.intrinsicMatrix = toSimdFloat3x3(rawFrame.camera.getIntrinsicMatrix());
    pointCloud.intrinsicMatrixReferenceDimensions = toSimdFloat2(rawFrame.camera.getIntrinsicMatrixReferenceSize());

    pointCloud.depthFrameSize = toSimdFloat2(math::Vec2(rawFrame.width, rawFrame.height));

    return pointCloud;
}

- (void)accumulateDepthBuffer:(CVPixelBufferRef)depthBuffer
                  colorBuffer:(CVPixelBufferRef)colorBuffer
              calibrationData:(AVCameraCalibrationData *)calibrationData
{
    if (depthBuffer == NULL || colorBuffer == NULL || calibrationData == nil) { return; }
    CVPixelBufferRetain(depthBuffer);
    CVPixelBufferRetain(colorBuffer);
    
    dispatch_async(_inputQueue, ^{
        _inputQueue_stopped = NO;
        
        int sequence = _inputQueue_incomingFrameSequence++;
        _IncomingFrameData *data = [[_IncomingFrameData alloc] initWithSequence:sequence
                                                                    depthBuffer:depthBuffer
                                                                    colorBuffer:colorBuffer
                                                                calibrationData:calibrationData];
        CVPixelBufferRelease(depthBuffer);
        CVPixelBufferRelease(colorBuffer);
        
        // We only use the most recent raw frame, dropping any other ones that haven't had a chance to process
        BOOL dropped = _inputQueue_incomingFrameData != nil;
        _inputQueue_incomingFrameData = data;
        
        if (!dropped) {
            dispatch_semaphore_signal(_incomingFrameDataSemaphore);
        }
    });
}

- (void)accumulateDeviceMotion:(CMDeviceMotion *)deviceMotion
{
    simd_float3 gravitySample = simd_make_float3(deviceMotion.gravity.x,
                                                 deviceMotion.gravity.y,
                                                 deviceMotion.gravity.z);
    
    simd_float4 attitudeSample = simd_make_float4(deviceMotion.attitude.quaternion.x,
                                                  deviceMotion.attitude.quaternion.y,
                                                  deviceMotion.attitude.quaternion.z,
                                                  deviceMotion.attitude.quaternion.w);
    
    _gravityEstimator.accumulate(gravitySample, attitudeSample);
}

- (void)finalize:(dispatch_block_t)completion
{
    _finalized = YES;
    
    dispatch_async(_inputQueue, ^{
        _inputQueue_stopped = YES;
        
        BOOL dropped = _inputQueue_incomingFrameData != nil;
        _inputQueue_incomingFrameData = nil;
        
        if (!dropped) {
            dispatch_semaphore_signal(_incomingFrameDataSemaphore);
        }
        
        dispatch_async(_modelQueue, ^{
            _modelQueue_model->finishAssimilating(_surfelFusionConfig);
            dispatch_async(dispatch_get_main_queue(), completion);
        });
    });
}

- (SCPointCloud *)buildPointCloud
{
    // TODO: Fix threading/queuing and run off of a copy in PBFModel
    const Surfels& surfels = _modelQueue_model->getSurfels();
    NSData *surfelData;
    if (_finalized) {
        surfelData = [NSData dataWithBytes:(void *)surfels.data() length:surfels.size() * sizeof(Surfel)];
    } else {
        // While scanning, for the sake of performance, avoid copying the whole surfels data structure and return it directly
        // This is an obvious thread safety issue, but we tend to get away with it 99+% of the time, so it's acceptable for now
        surfelData = [NSData dataWithBytesNoCopy:(void *)surfels.data() length:surfels.size() * sizeof(Surfel) freeWhenDone:NO];
    }
    
    simd_float3 gravity = [self gravity];
    
    return [[SCPointCloud alloc] initWithSurfelData:surfelData gravity:gravity];
}

- (void)reset
{
    _finalized = NO;
    
    dispatch_sync(_inputQueue, ^{
        _inputQueue_incomingFrameData = nil;
        _inputQueue_incomingFrameSequence = 0;
        _inputQueue_statistics = SCReconstructionManagerStatistics();
        _inputQueue_stopped = YES;
        
        dispatch_async(_modelQueue, ^{
            _modelQueue_hasCalculatedModelConfig = NO;
            _modelQueue_model->reset();
        });
        
        // Allows _modelQueue to run a loop
        dispatch_semaphore_signal(_incomingFrameDataSemaphore);
    });
}

// MARK: - Internal

static const float kCenterDepthExpansionRatio = 1.4;

+ (NSString *)_loadAPIKeyFromInfoPlist
{
    NSDictionary *infoDictionary = [[NSBundle mainBundle] infoDictionary];
    
    return infoDictionary[@"SC_API_KEY"];
}

- (void)_writeIntrinsics:(sc3d::PerspectiveCamera&)camera toFile:(NSString *)filename
{
    JSON metadata;
    metadata["camera_intrinsics"] = PointCloudIO::JSONFromPerspectiveCamera(camera);
    NSString *metadataString = [NSString stringWithUTF8String:metadata.dump().c_str()];
    
    [metadataString writeToFile:filename atomically:YES encoding:NSUTF8StringEncoding error:nil];
}

- (void)_modelQueueMain
{
    __block BOOL shuttingDown, stopped;
    __block _IncomingFrameData *incomingFrameData;
    __block SCReconstructionManagerStatistics statistics;
    
    while (YES) {
        dispatch_semaphore_wait(_incomingFrameDataSemaphore, DISPATCH_TIME_FOREVER);
        
        dispatch_sync(_inputQueue, ^{
            shuttingDown = _inputQueue_shuttingDown;
            stopped = _inputQueue_stopped;
            incomingFrameData = _inputQueue_incomingFrameData;
            _inputQueue_incomingFrameData = nil;
        });
        
        if (shuttingDown || stopped || incomingFrameData == nil) { break; }
        
        // Assimilate!
        PBFAssimilatedFrameMetadata pbfMetadata = [self _modelQueue_assimilateIncomingFrameData:incomingFrameData];
        
        SCAssimilatedFrameMetadata metadata = SCAssimilatedFrameMetadataFromPBFAssimilatedFrameMetadata(pbfMetadata,
                                                                                                        _inputQueue_statistics.consecutiveLostTrackingCount);
        
        if (_includesDepthBuffersInMetadata) {
            metadata.depthBuffer = CVPixelBufferRetain(incomingFrameData.depthBuffer);
        }
        
        if (_includesColorBuffersInMetadata) {
            metadata.colorBuffer = CVPixelBufferRetain(incomingFrameData.colorBuffer);
        }
        
        dispatch_sync(_inputQueue, ^{
            switch (metadata.result) {
                case SCAssimilatedFrameResultSucceeded:
                case SCAssimilatedFrameResultPoorTracking:
                    _inputQueue_statistics.succeededCount += 1;
                    _inputQueue_statistics.consecutiveLostTrackingCount = 0;
                    break;
                    
                case SCAssimilatedFrameResultLostTracking:
                    _inputQueue_statistics.lostTrackingCount += 1;
                    _inputQueue_statistics.consecutiveLostTrackingCount += 1;
                    // Intentional fall-through
                    
                case SCAssimilatedFrameResultFailed:
                    _inputQueue_stopped = YES;
                    _inputQueue_incomingFrameData = nil;
                    break;
            }
            
            statistics = _inputQueue_statistics;
        });
        
        dispatch_async(dispatch_get_main_queue(), ^{
#ifndef XCODE_ACTION_install // Avoid logging in archive builds
            printf("STATS: succeeded: %zd, lost tracking: %zd, consecutive lost tracking: %zd\n",
                   statistics.succeededCount, statistics.lostTrackingCount, statistics.consecutiveLostTrackingCount);
#endif
            
            [_delegate reconstructionManager:self didProcessWithMetadata:metadata statistics:statistics];
            
            if (metadata.depthBuffer != NULL) {
                CVPixelBufferRelease(metadata.depthBuffer);
            }
            
            if (metadata.colorBuffer != NULL) {
                CVPixelBufferRelease(metadata.colorBuffer);
            }
        });
    }
    
    if (!shuttingDown) {
        __weak SCReconstructionManager *weakSelf = self;
        dispatch_async(_modelQueue, ^{
            [weakSelf _modelQueueMain];
        });
    }
}

/** @return float The assimilation quality as reported by PBFModel */
- (PBFAssimilatedFrameMetadata)_modelQueue_assimilateIncomingFrameData:(_IncomingFrameData *)data
{
    CFAbsoluteTime startTime = CFAbsoluteTimeGetCurrent();
    
    [self _modelQueue_fillRawFrameWithData:data];
    
    [self _modelQueue_unprojectRawFrameIntoFrame];
    
    [self _modelQueue_configureModelForRawFrame];
    
    auto metadata = _modelQueue_model->assimilate(*_modelQueue_frame, _pbfConfig, _icpConfig, _surfelFusionConfig, startTime);
    
#ifndef XCODE_ACTION_install // Avoid logging in archive builds
    float quality = metadata.icpUnusedIterationFraction;
    
    CFAbsoluteTime endTime = CFAbsoluteTimeGetCurrent();
    printf("Assimilated frame %d in %.2f ms with quality %f\n", data.sequence, 1000.0 * (endTime - startTime), quality);
#endif
    
    return metadata;
}

- (void)_modelQueue_fillRawFrameWithData:(_IncomingFrameData *)data
{
    const size_t depthWidth = CVPixelBufferGetWidth(data.depthBuffer);
    const size_t depthHeight = CVPixelBufferGetHeight(data.depthBuffer);
    
    // Build up the raw frame
    sc3d::PerspectiveCamera camera = PerspectiveCameraFromAVCameraCalibrationData(data.calibrationData, depthWidth, depthHeight);
    
    if (!_wroteIntrinsicsToFile) {
        [self _writeIntrinsics:camera toFile:[NSTemporaryDirectory() stringByAppendingPathComponent:@"intrinsics.json"]];
        _wroteIntrinsicsToFile = YES;
    }
    
    if (_modelQueue_frame == nullptr ||
        _modelQueue_frame->rawFrame.width != depthWidth ||
        _modelQueue_frame->rawFrame.height != depthHeight)
    {
        if (_modelQueue_frame != nullptr) { delete _modelQueue_frame; }
        
        _modelQueue_frame = new ProcessedFrame(camera, depthWidth, depthHeight);
    }
    
    [[self class] _fillDepthVector:_modelQueue_frame->rawFrame.depths
                       colorMatrix:_modelQueue_frame->rawFrame.colors
                   fromDepthBuffer:data.depthBuffer
                       colorBuffer:data.colorBuffer
                 replacingNaNsWith:-1
            flipsInputHorizontally:_flipsInputHorizontally
              normalizedClipRegion:_normalizedFrameClipRegion];
}

- (void)_modelQueue_unprojectRawFrameIntoFrame
{
    // Un-project the raw frame into a ProcessedFrame
    assert(_modelQueue_frame != nullptr);
    
    _modelQueue_depthProcessor->computeFrameValues(*_modelQueue_frame, _modelQueue_frame->rawFrame);
}

- (void)_modelQueue_configureModelForRawFrame
{
    if (_modelQueue_hasCalculatedModelConfig) { return; }
    
    float averageDepthAtCenter = AverageDepthFromValues(_modelQueue_frame->rawFrame.depths.data(),
                                                        _modelQueue_frame->rawFrame.width,
                                                        _modelQueue_frame->rawFrame.height);
    
    
    _surfelFusionConfig.minDepth = 0;
    if (!isnan(averageDepthAtCenter) && _userSetMaxDepth == NO) {
        _surfelFusionConfig.maxDepth = averageDepthAtCenter * kCenterDepthExpansionRatio;
    }
    
    size_t frameWidth = _modelQueue_frame->rawFrame.width;
    size_t frameHeight = _modelQueue_frame->rawFrame.height;
    _pbfConfig.icpDownsampleFraction = 0.05 * 640.0 / (float)frameWidth * 360.0 / (float)frameHeight;
    
    _modelQueue_maxDepth = _surfelFusionConfig.maxDepth;
    _modelQueue_hasCalculatedModelConfig = YES;
    printf("Average depth at center was %f; set max depth to %f\n", averageDepthAtCenter, _surfelFusionConfig.maxDepth);
}

+ (void)_fillFloatVector:(std::vector<float>&)vectorOut
         fromPixelBuffer:(CVPixelBufferRef)buffer
       replacingNaNsWith:(float)nanReplacement
  flipsInputHorizontally:(BOOL)flipsInputHorizontally
{
    CVPixelBufferLockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
    
    const size_t width = CVPixelBufferGetWidth(buffer);
    const size_t height = CVPixelBufferGetHeight(buffer);
    const size_t pixelCount = width * height;
    const float *bufferValues = (const float *)CVPixelBufferGetBaseAddress(buffer);
    
    assert(pixelCount == vectorOut.size());
    vectorOut.resize(pixelCount);
    
    for (size_t i = 0; i < pixelCount; ++i) {
        size_t y = i / width;
        size_t x = i % width;
        
        float depth;
        
        if (flipsInputHorizontally) {
            depth = bufferValues[(height - 1 - y) * width + x];
        } else {
            depth = bufferValues[i];
        }
        
        vectorOut[i] = isnan(depth) ? nanReplacement : depth;
    }
    
    CVPixelBufferUnlockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
}

+ (void)_fillDepthVector:(std::vector<float>&)depthVectorOut
             colorMatrix:(std::vector<math::Vec3>&)colorMatrixOut
         fromDepthBuffer:(CVPixelBufferRef)depthBuffer
             colorBuffer:(CVPixelBufferRef)colorBuffer
       replacingNaNsWith:(float)nanReplacement
  flipsInputHorizontally:(BOOL)flipsInputHorizontally
    normalizedClipRegion:(CGRect)normalizedClipRegion
{
    CVPixelBufferLockBaseAddress(depthBuffer, kCVPixelBufferLock_ReadOnly);
    CVPixelBufferLockBaseAddress(colorBuffer, kCVPixelBufferLock_ReadOnly);
    
    const size_t depthWidth = CVPixelBufferGetWidth(depthBuffer);
    const size_t depthHeight = CVPixelBufferGetHeight(depthBuffer);
    const size_t depthPixelCount = depthWidth * depthHeight;
    const float *depthBufferValues = (const float *)CVPixelBufferGetBaseAddress(depthBuffer);
    
    const size_t colorWidth = CVPixelBufferGetWidth(colorBuffer);
    const size_t colorHeight = CVPixelBufferGetHeight(colorBuffer);
    const uint8_t *colorBufferValues = (const uint8_t *)CVPixelBufferGetBaseAddress(colorBuffer);
    
    const simd_float2 depthToColorRatio = simd_make_float2((float)colorWidth / (float)depthWidth,
                                                           (float)colorHeight / (float)depthHeight);
    const size_t rgbBytesPerRow = CVPixelBufferGetBytesPerRow(colorBuffer);
    const size_t rgbBytesPerPixel = 4;
    const float rgbNormalize = 1.0 / 255.0;
    
    assert(depthPixelCount == depthVectorOut.size());
    assert(depthPixelCount == colorMatrixOut.size());
    depthVectorOut.resize(depthPixelCount);
    colorMatrixOut.resize(depthPixelCount);
    
    CGRect frameClipRegion = CGRectMake(normalizedClipRegion.origin.x * depthWidth,
                                        normalizedClipRegion.origin.y * depthHeight,
                                        normalizedClipRegion.size.width * depthWidth,
                                        normalizedClipRegion.size.height * depthHeight);
    
    size_t depthIndex = 0;
    for (size_t y = 0; y < depthHeight; ++y)
    {
        for (size_t x = 0; x < depthWidth; ++x)
        {
            size_t rgbX = (size_t)(x * depthToColorRatio.x);
            
            size_t rgbY;
            if (flipsInputHorizontally) {
                rgbY = (size_t)((depthHeight - 1 - y) * depthToColorRatio.y);
            } else {
                rgbY = (size_t)(y * depthToColorRatio.y);
            }
            
            
            size_t rgbIndex = (rgbY * rgbBytesPerRow + rgbX * rgbBytesPerPixel);
            
            float depth;
            if (flipsInputHorizontally) {
                depth = depthBufferValues[(depthHeight - 1 - y) * depthWidth + x];
            } else {
                depth = depthBufferValues[y * depthWidth + x];
            }
            
            if (normalizedClipRegion.size.width > 0 && normalizedClipRegion.size.height > 0) {
                if (!CGRectContainsPoint(frameClipRegion, CGPointMake(x, y))) {
                    depth = NAN;
                }
            }
            
            uint8_t b = colorBufferValues[rgbIndex + 0];
            uint8_t g = colorBufferValues[rgbIndex + 1];
            uint8_t r = colorBufferValues[rgbIndex + 2];
            math::Vec3 normalizedRGB(fastApplyGammaCorrection(r * rgbNormalize),
                                               fastApplyGammaCorrection(g * rgbNormalize),
                                               fastApplyGammaCorrection(b * rgbNormalize));
            
            depthVectorOut[depthIndex] = isnan(depth) ? nanReplacement : depth;
            colorMatrixOut[depthIndex] = normalizedRGB;
            
            ++depthIndex;
        }
    }
    
    CVPixelBufferUnlockBaseAddress(depthBuffer, kCVPixelBufferLock_ReadOnly);
    CVPixelBufferUnlockBaseAddress(colorBuffer, kCVPixelBufferLock_ReadOnly);
}

@end

NS_ASSUME_NONNULL_END

#endif // !TARGET_OS_OSX
