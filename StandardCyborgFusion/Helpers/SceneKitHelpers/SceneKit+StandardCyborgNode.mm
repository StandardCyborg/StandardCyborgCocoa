//
//  SCNNode+StandardCyborgNode.mm
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 8/6/19.
//  Copyright © 2019 StandardCyborg. All rights reserved.
//

#import <standard_cyborg/sc3d/Landmark.hpp>

#import "SceneKit+Geometry.h"
#import "SceneKit+Plane.h"
#import "SceneKit+Polyline.h"
#import "SceneKit+StandardCyborgNode.h"

using namespace std;

using namespace standard_cyborg;
namespace sg = standard_cyborg::scene_graph;

static simd_float4x4 simd_float4x4FromMat3x4(math::Mat3x4 mat)
{
    return (simd_float4x4){
        .columns[0] = {mat.m00, mat.m10, mat.m20, 0},
        .columns[1] = {mat.m01, mat.m11, mat.m21, 0},
        .columns[2] = {mat.m02, mat.m12, mat.m22, 0},
        .columns[3] = {mat.m03, mat.m13, mat.m23, 1}
    };
}

@implementation SCNNode (StandardCyborgNode)

+ (SCNNode *)nodeFromStandardCyborgNode:(shared_ptr<sg::Node>)node withDefaultTransform:(BOOL)useDefaultTransform
{
    if (node == nullptr) { return nil; }
    
    SCNNode *resultNode = nil;
    
    CGColorRef nodeColor = NULL;
    math::Vec3 materialColor = node->getMaterial().objectColor;
    if (materialColor != math::Vec3(0)) {
        CGFloat colorComponents[] = {materialColor.x, materialColor.y, materialColor.z, 1};
        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        nodeColor = CGColorCreate(colorSpace, colorComponents);
        CGColorSpaceRelease(colorSpace);
    }
    
    switch (node->getType()) {
        case sg::SGNodeType::Generic:
            resultNode = [SCNNode node];
            break;
            
        case sg::SGNodeType::Geometry: {
            const sc3d::Geometry &geometry = node->asGeometryNode()->getGeometry();
            resultNode = [SCNNode nodeFromGeometry:geometry withDefaultTransform:NO];
            break;
        }
            
        case sg::SGNodeType::Landmark:
        case sg::SGNodeType::Point:
            resultNode = [self _landmarkNodeWithColor:(__bridge id)nodeColor];
            break;
            
        case sg::SGNodeType::PerspectiveCamera:
            NSLog(@"Error: SceneKit node bridge for SGNodeType::PerspectiveCamera is not yet implemented");
            break;
            
        case sg::SGNodeType::Plane: {
            sg::PlaneNode *planeNode = node->asPlaneNode();
            sc3d::Plane plane = planeNode->getPlane();
            resultNode = [SCNNode nodeFromPlane:plane
                                        ofWidth:planeNode->getExtents().x
                                         height:planeNode->getExtents().y
                                          color:(__bridge id)nodeColor
                                        opacity:1];
            break;
        }
        
        case sg::SGNodeType::Polyline: {
            const sc3d::Polyline &polyline = node->asPolylineNode()->getPolyline();
            resultNode = [SCNNode nodeFromPolyline:polyline];
            resultNode.geometry.firstMaterial.diffuse.contents = (__bridge id)nodeColor;
            break;
        }
            
        case sg::SGNodeType::ColorImage:
            NSLog(@"Error: SceneKit node bridge for SGNodeType::ColorImage is not yet implemented");
            break;
        case sg::SGNodeType::CoordinateFrame:
            NSLog(@"Error: SceneKit node bridge for SGNodeType::CoordinateFrame is not yet implemented");
            break;
        case sg::SGNodeType::DepthImage:
            NSLog(@"Error: SceneKit node bridge for SGNodeType::DepthImage is not yet implemented");
            break;
    }
    
    resultNode.name = [NSString stringWithUTF8String:node->getName().c_str()];
    resultNode.simdTransform = simd_float4x4FromMat3x4(math::Mat3x4::fromTransform(node->getTransform()));
    
    for (auto child : node) {
        SCNNode *childNode = [self nodeFromStandardCyborgNode:child withDefaultTransform:NO];
        
        if (childNode != nil) {
            [resultNode addChildNode:childNode];
        }
    }
    
    if (useDefaultTransform) {
        [resultNode _applyDefaultTransform];
    }
    
    if (nodeColor != NULL) { CGColorRelease(nodeColor); }
    
    return resultNode;
}

- (void)_applyDefaultTransform
{
    SCNVector3 center;
    double boundingSphereRadius;
    [self getBoundingSphereCenter:&center radius:&boundingSphereRadius];
    
    if (boundingSphereRadius > 0) {
        /* TRANSFORM NOTES!
         - We scale the node to fit based on its bounding radius, increased by a small factor to look just right.
         - The order of operations in the node's transform seems to be rotation, translation, scale.
         - Therefore, it is necessary to pre-apply scale to the position.
         */
        double scale = 0.3 / boundingSphereRadius;
        
        self.position = SCNVector3Make(+center.y * scale,
                                       -center.x * scale,
                                       -center.z * scale);
        self.scale = SCNVector3Make(scale, scale, scale);
    }
}

+ (SCNNode *)_landmarkNodeWithColor:(id)color
{
    SCNNode *node = [SCNNode node];
    node.geometry = [SCNSphere sphereWithRadius:0.0045];
    node.geometry.firstMaterial.diffuse.contents = color;
    
    return node;
}

@end
