//
//  SCMetalSimulatorStubs.h
//  StandardCyborgFusion
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

// NOTE: This exists only to allow building for the iOS Simulator on iOS versions < 13.0
//       It may safely be removed once the minimum iOS deployment target reaches 13.0

#import <Foundation/Foundation.h>
#if !__has_include(<QuartzCore/CAMetalLayer.h>)
#import <CoreVideo/CVImageBuffer.h>
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

NS_ASSUME_NONNULL_BEGIN

// MARK: - CVMetalTexture.h

typedef CVImageBufferRef CVMetalTextureRef;
CV_EXPORT const CFStringRef CV_NONNULL kCVMetalTextureUsage;
CV_EXPORT id <MTLTexture> CV_NULLABLE CVMetalTextureGetTexture(CVMetalTextureRef CV_NONNULL image);


// MARK: - CVMetalTextureCache.h
typedef struct
CV_BRIDGED_TYPE(id) __CVMetalTextureCache *CVMetalTextureCacheRef;
CV_EXPORT CVReturn CVMetalTextureCacheCreate(CFAllocatorRef CV_NULLABLE allocator,
                                             CFDictionaryRef CV_NULLABLE cacheAttributes,
                                             id <MTLDevice> CV_NONNULL metalDevice,
                                             CFDictionaryRef CV_NULLABLE textureAttributes,
                                             CV_RETURNS_RETAINED_PARAMETER CVMetalTextureCacheRef CV_NULLABLE * CV_NONNULL cacheOut);
CV_EXPORT CVReturn CVMetalTextureCacheCreateTextureFromImage(CFAllocatorRef CV_NULLABLE allocator,
                                                             CVMetalTextureCacheRef CV_NONNULL textureCache,
                                                             CVImageBufferRef CV_NONNULL sourceImage,
                                                             CFDictionaryRef CV_NULLABLE textureAttributes,
                                                             MTLPixelFormat pixelFormat,
                                                             size_t width,
                                                             size_t height,
                                                             size_t planeIndex,
                                                             CV_RETURNS_RETAINED_PARAMETER CVMetalTextureRef CV_NULLABLE * CV_NONNULL textureOut);


// MARK: - CAMetalLayer.h

@class CAMetalLayer;

@protocol CAMetalDrawable <MTLDrawable>
@property(readonly) id <MTLTexture> texture;
@property(readonly) CAMetalLayer *layer;
@end


@interface CAMetalLayer : CALayer
@property(nullable, retain) id <MTLDevice> device;
@property MTLPixelFormat pixelFormat;
@property BOOL framebufferOnly;
@property CGSize drawableSize;
- (nullable id <CAMetalDrawable>)nextDrawable;
@end


NS_ASSUME_NONNULL_END

#endif // __has_include(<QuartzCore/CAMetalLayer.h>)
