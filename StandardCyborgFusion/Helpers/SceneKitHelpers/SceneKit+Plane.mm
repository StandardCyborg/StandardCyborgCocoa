//
//  SceneKit+Plane.mm
//  StandardCyborgData
//
//  Created by Eric Arneback on 5/21/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import "SceneKit+Plane.h"
#include <vector>

using namespace standard_cyborg;

@implementation SCNNode (StandardCyborgDataMat3x3)

// From http://jcgt.org/published/0006/01/01/
static void revisedONB(const math::Vec3 &n, math::Vec3 &b1, math::Vec3 &b2)
{
    if (n.z < 0) {
        const float a = 1.0f / (1.0f - n.z);
        const float b = n.x * n.y * a;
        b1 = math::Vec3(1.0f - n.x * n.x * a, -b, n.x);
        b2 = math::Vec3(b, n.y * n.y * a - 1.0f, -n.y);
    }
    else {
        const float a = 1.0f / (1.0f + n.z);
        const float b = -n.x * n.y * a;
        b1 = math::Vec3(1.0f - n.x * n.x * a, b, -n.x);
        b2 = math::Vec3(b, 1.0f - n.y * n.y * a, -n.y);
    }
}

+ (instancetype)nodeFromPlane:(const sc3d::Plane&)plane
                      ofWidth:(float)width
                       height:(float)height
                        color:(id _Nullable)color
                      opacity:(float)opacity

{
    SCNMaterial *material = [SCNMaterial material];
    material.diffuse.contents = color;
    material.doubleSided = YES;
    material.blendMode = SCNBlendModeAdd;
    material.writesToDepthBuffer = NO;
    
    SCNGeometry *scnGeometry = [SCNPlane planeWithWidth:width height:height];
    scnGeometry.materials = @[material];
    
    SCNNode *node = [SCNNode nodeWithGeometry:scnGeometry];
    node.opacity = 1.0f;
    node.position = SCNVector3Make(plane.position.x, plane.position.y, plane.position.z);
    node.renderingOrder = 1;
    
    math::Vec3 n;
    math::Vec3 b0;
    math::Vec3 b1;
    math::Vec3 p = plane.position;
    
    n = plane.normal;
    revisedONB(n, b0, b1);

    
    SCNMatrix4 result;
    
    result.m11 = b0.x; result.m21 = b1.x; result.m31 = n.x; result.m41 = p.x;
    result.m12 = b0.y; result.m22 = b1.y; result.m32 = n.y; result.m42 = p.y;
    result.m13 = b0.z; result.m23 = b1.z; result.m33 = n.z; result.m43 = p.z;
    result.m14 = 0; result.m24 = 0; result.m34 = 0; result.m44 = 1;
    
    node.transform = result;
    
    return node;
}

@end
