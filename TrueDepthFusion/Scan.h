//
//  Scan.h
//  DepthRenderer
//
//  Created by Aaron Thompson on 5/14/18.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>

#import <StandardCyborgFusion/SCMeshTexturing.h>

@class SCPointCloud;
@class SCNGeometrySource;
@class UIImage;

NS_ASSUME_NONNULL_BEGIN

@interface Scan : NSObject

+ (NSString *)stringFromDate:(NSDate *)date;

- (instancetype)initWithPointCloud:(SCPointCloud *)scan
                         thumbnail:(UIImage * _Nullable)thumbnail
                         meshTexturing:(SCMeshTexturing*)meshTexturing
;

- (instancetype)initWithPLYPath:(NSString *)path;

- (BOOL)writeToContainerPath:(NSString *)path error:(NSError ** _Nullable)error;
- (BOOL)deleteFilesWithError:(NSError ** _Nullable)error;

@property (nonatomic, readonly) SCPointCloud *pointCloud;
@property (nonatomic, readonly, nullable) NSString *plyPath;
@property (nonatomic, readonly) NSDate *dateCreated;
@property (nonatomic, nullable) UIImage *thumbnail;

@property (nonatomic, readonly) SCNGeometrySource *vertexSource;
@property (nonatomic, readonly) SCNGeometrySource *normalSource;
@property (nonatomic, readonly) SCNGeometrySource *colorSource;
 
@property (nonatomic, readonly) SCMeshTexturing *meshTexturing;

@end

@interface Scan (Compression)
- (NSURL *)writeCompressedPLY;
- (NSURL * _Nullable)writeUSDZ;
@end

NS_ASSUME_NONNULL_END
