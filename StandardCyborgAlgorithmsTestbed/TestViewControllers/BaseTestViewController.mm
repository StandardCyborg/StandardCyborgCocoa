//
//  BaseTestViewController.m
//  StandardCyborgGeometryTestbed
//
//  Created by Aaron Thompson on 4/5/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <SceneKit/SceneKit.h>
#import <standard_cyborg/sc3d/Geometry.hpp>
#import <standard_cyborg/io/ply/GeometryFileIO_PLY.hpp>
#import <StandardCyborgFusion/SceneKit+Geometry.h>
#import <StandardCyborgFusion/StandardCyborgFusion.h>


#import "BaseTestViewController.h"

@implementation BaseTestViewController {
    SCNNode *_containerNode;
}

// MARK: - Public

- (SCNNode *)loadTestCaseNamed:(NSString *)testCaseName meshed:(BOOL)meshed
{
    NSString *PLYPath = [[NSBundle mainBundle] pathForResource:meshed ? @"Expected-meshed" : @"Expected"
                                                        ofType:@"ply"
                                                   inDirectory:testCaseName];
    
    sc3d::Geometry geometry;
    io::ply::ReadGeometryFromPLYFile(geometry, [PLYPath UTF8String]);
    
    SCNNode *node = [SCNNode nodeFromGeometry:geometry withDefaultTransform:YES];
    node.name = testCaseName;
    
    [_containerNode addChildNode:node];
    
    return node;
}

- (void)addNode:(SCNNode *)node
{
    if ([node parentNode] == nil) {
        [_containerNode addChildNode:node];
    }
}

- (void)removeNode:(SCNNode *)node
{
    [node removeFromParentNode];
}

// MARK: - Required Subclass Overrides

- (SCNView *)sceneView
{
    [NSException raise:NSGenericException
                format:@"Subclasses of BaseTestViewController must override -sceneView and return an SCNView instance"];
    return nil;
}

// MARK: - Optional Subclass Overrides

- (void)reloadScene
{
    for (SCNNode *child in [[_containerNode childNodes] copy]) {
        [child removeFromParentNode];
    }
}

- (void)didTapNodeWithResult:(SCNHitTestResult *)result { }

// MARK: - UIViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    UITapGestureRecognizer *recognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(_sceneViewTapped:)];
    [[self sceneView] addGestureRecognizer:recognizer];
    
    [self _reloadScene];
}

// MARK: - Private

- (void)_reloadScene
{
    [_containerNode removeFromParentNode];
    _containerNode = [SCNNode node];
    _containerNode.name = @"container";
    [[[[self sceneView] scene] rootNode] addChildNode:_containerNode];
    
    [self reloadScene];
}

- (void)_sceneViewTapped:(UITapGestureRecognizer *)sender
{
    CGPoint tappedPoint = [sender locationInView:[self sceneView]];
    NSDictionary *hitTestOptions =
    @{
      SCNHitTestOptionSearchMode: @(SCNHitTestSearchModeAll)
      };
    
    for (SCNHitTestResult *result in [[self sceneView] hitTest:tappedPoint options:hitTestOptions]) {
        NSLog(@"Tapped geometry for node %@ at %f, %f, %f", result.node.name,
              result.localCoordinates.x, result.localCoordinates.y, result.localCoordinates.z);
        [self didTapNodeWithResult:result];
    }
}

@end
