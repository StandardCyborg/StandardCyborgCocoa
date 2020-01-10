//
//  SCPointCloud+FileIO.h
//  StandardCyborgFusion
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <StandardCyborgFusion/SCPointCloud.h>

@class SCLandmark3D;

NS_ASSUME_NONNULL_BEGIN

@interface SCPointCloud (FileIO)

- (nullable instancetype)initWithPLYPath:(NSString *)PLYPath;
- (nullable instancetype)initWithPLYPath:(NSString *)PLYPath normalizeNormals:(BOOL)normalizeNormals;

- (nullable instancetype)initWithOBJPath:(NSString *)OBJPath;

- (BOOL)writeToPLYAtPath:(NSString *)PLYPath;

- (BOOL)writeToOBJAtPath:(NSString *)OBJPath;

- (BOOL)writeToUSDZAtPath:(NSString *)USDZPath;

- (BOOL)writeToSceneGraphGLTFAtPath:(NSString *)GLTFPath
                          landmarks:(NSSet<SCLandmark3D *> * _Nullable)landmarks;

@end

NS_ASSUME_NONNULL_END
