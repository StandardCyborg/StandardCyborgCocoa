//
//  SCPerceptionRecorder.h
//  StandardCyborgSDK
//
//  Created by Paul Wais on 5/24/20.
//  Copyright © 2020 Standard Cyborg. All rights reserved.
//

#import <AVKit/AVKit.h>
#import <CoreMotion/CoreMotion.h>
#import <CoreVideo/CoreVideo.h>
#import <Foundation/Foundation.h>
#import <SceneKit/SceneKit.h>

NS_ASSUME_NONNULL_BEGIN


// A nanosecond-precision unix timestamp; compatible with google::protobuf::Timestamp
typedef struct sc_nanostamp {
    int64_t seconds;
    int32_t nanos;
} sc_nanostamp;


@interface SCPerceptionRecorder : NSObject

/// Utilities

// Get the kernel boot time as unix epoch time. AVCaptureSession
// timestamps are given relative to this event.
+ (sc_nanostamp)getKernelBootAbsoluteTime;

// Convert an AVCapture session time / CMDeviceMotion time `timestamp` (which is relative to
// system boot-up (`bootTime`) to an absolute unix timestamp `nanostamp`.
+ (sc_nanostamp)getAbsoluteNanostamp:(CMTime)timestamp bootTime:(sc_nanostamp) bootTime
NS_SWIFT_NAME(getAbsoluteNanostamp(timestamp:bootTime:));


- (instancetype)init;

// Accumulate one BGR-D Frame for the given sensor.  The given `colorTime` and `depthTime`
// will be used as absolute timestamps for the recorded data-- the user should probably use
// getKernelBootAbsoluteTime() and getAbsoluteNanostamp() above to provide unix epoch
// timestamps.
- (void)accumulateSensor:(NSString *)sensorName
             colorBuffer:(CVPixelBufferRef)colorBuffer
               colorTime:(sc_nanostamp)colorTime
             depthBuffer:(CVPixelBufferRef)depthBuffer
               depthTime:(sc_nanostamp)depthTime
         calibrationData:(AVCameraCalibrationData *)calibrationData
NS_SWIFT_NAME(accumulateSensor(sensorName:colorBuffer:colorTime:depthBuffer:depthTime:calibrationData:));

- (void)accumulateDeviceMotion:(NSString *)frame
                    motionTime:(sc_nanostamp)motionTime
                        motion:(CMDeviceMotion *)motion
NS_SWIFT_NAME(accumulateMotion(frame:motionTime:deviceMotion:));

// Finalize and close the archive; futher accumulation is ignored.
- (void)finalizeArchive;

// Path to where the sensor data is being recorded
@property (readonly, nonatomic, copy) NSString* archivePath;

@end

NS_ASSUME_NONNULL_END
