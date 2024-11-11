//
//  SCPointCloud+SceneKit.m
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 12/17/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <CoreGraphics/CoreGraphics.h>
#import <SceneKit/SceneKit.h>
#import <StandardCyborgFusion/Surfel.hpp>

#import "SCLandmark3D.h"
#import "SCPointCloud+SceneKit.h"
#import "SCPointCloud_Private.h"

@implementation SCPointCloud (SceneKit)

- (SCNGeometrySource *)buildVertexGeometrySource
{
    return [SCNGeometrySource geometrySourceWithData:self.pointsData
                                            semantic:SCNGeometrySourceSemanticVertex
                                         vectorCount:self.pointCount
                                     floatComponents:YES
                                 componentsPerVector:3
                                   bytesPerComponent:sizeof(float)
                                          dataOffset:offsetof(Surfel, position)
                                          dataStride:sizeof(Surfel)];
}

- (SCNGeometrySource *)buildNormalGeometrySource
{
    return [SCNGeometrySource geometrySourceWithData:self.pointsData
                                            semantic:SCNGeometrySourceSemanticNormal
                                         vectorCount:self.pointCount
                                     floatComponents:YES
                                 componentsPerVector:3
                                   bytesPerComponent:sizeof(float)
                                          dataOffset:offsetof(Surfel, normal)
                                          dataStride:sizeof(Surfel)];
}

- (SCNGeometrySource *)buildColorGeometrySource
{
    return [SCNGeometrySource geometrySourceWithData:self.pointsData
                                            semantic:SCNGeometrySourceSemanticColor
                                         vectorCount:self.pointCount
                                     floatComponents:YES
                                 componentsPerVector:3
                                   bytesPerComponent:sizeof(float)
                                          dataOffset:offsetof(Surfel, color)
                                          dataStride:sizeof(Surfel)];
}

- (SCNGeometryElement *)buildPointCloudGeometryElement
{
    SCNGeometryElement *element = [SCNGeometryElement geometryElementWithData:nil
                                                                primitiveType:SCNGeometryPrimitiveTypePoint
                                                               primitiveCount:self.pointCount
                                                                bytesPerIndex:sizeof(NSInteger)];
    element.minimumPointScreenSpaceRadius = 3;
    element.maximumPointScreenSpaceRadius = 5;
    element.pointSize = 4;
    
    return element;
}

- (SCNNode *)buildPointCloudNode
{
    return [self buildPointCloudNodeWithLandmarks:nil];
}

- (SCNNode *)buildPointCloudNodeWithLandmarks:(NSSet<SCLandmark3D *> *)landmarks
{
    SCNGeometrySource *positionSource = [self buildVertexGeometrySource];
    SCNGeometrySource *normalSource = [self buildNormalGeometrySource];
    SCNGeometrySource *colorSource = [self buildColorGeometrySource];
    SCNGeometryElement *element = [self buildPointCloudGeometryElement];
    
    SCNGeometry *geometry = [SCNGeometry geometryWithSources:@[positionSource, normalSource, colorSource]
                                                    elements:@[element]];
    
    SCNNode *node = [SCNNode nodeWithGeometry:geometry];

    double boundingSphereRadius;
    [node getBoundingSphereCenter:NULL radius:&boundingSphereRadius];
    simd_float3 center = [self centerOfMass];
    
    if (boundingSphereRadius > 0) {
        /* TRANSFORM NOTES!
         - We scale the node to fit based on its bounding radius, increased by a small factor to look just right.
         - The model needs to be rotated 90º about Z for it to appear correctly in SceneKit
         - The order of operations in the node's transform seems to be rotation, translation, scale.
           Therefore, it is necessary to pre-apply scale to the position.
           Also, after rotating 90º about z, (-x,-y,-z) becomes (y,-x,-z).
         */
        double scale = 0.3 / boundingSphereRadius;
        
        node.position = SCNVector3Make( center.y / scale,
                                       -center.x / scale,
                                       -center.z / scale);
        node.scale = SCNVector3Make(scale, scale, scale);
    }
    
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    
    for (SCLandmark3D *landmark in landmarks) {
        NSString *nodeName = [NSString stringWithFormat:@"Landmark %@", landmark.label];
        
        SCNVector3 position;
        position.x = landmark.position.x;
        position.y = landmark.position.y;
        position.z = landmark.position.z;
        
        CGFloat colorArray[4] = {
            landmark.color.x,
            landmark.color.y,
            landmark.color.z,
            1
        };
        CGColorRef color = CGColorCreate(colorSpace, colorArray);
        
        SCNNode *landmarkNode = [self _makePointNodeWithName:nodeName color:color];
        landmarkNode.position = position;
        landmarkNode.scale = SCNVector3Make(1.0 / node.scale.x, 1.0 / node.scale.y, 1.0 / node.scale.z);
        [node addChildNode:landmarkNode];
        
        CGColorRelease(color);
    }
    
    CGColorSpaceRelease(colorSpace);
    
    return node;
}

- (SCNNode *)_makePointNodeWithName:(NSString *)name color:(CGColorRef)color
{
    SCNNode *node = [SCNNode node];
    node.name = name;
    node.geometry = [SCNSphere sphereWithRadius:0.0025];
    node.geometry.firstMaterial.diffuse.contents = (__bridge id)color;
    
    return node;
}

@end
