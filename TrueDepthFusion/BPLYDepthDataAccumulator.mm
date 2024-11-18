//
//  BPLYDepthDataAccumulator.mm
//  DepthRenderer
//
//  Created by Aaron Thompson on 7/5/18.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import "BPLYDepthDataAccumulator.h"
#import <standard_cyborg/math/MathHelpers.hpp>

#import <CoreImage/CoreImage.h>
#import <simd/simd.h>
#import <SSZipArchive/SSZipArchive.h>
#import <StandardCyborgFusion/PerspectiveCamera+AVFoundation.hpp>
#import <StandardCyborgFusion/GeometryHelpers.hpp>
#import <StandardCyborgFusion/PointCloudIO.hpp>
#import <StandardCyborgFusion/SCOfflineReconstructionManager.h>
#import <StandardCyborgFusion/StandardCyborgFusion.h>

using namespace standard_cyborg;

@interface _BPLYPointCloudWriter : NSObject

+ (void)writeMetadata:(NSDictionary *)metadata
           pointCloud:(SCPointCloud *)pointCloud
               toPath:(NSString *)path;

@end

@implementation BPLYDepthDataAccumulator {
    NSString *_containerDirectory;
    NSInteger _lastWrittenFrameIndex;
    NSInteger _lastReadFrameIndex;
    SCOfflineReconstructionManager *_reconstructionManager;
    NSMutableArray *_deviceMotions;
    CIContext *_imageContext;
    CGColorSpaceRef _imageColorSpace;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        _deviceMotions = [[NSMutableArray alloc] init];
        _containerDirectory = [NSTemporaryDirectory() stringByAppendingPathComponent:@"/DepthFrames"];
        _imageContext = [CIContext contextWithMTLDevice:MTLCreateSystemDefaultDevice()];
        _imageColorSpace = CGColorSpaceCreateDeviceRGB();

        if ([[NSFileManager defaultManager] fileExistsAtPath:_containerDirectory]) {
            [[NSFileManager defaultManager] removeItemAtPath:_containerDirectory error:NULL];
        }
        NSError *error;
        if (![[NSFileManager defaultManager] createDirectoryAtPath:_containerDirectory withIntermediateDirectories:NO attributes:nil error:NULL]) {
            NSLog(@"Error creating container directory at %@: %@", _containerDirectory, error);
        }
    }
    return self;
}

- (void)dealloc
{
    CGColorSpaceRelease(_imageColorSpace);
}

- (void)accumulateColorBuffer:(CVPixelBufferRef)colorBuffer
                    colorTime:(CMTime)colorTime
                  depthBuffer:(CVPixelBufferRef)depthBuffer
                    depthTime:(CMTime)depthTime
              calibrationData:(AVCameraCalibrationData *)calibrationData
{
    CVPixelBufferLockBaseAddress(colorBuffer, kCVPixelBufferLock_ReadOnly);
    CVPixelBufferLockBaseAddress(depthBuffer, kCVPixelBufferLock_ReadOnly);
    
    size_t width = CVPixelBufferGetWidth(depthBuffer);
    size_t height = CVPixelBufferGetHeight(depthBuffer);
    
    sc3d::PerspectiveCamera camera = PerspectiveCameraFromAVCameraCalibrationData(calibrationData, width, height);
    RawFrame rawFrame(camera, width, height);
    
    [[self class] _fillDepthVector:rawFrame.depths
                       colorMatrix:rawFrame.colors
                   fromDepthBuffer:depthBuffer
                       colorBuffer:colorBuffer
                 replacingNaNsWith:9999];
    
    CVPixelBufferUnlockBaseAddress(colorBuffer, kCVPixelBufferLock_ReadOnly);
    CVPixelBufferUnlockBaseAddress(depthBuffer, kCVPixelBufferLock_ReadOnly);
    
    NSString *BPLYPath = [self _dequeueNextFilePathForWriting];
    PointCloudIO::WriteRawFrameToBPLYFile(rawFrame, [BPLYPath UTF8String]);
    
    NSString *JPEGPath = [BPLYPath stringByReplacingOccurrencesOfString:@".ply" withString:@".jpeg"];
    CIImage *ciImage = [CIImage imageWithCVPixelBuffer:colorBuffer];
    NSError *error = nil;
    BOOL success = [_imageContext writeJPEGRepresentationOfImage:ciImage
                                                           toURL:[NSURL fileURLWithPath:JPEGPath]
                                                      colorSpace:_imageColorSpace
                                                         options:@{}
                                                           error:&error];
    if (!success) {
        NSLog(@"Failed to write JPEG to %@: %@", JPEGPath, error);
    }
}

