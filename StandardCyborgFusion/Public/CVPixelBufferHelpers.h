//
//  CVPixelBufferHelpers.h
//  StandardCyborgFusion
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreVideo/CoreVideo.h>

NS_ASSUME_NONNULL_BEGIN

FOUNDATION_EXPORT void CVPixelBufferReplaceNaNs(CVPixelBufferRef buffer, float replacement);

FOUNDATION_EXPORT float AverageDepthFromValues(float *depthValues,
                                               off_t frameWidth,
                                               off_t frameHeight);

FOUNDATION_EXPORT float CVPixelBufferAverageDepthAroundCenter(CVPixelBufferRef buffer);

FOUNDATION_EXPORT CVReturn CVPixelBufferDeepCopy(CVPixelBufferRef source, CVPixelBufferRef _Nullable * _Nonnull copyOut);

NS_ASSUME_NONNULL_END
