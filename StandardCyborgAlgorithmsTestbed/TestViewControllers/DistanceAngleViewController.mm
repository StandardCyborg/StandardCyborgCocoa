//
//  DistanceAngleViewController.mm
//  StandardCyborgGeometryTestbed
//
//  Created by Aaron Thompson on 3/28/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <SceneKit/SceneKit.h>
#import <StandardCyborgFusion/SceneKit+Geometry.h>

#import "DistanceAngleViewController.h"
#import <standard_cyborg/sc3d/Geometry.hpp>
#import <standard_cyborg/io/ply/GeometryFileIO_PLY.hpp>
#import "NodeToNodeLineNode.h"

@implementation DistanceAngleViewController {
    __weak IBOutlet SCNView *sceneView;
    __weak IBOutlet UILabel *distanceLabel1_2;
    __weak IBOutlet UILabel *distanceLabel2_3;
    
    SCNNode *_modelNode;
    int _selectedPointNodeIndex;
    NSDictionary<NSString *, SCNNode *> *_UINodes;
    simd_float3 _distance1_2;
    float _distanceMagnitude1_2;
    simd_float3 _distance2_3;
    float _distanceMagnitude2_3;
}

// MARK: - UIViewController

- (void)reloadScene
{
    _modelNode = [self loadTestCaseNamed:@"TestCase-Hen" meshed:YES];
    
    NSMutableDictionary<NSString *, SCNNode *> *UINodes =
    [@{
       @"point1" : [self _makePointNodeWithName:@"point1" color:[UIColor redColor]],
       @"point2" : [self _makePointNodeWithName:@"point2" color:[UIColor blueColor]],
       @"point3" : [self _makePointNodeWithName:@"point3" color:[UIColor greenColor]],
    } mutableCopy];
    UINodes[@"distance_1_2"] = [self _makeDistanceNodeFrom:UINodes[@"point1"]
                                                        to:UINodes[@"point2"]
                                                  withName:@"distance_1_2"
                                                     color:[UIColor purpleColor]];
    UINodes[@"distance_2_3"] = [self _makeDistanceNodeFrom:UINodes[@"point2"]
                                                        to:UINodes[@"point3"]
                                                  withName:@"distance_2_3"
                                                     color:[UIColor yellowColor]];
    _UINodes = [UINodes copy];
    
    for (SCNNode *node in [_UINodes allValues]) {
        [_modelNode addChildNode:node];
        [node setHidden:YES];
    }
}

// MARK: - BaseTestViewController

- (SCNView *)sceneView
{
    return sceneView;
}

- (void)didTapNodeWithResult:(SCNHitTestResult *)result
{
    if ([result node] != _modelNode) { return; }
    
    [[self _pointNodeForIndex:_selectedPointNodeIndex] setPosition:result.localCoordinates];
    [self _updatePointMetrics];
}

// MARK: - IBActions

- (IBAction)pickPoint1:(UIButton *)sender
{
    _selectedPointNodeIndex = 1;
    [_UINodes[@"point1"] setHidden:NO];
    [self _updatePointMetrics];
}

- (IBAction)pickPoint2:(UIButton *)sender
{
    _selectedPointNodeIndex = 2;
    [_UINodes[@"point2"] setHidden:NO];
    [self _updatePointMetrics];
}

- (IBAction)pickPoint3:(UIButton *)sender
{
    _selectedPointNodeIndex = 3;
    [_UINodes[@"point3"] setHidden:NO];
    [self _updatePointMetrics];
}

- (IBAction)clearPoint1:(UIButton *)sender
{
    [_UINodes[@"point1"] setHidden:YES];
    [self _updatePointMetrics];
}

- (IBAction)clearPoint2:(UIButton *)sender
{
    [_UINodes[@"point2"] setHidden:YES];
    [self _updatePointMetrics];
}

