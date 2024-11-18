//
//  SCEarTracking.m
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 12/02/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <CoreImage/CoreImage.h>
#import <CoreML/CoreML.h>
#import <Vision/Vision.h>

#import "CVPixelBufferHelpers.h"
#import "MathHelpers.h"
#import "SCEarTracking.h"
#import "SCEarTrackingModel.h"

NS_ASSUME_NONNULL_BEGIN

@implementation SCEarTracking {
    SCEarTrackingModel *_model;
    VNCoreMLModel *_visionModel;
    VNCoreMLRequest *_visionRequest;
    
    dispatch_queue_t _queue;
    BOOL _handlingRequest;
    NSLock *_handlingRequestLock;
    CVPixelBufferRef _pixelBufferToAnalyze;
    CGRect _previousBoundingBox;
}

- (instancetype)initWithModelURL:(NSURL *)modelURL
                        delegate:(id<SCEarTrackingDelegate> _Nullable)delegate
{
    self = [super init];
    if (self) {
        _smoothing = 0.5;
        _delegate = delegate;
        
        dispatch_queue_attr_t queueAttrs = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL, QOS_CLASS_USER_INITIATED, -1);
        _queue = dispatch_queue_create("SCEarTracking._queue", queueAttrs);
        _handlingRequestLock = [[NSLock alloc] init];
        _previousBoundingBox = CGRectZero;
        
        NSError *error = nil;
        MLModelConfiguration *modelConfig = [[MLModelConfiguration alloc] init];
        modelConfig.computeUnits = MLComputeUnitsCPUAndGPU;
        
        _model = [[SCEarTrackingModel alloc] initWithContentsOfURL:modelURL configuration:modelConfig error:&error];
        if (_model == nil) {
            NSLog(@"Error instantiating SCEarTrackingModel with model at %@: %@", modelURL, error);
            return nil;
        }
        
        _visionModel = [VNCoreMLModel modelForMLModel:[_model model] error:&error];
        if (_visionModel == nil) {
            NSLog(@"Failed to create VNCoreMLModel: %@", error);
            return nil;
        }
        
        _visionRequest = [[VNCoreMLRequest alloc] initWithModel:_visionModel];
        _visionRequest.imageCropAndScaleOption = VNImageCropAndScaleOptionScaleFill;
    }
    return self;
}

- (void)dealloc
{
    if (_pixelBufferToAnalyze != NULL) { CVPixelBufferRelease(_pixelBufferToAnalyze); }
}

- (void)analyzePixelBuffer:(CVPixelBufferRef)pixelBuffer orientation:(CGImagePropertyOrientation)orientation
{
    if ([self _setHandlingRequest:YES] == YES) {
        // It's already handling a request, so drop this frame
        return;
    }
    
    // If this pixel buffer is retained for too long, it can hold up incoming buffers,
    // perhaps to keep a finite pool of buffers in memory.
    // So instead, copy the pixel buffer's contents and analyze those instead.
    CVPixelBufferDeepCopy(pixelBuffer, &_pixelBufferToAnalyze);
    
    dispatch_async(_queue, ^{
        CFAbsoluteTime start = CFAbsoluteTimeGetCurrent();
        CGRect boundingBox = CGRectZero;
        float confidence = 0;
        NSError *error;
        VNImageRequestHandler *handler = [[VNImageRequestHandler alloc] initWithCVPixelBuffer:_pixelBufferToAnalyze
                                                                                  orientation:orientation
                                                                                      options:@{}];
        BOOL success = [self _analyzeImageWithRequestHandler:handler
                                                 orientation:orientation
                                              applySmoothing:YES
                                                 boundingBox:&boundingBox
                                                  confidence:&confidence
                                                       error:&error];
        
        CFAbsoluteTime end = CFAbsoluteTimeGetCurrent();
        NSLog(@"SCEarTracking analyzed frame in %.1f ms", 1000.0 * (end - start));
        
        dispatch_async(dispatch_get_main_queue(), ^{
            if (success && !CGRectIsEmpty(boundingBox)) {
                [_delegate earTracking:self didDetectEarAt:boundingBox confidence:confidence];
            } else {
                _previousBoundingBox = CGRectZero;
                [_delegate earTrackingDidLoseTracking:self];
            }
        });
        
        [self _setHandlingRequest:NO];
    });
}

