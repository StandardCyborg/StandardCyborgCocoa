//
//  SCPointCloud+SceneKit.h
//  StandardCyborgFusion
//
//

#import <Foundation/Foundation.h>
#import <StandardCyborgFusion/SCPointCloud.h>

@class SCNGeometryElement;
@class SCNGeometrySource;
@class SCNNode;
@class SCLandmark3D;

NS_ASSUME_NONNULL_BEGIN

@interface SCPointCloud (SceneKit)

- (SCNGeometrySource *)buildVertexGeometrySource;
- (SCNGeometrySource *)buildNormalGeometrySource;
- (SCNGeometrySource *)buildColorGeometrySource;

- (SCNGeometryElement *)buildPointCloudGeometryElement;

- (SCNNode *)buildPointCloudNode;
- (SCNNode *)buildPointCloudNodeWithLandmarks:(NSSet<SCLandmark3D *> * _Nullable)landmarks;

@end

NS_ASSUME_NONNULL_END
