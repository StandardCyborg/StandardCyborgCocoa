//
//  SCFootTracking.m
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 10/11/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <CoreImage/CoreImage.h>
#import <CoreML/CoreML.h>
#import <Vision/Vision.h>

#import "CVPixelBufferHelpers.h"
#import "MathHelpers.h"
#import "SCFootTracking.h"
#import "SCFootTrackingModel.h"

NS_ASSUME_NONNULL_BEGIN

@implementation SCFootTracking {
    SCFootTrackingModel *_model;
    VNCoreMLModel *_visionModel;
    VNCoreMLRequest *_visionRequest;
    
    dispatch_queue_t _queue;
    BOOL _handlingRequest;
    NSLock *_handlingRequestLock;
    CVPixelBufferRef _pixelBufferToAnalyze;
    CGRect _previousBoundingBox;
}

- (instancetype)initWithModelURL:(NSURL *)modelURL
                        delegate:(id<SCFootTrackingDelegate>)delegate
{
    self = [super init];
    if (self) {
        _smoothing = 0.5;
        _delegate = delegate;
        
        dispatch_queue_attr_t queueAttrs = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL, QOS_CLASS_USER_INITIATED, -1);
        _queue = dispatch_queue_create("SCFootTracking._queue", queueAttrs);
        _handlingRequestLock =  [[NSLock alloc] init];
        _previousBoundingBox = CGRectZero;
        
        NSError *error = nil;
        MLModelConfiguration *modelConfig = [[MLModelConfiguration alloc] init];
        modelConfig.computeUnits = MLComputeUnitsCPUAndGPU;
        
        _model = [[SCFootTrackingModel alloc] initWithContentsOfURL:modelURL configuration:modelConfig error:&error];
        if (_model == nil) {
            NSLog(@"Error instantiating SCFootTrackingModel with model at %@: %@", modelURL, error);
        }
        
        _visionModel = [VNCoreMLModel modelForMLModel:[_model model] error:&error];
        if (_visionModel == nil) {
            NSLog(@"Failed to create VNCoreMLModel: %@", error);
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
        CGRect boundingBox = CGRectZero;
        CGFloat confidence = 0;
        NSError *error;
        VNImageRequestHandler *handler = [[VNImageRequestHandler alloc] initWithCVPixelBuffer:_pixelBufferToAnalyze
                                                                                  orientation:orientation
                                                                                      options:@{}];
        BOOL success = [self _analyzeImageWithRequestHandler:handler
                                                 orientation:orientation
                                                 boundingBox:&boundingBox
                                                  confidence:&confidence
                                                       error:&error];
        
        dispatch_async(dispatch_get_main_queue(), ^{
            if (success && !CGRectIsEmpty(boundingBox)) {
                [_delegate footTracking:self didDetectFootAt:boundingBox confidence:confidence];
            } else {
                _previousBoundingBox = CGRectZero;
                [_delegate footTrackingDidLoseTracking:self];
            }
        });
        
        [self _setHandlingRequest:NO];
    });
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
                            boundingBox:(CGRect *)boundingBoxOut
                             confidence:(CGFloat *)confidenceOut
                                  error:(NSError **)errorOut
{
    BOOL success = [handler performRequests:@[_visionRequest] error:errorOut];
    if (!success) { NSLog(@"Failed to perform Vision request: %@", *errorOut); }
    
    VNRecognizedObjectObservation *bestObservation = [self _highestConfidenceObservation:[_visionRequest results]];
    
    // Apply an exponential moving average to smooth out the results
    CGRect boundingBox = _previousBoundingBox;
    if (bestObservation != nil) {
        CGFloat positionAlpha = 1.0 - _smoothing;
        CGFloat sizeAlpha = 0.5 * positionAlpha;
        boundingBox = CGRectExponentialMovingAverage([bestObservation boundingBox], _previousBoundingBox, positionAlpha, sizeAlpha);
        _previousBoundingBox = boundingBox;
    } else {
        _previousBoundingBox = [bestObservation boundingBox];
    }
    
    CGRect flippedBoundingBox = boundingBox;
    if (orientation == kCGImagePropertyOrientationLeftMirrored || orientation == kCGImagePropertyOrientationRightMirrored) {
        flippedBoundingBox.origin.y = 1.0 - boundingBox.origin.y - boundingBox.size.height; // flip
    } else if (orientation == kCGImagePropertyOrientationUpMirrored) {
        flippedBoundingBox.origin.x = 1.0 - boundingBox.origin.y - boundingBox.size.height; // rotate
        flippedBoundingBox.origin.y = 1.0 - boundingBox.origin.x - boundingBox.size.width; // rotate
        flippedBoundingBox.size.width = boundingBox.size.height;
        flippedBoundingBox.size.height = boundingBox.size.width;
    } else if (orientation == kCGImagePropertyOrientationDownMirrored) {
       flippedBoundingBox.origin.x = 0.0 + boundingBox.origin.y;// - boundingBox.size.height; // rotate
       flippedBoundingBox.origin.y = 0.0 + boundingBox.origin.x;// - boundingBox.size.width; // rotate
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