- (BOOL)synchronousAnalyzeImage:(CIImage *)image
                    orientation:(CGImagePropertyOrientation)orientation
                 boundingBoxOut:(CGRect *)boundingBoxOut
                  confidenceOut:(float *)confidenceOut
                          error:(NSError **)errorOut
{
    NSError *error;
    VNImageRequestHandler *handler = [[VNImageRequestHandler alloc] initWithCIImage:image
                                                                        orientation:orientation
                                                                            options:@{}];
    return [self _analyzeImageWithRequestHandler:handler
                                     orientation:orientation
                                  applySmoothing:NO
                                     boundingBox:boundingBoxOut
                                      confidence:confidenceOut
                                           error:&error];
}

// MARK: - Private

- (BOOL)_setHandlingRequest:(BOOL)newValue
{
    [_handlingRequestLock lock];
    BOOL currentlyHandling = _handlingRequest;
    _handlingRequest = newValue;
    [_handlingRequestLock unlock];
    
    return currentlyHandling;
}

- (BOOL)_analyzeImageWithRequestHandler:(VNImageRequestHandler *)handler
                            orientation:(CGImagePropertyOrientation)orientation
                         applySmoothing:(BOOL)applySmoothing
                            boundingBox:(CGRect *)boundingBoxOut
                             confidence:(float *)confidenceOut
                                  error:(NSError **)errorOut
{
    BOOL success = [handler performRequests:@[_visionRequest] error:errorOut];
    if (!success) { NSLog(@"Failed to perform Vision request: %@", *errorOut); }
    
    VNRecognizedObjectObservation *bestObservation = [self _highestConfidenceObservation:[_visionRequest results]];
    
    // Apply an exponential moving average to smooth out the results
    CGRect boundingBox = _previousBoundingBox;
    
    if (bestObservation != nil) {
        if (applySmoothing) {
            CGFloat positionAlpha = 1.0 - _smoothing;
            CGFloat sizeAlpha = 0.5 * positionAlpha;
            boundingBox = CGRectExponentialMovingAverage([bestObservation boundingBox], _previousBoundingBox, positionAlpha, sizeAlpha);
        } else {
            boundingBox = [bestObservation boundingBox];
        }
        
        _previousBoundingBox = boundingBox;
    }
    
    CGRect flippedBoundingBox = boundingBox;
    if (orientation == kCGImagePropertyOrientationLeftMirrored || orientation == kCGImagePropertyOrientationRightMirrored) {
        flippedBoundingBox.origin.y = 1.0 - boundingBox.origin.y - boundingBox.size.height;
    } else if (orientation == kCGImagePropertyOrientationUpMirrored) {
        flippedBoundingBox.origin.x = 1.0 - boundingBox.origin.y - boundingBox.size.height;
        flippedBoundingBox.origin.y = 1.0 - boundingBox.origin.x - boundingBox.size.width;
        flippedBoundingBox.size.width = boundingBox.size.height;
        flippedBoundingBox.size.height = boundingBox.size.width;
    }
    
    *boundingBoxOut = flippedBoundingBox;
    *confidenceOut = bestObservation.confidence;
    return success;
}

- (VNRecognizedObjectObservation * _Nullable)_highestConfidenceObservation:(NSArray *)observations
{
    VNRecognizedObjectObservation *result = nil;
    
    for (VNRecognizedObjectObservation *observation in observations) {
        if ([observation confidence] > [result confidence]) {
            result = observation;
        }
    }
    
    return result;
}

@end

NS_ASSUME_NONNULL_END
