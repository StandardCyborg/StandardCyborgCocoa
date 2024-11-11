//
//  CVPixelBufferHelpers.m
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 12/14/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreVideo/CoreVideo.h>
#import <StandardCyborgFusion/CVPixelBufferHelpers.h>

void CVPixelBufferReplaceNaNs(CVPixelBufferRef buffer, float replacement)
{
    CVPixelBufferLockBaseAddress(buffer, 0);
    
    size_t width = CVPixelBufferGetWidth(buffer);
    size_t height = CVPixelBufferGetHeight(buffer);
    size_t pixelCount = width * height;
    float *floatValues = (float *)CVPixelBufferGetBaseAddress(buffer);
    
    for (off_t i = 0; i < pixelCount; ++i)
    {
        float value = floatValues[i];
        
        if (isnan(value))
        {
            floatValues[i] = replacement;
        }
    }
    
    CVPixelBufferUnlockBaseAddress(buffer, 0);
}


static const off_t kSearchRadiusSampleCount = 4;
static const float kSearchRadiusPercentageFromCenter = 0.06;

float AverageDepthFromValues(float *depthValues,
                             off_t frameWidth,
                             off_t frameHeight)
{
    const float widthFloat = frameWidth;
    const float heightFloat = frameHeight;
    const float centerRow = heightFloat / 2.0f;
    const float centerCol = widthFloat / 2.0f;
    
    float depthSum = 0;
    float depthCount = 0;
    
    // The method says "radius", but it's really using a square just for ease of implementation
    for (off_t rowIndex = -kSearchRadiusSampleCount; rowIndex <= kSearchRadiusSampleCount; ++rowIndex)
    {
        off_t row = (off_t)(centerRow + rowIndex * kSearchRadiusPercentageFromCenter * heightFloat / 2.0f);
        
        for (off_t colIndex = -kSearchRadiusSampleCount; colIndex <= kSearchRadiusSampleCount; ++colIndex)
        {
            off_t col = (off_t)(centerCol + colIndex * kSearchRadiusPercentageFromCenter * widthFloat / 2.0f);
            off_t index = row * frameWidth + col;
            
            float depthValue = depthValues[index];
            
            if (!isnan(depthValue) && depthValue > 0)
            {
                depthSum += depthValue;
                ++depthCount;
            }
        }
    }
    
    float averageDepth = depthSum / depthCount;
    
    return averageDepth;
}

float CVPixelBufferAverageDepthAroundCenter(CVPixelBufferRef buffer)
{
    if (buffer == NULL) { return -1; }
    
    CVPixelBufferLockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
    size_t width = CVPixelBufferGetWidth(buffer);
    size_t height = CVPixelBufferGetHeight(buffer);
    float *depthValues = (float *)CVPixelBufferGetBaseAddress(buffer);
    
    float result = AverageDepthFromValues(depthValues, width, height);
    
    CVPixelBufferUnlockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
    
    return result;
}

CVReturn CVPixelBufferDeepCopy(CVPixelBufferRef source, CVPixelBufferRef *copyOut)
{
    CVReturn result = kCVReturnSuccess;
    
    if (*copyOut == NULL) {
        result = CVPixelBufferCreate(NULL,
                                     CVPixelBufferGetWidth(source),
                                     CVPixelBufferGetHeight(source),
                                     CVPixelBufferGetPixelFormatType(source),
                                     CVBufferGetAttachments(source, kCVAttachmentMode_ShouldPropagate),
                                     copyOut);
    } else {
        assert(CVPixelBufferGetWidth(*copyOut) == CVPixelBufferGetWidth(source));
        assert(CVPixelBufferGetHeight(*copyOut) == CVPixelBufferGetHeight(source));
    }
    
    CVPixelBufferLockBaseAddress(source, kCVPixelBufferLock_ReadOnly);
    CVPixelBufferLockBaseAddress(*copyOut, 0);
    
    for (off_t plane = 0; plane < CVPixelBufferGetPlaneCount(source); plane++) {
        void *sourceAddress = CVPixelBufferGetBaseAddressOfPlane(source, plane);
        void *destAddress   = CVPixelBufferGetBaseAddressOfPlane(*copyOut, plane);
        size_t height       = CVPixelBufferGetHeightOfPlane(source, plane);
        size_t bytesPerRow   = CVPixelBufferGetBytesPerRowOfPlane(source, plane);
        
        memcpy(destAddress, sourceAddress, height * bytesPerRow);
    }
    
    if (CVPixelBufferGetPlaneCount(source) == 0) {
        void *sourceAddress = CVPixelBufferGetBaseAddress(source);
        void *destAddress   = CVPixelBufferGetBaseAddress(*copyOut);
        size_t height       = CVPixelBufferGetHeight(source);
        size_t bytesPerRow  = CVPixelBufferGetBytesPerRow(source);
        
        memcpy(destAddress, sourceAddress, height * bytesPerRow);
    }
    
    CVPixelBufferUnlockBaseAddress(source, kCVPixelBufferLock_ReadOnly);
    CVPixelBufferUnlockBaseAddress(*copyOut, 0);
    
    return result;
}
