//
//  SCAssimilatedFrameMetadata.h
//  StandardCyborgFusion
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <CoreVideo/CoreVideo.h>
#import <Foundation/Foundation.h>
#import <simd/simd.h>

typedef NS_ENUM(NSInteger, SCAssimilatedFrameResult) {
    /** The frame was successfully assimilated */
    SCAssimilatedFrameResultSucceeded,
    /** The frame assimilated successfully, but tracking it was challenging; the user may need to slow down */
    SCAssimilatedFrameResultPoorTracking,
    /** The frame failed to be assimilated and was dropped */
    SCAssimilatedFrameResultLostTracking,
    /** Too many successive frames failed to assimilate; scanning has now ended */
    SCAssimilatedFrameResultFailed
};

typedef struct {
    SCAssimilatedFrameResult result;
    matrix_float4x4 viewMatrix;
    matrix_float4x4 projectionMatrix;
    CVPixelBufferRef _Nullable depthBuffer;
    CVPixelBufferRef _Nullable colorBuffer;
    
    CFAbsoluteTime assimilationTime;
} SCAssimilatedFrameMetadata;

extern simd_float3 EulerAnglesFromSCAssimilatedFrameMetadata(SCAssimilatedFrameMetadata metadata);
