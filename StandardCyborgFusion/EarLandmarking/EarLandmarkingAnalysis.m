//
//  EarLandmarkingAnalysis.m
//  EarLandmarking
//
//  Created by Aaron Thompson on 5/23/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <CoreImage/CoreImage.h>
#import <CoreServices/CoreServices.h>

#import "EarLandmarkingAnalysis.h"
#import "SCEarLandmarking.h"
#import "SCLandmark2D.h"
#import "SCLandmark2D_Private.h"

NS_ASSUME_NONNULL_BEGIN

@implementation EarLandmarkingAnalysis {
    SCEarLandmarking *_model;
}

- (instancetype)initWithModelAtURL:(NSURL *)modelURL
{
    self = [super init];
    if (self) {
        MLModelConfiguration *config = [[MLModelConfiguration alloc] init];
        config.computeUnits = MLComputeUnitsAll;
        
        NSError *error = nil;
        _model = [[SCEarLandmarking alloc] initWithContentsOfURL:modelURL configuration:config error:&error];
        
        if (_model == nil) {
            NSLog(@"Error instantiating model at %@: %@", modelURL, error);
        }
        NSAssert(_model != nil, @"Failed to instantiate face landmarking model at URL %@", modelURL);
    }
    return self;
}

+ (CIImage *)_cropAndScaleImage:(CIImage *)inputImage
             normalizedCropRect:(CGRect)normalizedCropRect
                     scaledSize:(CGSize)scaledSize
{
    CGFloat inputWidth = inputImage.extent.size.width, inputHeight = inputImage.extent.size.height;
    CGRect cropRect = CGRectMake(normalizedCropRect.origin.x * inputWidth,
                                 normalizedCropRect.origin.y * inputHeight,
                                 normalizedCropRect.size.width * inputWidth,
                                 normalizedCropRect.size.height * inputHeight);
    
    CIImage *image = [inputImage imageByCroppingToRect:cropRect];
    
    CGFloat scaleX = scaledSize.width / CGRectGetWidth(image.extent);
    CGFloat scaleY = scaledSize.height / CGRectGetHeight(image.extent);
    
    // When scaling the image, CI will use 0 for values at the edge of the scan
    // Fix this by clamping to the outer values, then scaling, then re-cropping to the intended size
    image = [image imageByClampingToRect:cropRect];
    image = [image imageByApplyingTransform:CGAffineTransformMakeScale(scaleX, scaleY)];
    image = [image imageByCroppingToRect:CGRectMake(cropRect.origin.x * scaleX, cropRect.origin.y * scaleY, scaledSize.width, scaledSize.height)];
    
    // Due to the way [CIContext:render:toBitmap::::] works, we need to translate the image so the cropped section is at the origin
    image = [image imageByApplyingTransform:CGAffineTransformMakeTranslation(-image.extent.origin.x, -image.extent.origin.y)];
    
    return image;
}

/// @param normalizedRect origin is top-left
+ (void)_drawImage:(CIImage *)ciImage
         landmarks:(NSArray<SCLandmark2D *> *)landmarks
    normalizedRect:(CGRect)normalizedRect
            toFile:(NSString *)path
{
    // Convert the image to sRGB
    CGColorSpaceRef sRGBColorSpace = CGColorSpaceCreateDeviceRGB();
    ciImage = [ciImage imageByColorMatchingColorSpaceToWorkingSpace:sRGBColorSpace];
    
    const int width = (int)ciImage.extent.size.width;
    const int height = (int)ciImage.extent.size.height;
    
    CGContextRef cgContext = CGBitmapContextCreate(NULL, // data
                                                   width,
                                                   height,
                                                   8, // bitsPerComponent
                                                   width * 4 * sizeof(uint8_t), // bytesPerRow
                                                   sRGBColorSpace,
                                                   kCGImageAlphaPremultipliedFirst);
    
    CIContext *ciContext = [CIContext contextWithCGContext:cgContext options:nil];
    [ciContext drawImage:ciImage inRect:ciImage.extent fromRect:ciImage.extent];
    
    // Double check which way the origin is
    CGContextSetRGBFillColor(cgContext, 1, 0, 0, 1);
    CGContextFillRect(cgContext, CGRectMake(0, 0, 10, 10));
    
    if (!CGRectIsEmpty(normalizedRect)) {
        // The Core Graphics origin is bottom left, but the landmarks are top left. Flip accordingly.
        CGRect unnormalizedRect = CGRectMake(normalizedRect.origin.x * width,
                                             (1.0 - normalizedRect.origin.y - normalizedRect.size.height) * height,
                                             normalizedRect.size.width * width,
                                             normalizedRect.size.height * height);
        CGContextSetRGBStrokeColor(cgContext, 0, 1, 0, 1);
        CGContextSetLineWidth(cgContext, 2);
        CGContextStrokeRect(cgContext, unnormalizedRect);
    }
    
    for (SCLandmark2D *landmark in landmarks) {
        CGContextSetRGBFillColor(cgContext, landmark.color.x, landmark.color.y, landmark.color.z, 1);
        
        // The Core Graphics origin is bottom left, but the landmarks are top left. Flip accordingly.
        CGFloat landmarkY = 1.0 - landmark.y;
        CGRect rect = CGRectMake(landmark.x * width - 2.0,
                                 landmarkY * height - 2.0,
                                 4, 4);
        
        CGContextFillEllipseInRect(cgContext, rect);
    }
    
    CGImageRef imageRef = CGBitmapContextCreateImage(cgContext);
    
    // Now write it
    NSURL *fileURL = [NSURL fileURLWithPath:path];
    CGImageDestinationRef jpegDest = CGImageDestinationCreateWithURL((__bridge CFURLRef)fileURL, kUTTypeJPEG, 1, NULL);
    CGImageDestinationAddImage(jpegDest, imageRef, NULL);
    CGImageDestinationFinalize(jpegDest);
    
    CGImageRelease(imageRef);
    CGContextRelease(cgContext);
    CFRelease(jpegDest);
    CGColorSpaceRelease(sRGBColorSpace);
}

