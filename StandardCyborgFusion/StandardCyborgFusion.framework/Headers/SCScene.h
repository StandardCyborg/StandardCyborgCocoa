//  SCScene.h
//
//  Created by Aaron Thompson on 12/30/19.
//

#import <Foundation/Foundation.h>

@class SCMesh;
@class SCNNode;
@class SCPointCloud;

NS_ASSUME_NONNULL_BEGIN

@interface SCScene : NSObject

@property (nonatomic, readonly, nullable) SCPointCloud *pointCloud;
@property (nonatomic, readonly, nullable) SCMesh *mesh;
@property (nonatomic, readonly) SCNNode *rootNode;

- (instancetype)initWithPointCloud:(SCPointCloud * _Nullable)pointCloud mesh:(SCMesh * _Nullable)mesh;

- (instancetype)initWithGLTFAtPath:(NSString *)GLTFPath;

- (void)writeToGLTFAtPath:(NSString *)GLTFPath;

@end

NS_ASSUME_NONNULL_END
