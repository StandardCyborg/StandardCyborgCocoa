//
//  SCMetalSimulatorStubs.m
//  StandardCyborgFusion
//

// NOTE: This exists only to allow building for the iOS Simulator on iOS versions < 13.0
//       It may safely be removed once the minimum iOS deployment target reaches 13.0

#import "SCMetalSimulatorStubs.h"
#if !__has_include(<QuartzCore/CAMetalLayer.h>)

const CFStringRef CV_NONNULL kCVMetalTextureUsage = CFSTR("INVALID");

id <MTLTexture> CV_NULLABLE CVMetalTextureGetTexture(CVMetalTextureRef CV_NONNULL image) {
    return nil;
}

CVReturn CVMetalTextureCacheCreate(CFAllocatorRef CV_NULLABLE allocator,
                                   CFDictionaryRef CV_NULLABLE cacheAttributes,
                                   id <MTLDevice> CV_NONNULL metalDevice,
                                   CFDictionaryRef CV_NULLABLE textureAttributes,
                                   CV_RETURNS_RETAINED_PARAMETER CVMetalTextureCacheRef CV_NULLABLE * CV_NONNULL cacheOut) {
    return kCVReturnError;
}

CVReturn CVMetalTextureCacheCreateTextureFromImage(CFAllocatorRef CV_NULLABLE allocator,
                                                   CVMetalTextureCacheRef CV_NONNULL textureCache,
                                                   CVImageBufferRef CV_NONNULL sourceImage,
                                                   CFDictionaryRef CV_NULLABLE textureAttributes,
                                                   MTLPixelFormat pixelFormat,
                                                   size_t width,
                                                   size_t height,
                                                   size_t planeIndex,
                                                   CV_RETURNS_RETAINED_PARAMETER CVMetalTextureRef CV_NULLABLE * CV_NONNULL textureOut) {
    return kCVReturnError;
}

@implementation CAMetalLayer
- (nullable id <CAMetalDrawable>)nextDrawable {
    return nil;
}
@end

#endif // __has_include(<QuartzCore/CAMetalLayer.h>)
