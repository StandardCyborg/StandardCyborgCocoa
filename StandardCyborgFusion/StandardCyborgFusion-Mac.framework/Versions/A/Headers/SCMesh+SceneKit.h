//
//  SCMesh+SceneKit.h
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 10/19/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <StandardCyborgFusion/SCMesh.h>

@class SCNGeometryElement;
@class SCNGeometrySource;
@class SCNNode;
@class SCLandmark3D;

NS_ASSUME_NONNULL_BEGIN

@interface SCMesh (SceneKit)

- (SCNGeometrySource *)buildVertexGeometrySource;
- (SCNGeometrySource *)buildNormalGeometrySource;
- (SCNGeometrySource *)buildTexCoordGeometrySource;
- (SCNGeometryElement *)buildMeshGeometryElement;
- (SCNNode *)buildMeshNode NS_SWIFT_NAME(buildMeshNode());

@end

NS_ASSUME_NONNULL_END