- (IBAction)clearPoint3:(UIButton *)sender
{
    [_UINodes[@"point3"] setHidden:YES];
    [self _updatePointMetrics];
}

// MARK: - Private

- (SCNNode *)_pointNodeForIndex:(int)index
{
    NSString *nodeName = [NSString stringWithFormat:@"point%d", index];
    
    return _UINodes[nodeName];
}

- (SCNNode *)_makePointNodeWithName:(NSString *)name color:(UIColor *)color
{
    SCNNode *node = [SCNNode node];
    node.name = name;
    node.geometry = [SCNSphere sphereWithRadius:0.005];
    node.geometry.firstMaterial.diffuse.contents = color;
    
    return node;
}

- (SCNNode *)_makeDistanceNodeFrom:(SCNNode *)startNode to:(SCNNode *)endNode withName:(NSString *)name color:(UIColor *)color
{
    SCNNode *node = [[NodeToNodeLineNode alloc] initWithStartNode:startNode endNode:endNode thickness:0.003 color:color];
    node.name = name;
    
    return node;
}

- (void)_updatePointMetrics
{
    SCNNode *pointNode1 = [self _pointNodeForIndex:1];
    SCNNode *pointNode2 = [self _pointNodeForIndex:2];
    SCNNode *pointNode3 = [self _pointNodeForIndex:3];
    SCNNode *distanceNode1_2 = _UINodes[@"distance_1_2"];
    SCNNode *distanceNode2_3 = _UINodes[@"distance_2_3"];
    
    if (![pointNode1 isHidden] && ![pointNode2 isHidden]) {
        _distance1_2 = simd_make_float3(2, 4, 6);
        _distanceMagnitude1_2 = simd_length(_distance1_2);
        [distanceNode1_2 setHidden:NO];
    } else {
        _distance1_2 = simd_make_float3(0, 0, 0);
        _distanceMagnitude1_2 = 0;
        [distanceNode1_2 setHidden:YES];
    }
    
    if (![pointNode2 isHidden] && ![pointNode3 isHidden]) {
        _distance2_3 = simd_make_float3(2, 4, 6);
        _distanceMagnitude2_3 = simd_length(_distance2_3);
        [distanceNode2_3 setHidden:NO];
    } else {
        _distance2_3 = simd_make_float3(0, 0, 0);
        _distanceMagnitude2_3 = 0;
        [distanceNode2_3 setHidden:YES];
    }
    
    [distanceLabel1_2 setText:[NSString stringWithFormat:
                               @"xyz\t\t%.3f\nx\t\t%.3f\ny\t\t%.3f\nz\t\t%.3f",
                               _distanceMagnitude1_2, _distance1_2.x, _distance1_2.y, _distance1_2.z]];
    [distanceLabel2_3 setText:[NSString stringWithFormat:
                               @"xyz\t\t%.3f\nx\t\t%.3f\ny\t\t%.3f\nz\t\t%.3f",
                               _distanceMagnitude2_3, _distance2_3.x, _distance2_3.y, _distance2_3.z]];
    
    for (int i = 1; i <= 3; ++i) {
        SCNNode *pointNode = [self _pointNodeForIndex:i];
        if (i == _selectedPointNodeIndex) {
            [pointNode addAnimation:[self _selectedNodeAnimationForNode:pointNode] forKey:@"selection"];
        } else {
            [pointNode removeAnimationForKey:@"selection" blendOutDuration:1.0];
        }
    }
}

- (CABasicAnimation *)_selectedNodeAnimationForNode:(SCNNode *)node
{
    CABasicAnimation *animation = [CABasicAnimation animationWithKeyPath:@"geometry.firstMaterial.diffuse.contents"];
    animation.fromValue = node.geometry.firstMaterial.diffuse.contents;
    animation.toValue = [UIColor whiteColor];
    animation.duration = 0.5;
    animation.autoreverses = YES;
    animation.repeatCount = INFINITY;
    animation.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseIn];
    
    return animation;
}

@end