- (void)accumulateDeviceMotion:(CMDeviceMotion *)motion
{
    [_deviceMotions addObject:motion];
}

- (void)accumulatePointCloud:(SCPointCloud *)pointCloud
{
    NSMutableDictionary *metadata = [[NSMutableDictionary alloc] init];
    metadata[@"color_space"] = @"sRGB";
    metadata[@"absolute_timestamp"] = @(CFAbsoluteTimeGetCurrent());
    metadata[@"timestamp"] = @([[NSProcessInfo processInfo] systemUptime]);
    
    NSString *filePath = [self _dequeueNextFilePathForWriting];
    
    [_BPLYPointCloudWriter writeMetadata:metadata pointCloud:pointCloud toPath:filePath];
}

- (NSURL *)exportFrameSequenceToZip
{
    [self _writeIMUDataToJSON];
    
    NSString *zipPath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"DepthFrames.zip"];
    [SSZipArchive createZipFileAtPath:zipPath withContentsOfDirectory:_containerDirectory];
    
    return [NSURL fileURLWithPath:zipPath];
}

- (void)_writeIMUDataToJSON
{
    NSMutableArray *motionDicts = [[NSMutableArray alloc] init];
    
    for (CMDeviceMotion *motion in _deviceMotions) {
        NSDictionary *motionDict = [self _dictionaryFromDeviceMotion:motion];
        [motionDicts addObject:motionDict];
    }
    
    NSData *jsonData = [NSJSONSerialization dataWithJSONObject:motionDicts options:NSJSONWritingPrettyPrinted error:NULL];
    [jsonData writeToFile:[_containerDirectory stringByAppendingPathComponent:@"motion-data.json"] atomically:NO];
}

- (NSDictionary *)_dictionaryFromDeviceMotion:(CMDeviceMotion *)motion
{
    return
    @{
      @"timestamp" : @(motion.timestamp),
      @"rotation_x" : @(motion.rotationRate.x),
      @"rotation_y" : @(motion.rotationRate.y),
      @"rotation_z" : @(motion.rotationRate.z),
      @"gravity_x" : @(motion.gravity.x),
      @"gravity_y" : @(motion.gravity.y),
      @"gravity_z" : @(motion.gravity.z),
      @"acceleration_x" : @(motion.userAcceleration.x),
      @"acceleration_y" : @(motion.userAcceleration.y),
      @"acceleration_z" : @(motion.userAcceleration.z),
      @"magnetic_field_accuracy" : @(motion.magneticField.accuracy),
      @"magnetic_field_x" : @(motion.magneticField.field.x),
      @"magnetic_field_y" : @(motion.magneticField.field.y),
      @"magnetic_field_z" : @(motion.magneticField.field.z),
      @"attitude_x" : @(motion.attitude.quaternion.x),
      @"attitude_y" : @(motion.attitude.quaternion.y),
      @"attitude_z" : @(motion.attitude.quaternion.z),
      @"attitude_w" : @(motion.attitude.quaternion.w),
    };
}

- (SCPointCloud *)loadNextPointCloud
{
    NSString *nextFilePath = [self _dequeueNextFilePathForReading];
    if (![[NSFileManager defaultManager] fileExistsAtPath:nextFilePath]) {
        return nil;
    }
    
    if (_reconstructionManager == nil) {
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        id<MTLCommandQueue> commandQueue = [device newCommandQueue];
        _reconstructionManager = [[SCOfflineReconstructionManager alloc] initWithDevice:device commandQueue:commandQueue maxThreadCount:2];
    }
    
    return [_reconstructionManager reconstructRawFrameFromBPLYAtPath:nextFilePath];
}

- (void)resetNextPointCloud
{
    _lastReadFrameIndex = 0;
}

- (NSString *)containerPath
{
    return _containerDirectory;
}

// MARK: - Private

- (NSString *)_dequeueNextFilePathForWriting
{
    NSString *filename = [NSString stringWithFormat:@"frame-%03d.ply", (int)_lastWrittenFrameIndex];
    
    ++_lastWrittenFrameIndex;
    
    return [_containerDirectory stringByAppendingPathComponent:filename];
}

- (NSString *)_dequeueNextFilePathForReading
{
    NSString *filename = [NSString stringWithFormat:@"frame-%03d.ply", (int)_lastReadFrameIndex];
    
    ++_lastReadFrameIndex;
    
    return [_containerDirectory stringByAppendingPathComponent:filename];
}

