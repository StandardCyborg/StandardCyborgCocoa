//
//  SCPointCloud+SceneKit.h
//  StandardCyborgFusion
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <StandardCyborgFusion/SCPointCloud.h>

@class SCNGeometryElement;
@class SCNGeometrySource;
@class SCNNode;

NS_ASSUME_NONNULL_BEGIN

@interface SCPointCloud (SceneKit)

- (SCNGeometrySource *)buildVertexGeometrySource;
- (SCNGeometrySource *)buildNormalGeometrySource;
- (SCNGeometrySource *)buildColorGeometrySource;

- (SCNGeometryElement *)buildPointCloudGeometryElement;

- (SCNNode *)buildPointCloudNode;

@end

NS_ASSUME_NONNULL_END
