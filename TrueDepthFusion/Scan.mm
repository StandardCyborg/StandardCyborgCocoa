//
//  Scan.m
//  DepthRenderer
//
//  Created by Aaron Thompson on 5/14/18.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <ModelIO/ModelIO.h>
#import <SceneKit/SceneKit.h>
#import <SSZipArchive/SSZipArchive.h>
#import <StandardCyborgFusion/StandardCyborgFusion.h>
#import <UIKit/UIKit.h>
#import <standard_cyborg/sc3d/ColorImage.hpp>

#import "Scan.h"

@implementation Scan {
    SCNGeometrySource *_vertexSource;
    SCNGeometrySource *_normalSource;
    SCNGeometrySource *_colorSource;
    SCPointCloud *_pointCloud;
    
    SCMeshTexturing *_meshTexturing;
}

+ (NSString *)stringFromDate:(NSDate *)date
{
    static NSDateFormatter *__datetimeFormatter;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        __datetimeFormatter = [[NSDateFormatter alloc] init];
        __datetimeFormatter.dateFormat = @"yyyy-MM-dd--HH-mm-ss";
    });
    
    return [__datetimeFormatter stringFromDate:date];
}
 
- (instancetype)initWithPointCloud:(SCPointCloud *)pointCloud
                         thumbnail:(UIImage * _Nullable)thumbnail
                         meshTexturing:(SCMeshTexturing*)meshTexturing
{
    self = [super init];
    if (self) {
        _pointCloud = pointCloud;
        _thumbnail = thumbnail;
        _dateCreated = [NSDate date];
        
        _meshTexturing = meshTexturing;

    }
    return self;
}

- (instancetype)initWithPLYPath:(NSString *)path
{
    self = [super init];
    if (self) {
        // Save this off for later lazy loading of data from the PLY file
        _plyPath = path;
        
        NSError *error = nil;
        NSDictionary *attributes = [[NSFileManager defaultManager] attributesOfItemAtPath:path error:&error];
        _dateCreated = attributes[NSFileCreationDate];
        
        NSString *thumbnailPath = [[self class] _thumbnailPathForPLYPath:path];
        _thumbnail = [UIImage imageWithContentsOfFile:thumbnailPath];
    }
    return self;
}

- (BOOL)writeToContainerPath:(NSString *)containerPath error:(NSError **)errorOut
{
    BOOL success = YES;
    NSString *plyFilename = [NSString stringWithFormat:@"Scan-%@.ply", [Scan stringFromDate:_dateCreated]];
    NSString *plyPath = [containerPath stringByAppendingPathComponent:plyFilename];
    NSString *jpegPath = [[self class] _thumbnailPathForPLYPath:plyPath];
    
    @autoreleasepool {
        NSData *jpegData = UIImageJPEGRepresentation(_thumbnail, 0.8);
        success = [jpegData writeToFile:jpegPath options:NSDataWritingAtomic error:errorOut];
    }
    
    if (success) {
        success = [_pointCloud writeToPLYAtPath:plyPath];
    }
    
    if (!success && errorOut != NULL) {
        *errorOut = [NSError errorWithDomain:NSCocoaErrorDomain code:0
                                    userInfo:@{ NSLocalizedDescriptionKey : plyPath }];
    }
    
    if (success) {
        _plyPath = plyPath;
    }
    
    return success;
}

- (BOOL)deleteFilesWithError:(NSError **)errorOut
{
    BOOL success = YES;
    
    if (_plyPath != nil) {
        NSString *thumbnailPath = [[self class] _thumbnailPathForPLYPath:_plyPath];
        success = success && [[NSFileManager defaultManager] removeItemAtPath:_plyPath error:errorOut];
        success = success && [[NSFileManager defaultManager] removeItemAtPath:thumbnailPath error:errorOut];
    }
    
    return success;
}

- (SCPointCloud *)pointCloud
{
    if (_pointCloud == nil) {
        [self _loadDataFromPLY];
    }
    
    return _pointCloud;
}

- (SCNGeometrySource *)vertexSource
{
    if (_vertexSource == nil) {
        _vertexSource = [[self pointCloud] buildVertexGeometrySource];
    }
    
    return _vertexSource;
}

- (SCNGeometrySource *)normalSource
{
    if (_normalSource == nil) {
        _normalSource = [[self pointCloud] buildNormalGeometrySource];
    }
    
    return _normalSource;
}

- (SCNGeometrySource *)colorSource
{
    if (_colorSource == nil) {
        _colorSource = [[self pointCloud] buildColorGeometrySource];
    }
    
    return _colorSource;
}

- (void)_loadDataFromPLY
{
    _pointCloud = [[SCPointCloud alloc] initWithPLYPath:_plyPath normalizeNormals:YES];
}

+ (NSString * _Nullable)_thumbnailPathForPLYPath:(NSString *)plyPath {
    return [plyPath stringByReplacingOccurrencesOfString:@".ply" withString:@".jpeg"];
}

@end

@implementation Scan (Compression)

- (NSURL *)writeCompressedPLY
{
    NSString *plyPath = _plyPath;
    NSString *zipPath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"scan.ply.zip"];
    
    if (plyPath == nil) {
        plyPath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"scan.ply"];
        
        [_pointCloud writeToPLYAtPath:plyPath];
    }
    
    [SSZipArchive createZipFileAtPath:zipPath withFilesAtPaths:@[plyPath]];
    
    return [NSURL fileURLWithPath:zipPath];
}

- (NSURL *)writeUSDZ
{
    NSURL *USDZURL = [[NSURL fileURLWithPath:NSTemporaryDirectory()] URLByAppendingPathComponent:@"Scan.usdz"];
    
    BOOL success = [_pointCloud writeToUSDZAtPath:[USDZURL path]];
    
    return success ? USDZURL : nil;
}

@end