+ (void)_fillDepthVector:(std::vector<float>&)depthVectorOut
             colorMatrix:(std::vector<math::Vec3>&)colorMatrixOut
         fromDepthBuffer:(CVPixelBufferRef)depthBuffer
             colorBuffer:(CVPixelBufferRef)colorBuffer
       replacingNaNsWith:(float)nanReplacement
{
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
    static const float kGammaCorrection = 2.2f;
    
    
    assert(depthPixelCount == depthVectorOut.size());
    assert(depthPixelCount == colorMatrixOut.size());
    depthVectorOut.resize(depthPixelCount);
    colorMatrixOut.resize(depthPixelCount);
    
    size_t depthIndex = 0;
    for (size_t y = 0; y < depthHeight; ++y)
    {
        for (size_t x = 0; x < depthWidth; ++x)
        {
            float depth = depthBufferValues[y * depthWidth + x];
            
            size_t rgbX = (size_t)floorf(x * depthToColorRatio.x);
            size_t rgbY = (size_t)floorf(y * depthToColorRatio.y);
            size_t rgbIndex = (rgbY * rgbBytesPerRow + rgbX * rgbBytesPerPixel);
            
            uint8_t b = colorBufferValues[rgbIndex + 0];
            uint8_t g = colorBufferValues[rgbIndex + 1];
            uint8_t r = colorBufferValues[rgbIndex + 2];
            math::Vec3 normalizedRGB(
                                   pow(r * rgbNormalize, kGammaCorrection),
                                   pow(g * rgbNormalize, kGammaCorrection),
                                   pow(b * rgbNormalize, kGammaCorrection));
            
            depthVectorOut[depthIndex] = isnan(depth) ? nanReplacement : depth;
            colorMatrixOut[depthIndex] = normalizedRGB;
            
            ++depthIndex;
        }
    }
}

@end

@implementation _BPLYPointCloudWriter

+ (void)writeMetadata:(NSDictionary *)metadata
           pointCloud:(SCPointCloud *)pointCloud
               toPath:(NSString *)path
{
    FILE *file = fopen([path UTF8String], "w");
    
    [self _writeHeaderStartToFile:file];
    [self _writeHeaderMetadata:metadata toFile:file];
    [self _writeHeadersForPointCloud:pointCloud toFile:file];
    [self _writeHeaderEndToFile:file];
    [self _writePointCloudData:pointCloud toFile:file];
    
    fclose(file);
}

+ (void)_writeHeaderStartToFile:(FILE *)file
{
    NSBundle *bundle = [NSBundle mainBundle];
    NSString *frameworkVersionString = [bundle objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
    
    fprintf(file, "ply\n");
    fprintf(file, "format binary_little_endian 1.0\n");
    fprintf(file, "comment StandardCyborgFusionVersion %s\n", [frameworkVersionString UTF8String]);
}

+ (void)_writeHeaderMetadata:(NSDictionary *)dict toFile:(FILE *)file
{
    NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dict options:0 error:NULL];
    
    if ([jsonData length] > 0) {
        fprintf(file, "comment StandardCyborgFusionMetadata ");
        fwrite([jsonData bytes], 1, [jsonData length], file);
        fprintf(file, "\n");
    }
}

+ (void)_writeHeadersForPointCloud:(SCPointCloud *)pointCloud toFile:(FILE *)file
{
    fprintf(file, "element vertex %ld\n", [pointCloud pointCount]);
    fprintf(file, "property float x\n");
    fprintf(file, "property float y\n");
    fprintf(file, "property float z\n");
    fprintf(file, "property float nx\n");
    fprintf(file, "property float ny\n");
    fprintf(file, "property float nz\n");
    fprintf(file, "property float red\n");
    fprintf(file, "property float green\n");
    fprintf(file, "property float blue\n");
}

+ (void)_writeHeaderEndToFile:(FILE *)file
{
    fprintf(file, "end_header\n");
}

+ (void)_writePointCloudData:(SCPointCloud *)pointCloud toFile:(FILE *)file
{
    const void *pointsData = [[pointCloud pointsData] bytes];
    NSInteger pointCount = [pointCloud pointCount];
    size_t stride = [SCPointCloud pointStride];
    // For performance reasons, abusing the fact that we already know the layout
    // of each surfel to be position, normal, color in contiguous order
    size_t perPointDataStart = [SCPointCloud positionOffset];
    size_t perPointDataEnd = [SCPointCloud colorOffset] + [SCPointCloud colorComponentSize] * [SCPointCloud colorComponentCount];
    size_t perPointDataSize = perPointDataEnd - perPointDataStart;
    size_t dataEnd = pointCount * stride;
    
    for (size_t dataOffset = perPointDataStart; dataOffset < dataEnd; dataOffset += stride) {
        fwrite((uint8_t *)pointsData + dataOffset, perPointDataSize, 1, file);
    }
}

@end