- (NSArray<SCLandmark2D *> *)analyzeImage:(CIImage *)image
                 normalizedEarBoundingBox:(CGRect)normalizedEarBoundingBox
                                isLeftEar:(BOOL)isLeftEar
{
    NSMutableArray *result = [[NSMutableArray alloc] init];
    
    // Expand the bounding box
    const CGFloat expansionFactor = 0.15;
    CGRect expandedBoundingBox = CGRectInset(normalizedEarBoundingBox,
                                             -normalizedEarBoundingBox.size.width * expansionFactor,
                                             -normalizedEarBoundingBox.size.height * expansionFactor);
    
    CGSize targetSize = CGSizeMake(300, 300);
    CIImage *croppedAndScaled = [[self class] _cropAndScaleImage:image
                                              normalizedCropRect:expandedBoundingBox
                                                      scaledSize:targetSize];
    
    CVPixelBufferRef pixelBuffer;
    if (CVPixelBufferCreate(NULL, // allocator
                            (int)targetSize.width,
                            (int)targetSize.height,
                            kCVPixelFormatType_32BGRA,
                            NULL, // attributes
                            &pixelBuffer)
        != kCVReturnSuccess)
    {
        return result;
    }
    
    [[CIContext context] render:croppedAndScaled toCVPixelBuffer:pixelBuffer];
    
    SCEarLandmarkingInput *inputs = [[SCEarLandmarkingInput alloc] initWithInputs__image_tensor__0:pixelBuffer];
    
    NSError *error = nil;
    SCEarLandmarkingOutput *predictions = [_model predictionFromFeatures:inputs error:&error];
    
    if (predictions == nil) {
        NSLog(@"Error predicting ear landmarks from inputs %@: %@", inputs, error);
        return result;
    }
    
    int landmarkCount = [predictions.normed_coordinates_yx.shape[0] intValue];
    double *landmarkData = (double *)predictions.normed_coordinates_yx.dataPointer;
    
    for (int landmarkIndex = 0; landmarkIndex < landmarkCount; ++landmarkIndex) {
        double landmarkX = landmarkData[landmarkIndex * 2 + 0];
        double landmarkY = landmarkData[landmarkIndex * 2 + 1];
        
        NSString *landmarkName = [[self class] landmarkNames][landmarkIndex];
        simd_float3 color;
        switch (landmarkIndex) {
            case 0:  color = simd_make_float3(1, 1, 1); break;
            case 1:  color = simd_make_float3(0, 1, 0); break;
            case 2:  color = simd_make_float3(0, 0, 1); break;
            case 3:  color = simd_make_float3(1, 1, 0); break;
            case 4:  color = simd_make_float3(1, 0, 1); break;
            default: color = simd_make_float3(0, 1, 1); break;
        }
        
        SCLandmark2D *landmark = [SCLandmark2D landmarkNamed:landmarkName
                                                       index:landmarkIndex + (int)(isLeftEar ? 0 : [[[self class] landmarkNames] count] / 2)
                                                    position:simd_make_float2(landmarkX, landmarkY)
                                                  confidence:1
                                                       color:color];
        
        [result addObject:landmark];
    }
    
#if TARGET_OS_OSX && DEBUG
    [[self class] _drawImage:croppedAndScaled
                   landmarks:result
              normalizedRect:CGRectZero
                      toFile:[NSString stringWithFormat:@"%@/EarIntermediate-%@.jpeg", @"/tmp", isLeftEar ? @"left" : @"right"]];
#endif
    
    // normalizedEarBoundingBox and expandedBoundingBox have bottom-left origin,
    // but landmarks have top-left origin. Flip accordingly.
    CGRect flippedExpandedBoundingBox = expandedBoundingBox;
    flippedExpandedBoundingBox.origin.y = 1.0 - expandedBoundingBox.origin.y - expandedBoundingBox.size.height;
    
    for (SCLandmark2D *landmark in result) {
        // Convert from the cropped image's coordinate system to the source image's
        landmark.x = landmark.x * flippedExpandedBoundingBox.size.width  + flippedExpandedBoundingBox.origin.x;
        landmark.y = landmark.y * flippedExpandedBoundingBox.size.height + flippedExpandedBoundingBox.origin.y;
    }
     
#if TARGET_OS_OSX && DEBUG
    [[self class] _drawImage:image
                   landmarks:result
              normalizedRect:flippedExpandedBoundingBox
                      toFile:[NSString stringWithFormat:@"%@/EarFinal-%@.jpeg", @"/tmp", isLeftEar ? @"left" : @"right"]];
#endif
    
    return result;
}

+ (NSArray<NSString *> *)landmarkNames
{
    static NSArray *landmarkNames;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        landmarkNames = @[
            @"earSaddleLeft",
            @"superiorPointLeft",
            @"posteriorPointLeft",
            @"inferiorPointLeft",
            @"tragionLeft",
            
            @"earSaddleRight",
            @"superiorPointRight",
            @"posteriorPointRight",
            @"inferiorPointRight",
            @"tragionRight",
        ];
    });
    return landmarkNames;
}

@end

NS_ASSUME_NONNULL_END
