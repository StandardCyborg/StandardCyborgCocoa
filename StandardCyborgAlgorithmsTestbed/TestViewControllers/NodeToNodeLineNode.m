//
//  NodeToNodeLineNode.m
//  StandardCyborgGeometryTestbed
//
//  Created by Aaron Thompson on 4/6/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import "NodeToNodeLineNode.h"

static CGFloat SCNVector3Distance(SCNVector3 a, SCNVector3 b)
{
    CGFloat dx = a.x - b.x;
    CGFloat dy = a.y - b.y;
    CGFloat dz = a.z - b.z;
    
    return sqrt(dx * dx + dy * dy + dz * dz);
}

static SCNVector3 SCNVector3Midpoint(SCNVector3 a, SCNVector3 b)
{
    CGFloat dx = b.x - a.x;
    CGFloat dy = b.y - a.y;
    CGFloat dz = b.z - a.z;
    
    return SCNVector3Make(dx / 2.0 + a.x, dy / 2.0 + a.y, dz / 2.0 + a.z);
}

@implementation NodeToNodeLineNode {
    SCNNode *_cylinderNode;
}

- (instancetype)initWithStartNode:(SCNNode *)startNode
                          endNode:(SCNNode *)endNode
                        thickness:(CGFloat)thickness
                            color:(UIColor *)color
{
    self = [super init];
    if (self) {
        __weak SCNNode *weakStartNode = startNode;
        __weak SCNNode *weakEndNode = endNode;
        
        __weak SCNCylinder *cylinder = [SCNCylinder cylinderWithRadius:thickness height:1];
        cylinder.firstMaterial.diffuse.contents = color;
        _cylinderNode = [SCNNode nodeWithGeometry:cylinder];
        _cylinderNode.eulerAngles = SCNVector3Make(M_PI_2, 0, 0);
        [self addChildNode:_cylinderNode];
        
        SCNConstraint *positionConstraint = [SCNTransformConstraint positionConstraintInWorldSpace:NO withBlock:^SCNVector3(SCNNode *node, SCNVector3 position) {
            __strong SCNNode *strongStartNode = weakStartNode;
            __strong SCNNode *strongEndNode = weakEndNode;
            if (strongStartNode == nil || strongEndNode == nil) { return SCNVector3Zero; }
            
            // Update the cylinder's height while we're at it
            cylinder.height = SCNVector3Distance(startNode.position, endNode.position);
            
            SCNVector3 start = strongStartNode.position;
            SCNVector3 end = strongEndNode.position;
            SCNVector3 middle = SCNVector3Midpoint(start, end);
            
            return middle;
        }];
        positionConstraint.incremental = NO;
        
        SCNConstraint *lookAtConstraint = [SCNLookAtConstraint lookAtConstraintWithTarget:weakEndNode];
        lookAtConstraint.incremental = NO;
        
        [self setConstraints:@[positionConstraint, lookAtConstraint]];
    }
    return self;
}

@end
