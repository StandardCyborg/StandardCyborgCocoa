//  SCScene.h
//
//  Created by Aaron Thompson on 12/30/19.
//

#import <Foundation/Foundation.h>
#import <simd/simd.h>

@class SCMesh;
@class SCNNode;
@class SCPointCloud;

NS_ASSUME_NONNULL_BEGIN

@interface SCScene : NSObject

@property (nonatomic, readonly, nullable) SCPointCloud *pointCloud;
@property (nonatomic, readonly, nullable) SCMesh *mesh;
@property (nonatomic, readonly) SCNNode *rootNode;

- (instancetype)init NS_UNAVAILABLE;

- (instancetype)initWithPointCloud:(SCPointCloud * _Nullable)pointCloud
                              mesh:(SCMesh * _Nullable)mesh;

/// Useful for migrating scans generated with StandardCyborgFusion versions < 1.6.4
/// into the new standard orientation
- (instancetype)initWithPointCloud:(SCPointCloud * _Nullable)pointCloud
                              mesh:(SCMesh * _Nullable)mesh
                         transform:(simd_float4x4)transform;

- (instancetype)initWithGLTFAtPath:(NSString *)GLTFPath;

- (void)writeToGLTFAtPath:(NSString *)GLTFPath;

@end

NS_ASSUME_NONNULL_END
