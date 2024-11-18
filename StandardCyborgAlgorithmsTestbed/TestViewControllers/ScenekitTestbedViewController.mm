//
//  ScenekitTestbedViewController.m
//  StandardCyborgAlgorithmsTestbed
//
//  Created by Ricky Reusser on 5/17/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <standard_cyborg/algorithms/MeshSlice.hpp>
#import <standard_cyborg/sc3d/BoundingBox3.hpp>
#import <standard_cyborg/util/DataUtils.hpp>
#import <standard_cyborg/util/DebugHelpers.hpp>
#import <standard_cyborg/sc3d/Geometry.hpp>
#import <standard_cyborg/math/Mat3x3.hpp>
#import <standard_cyborg/sc3d/Polyline.hpp>
#import <standard_cyborg/sc3d/VertexSelection.hpp>
#import <standard_cyborg/io/ply/GeometryFileIO_PLY.hpp>
#import <StandardCyborgFusion/SceneKit+BoundingBox3.h>
#import <StandardCyborgFusion/SceneKit+Geometry.h>
#import <StandardCyborgFusion/SceneKit+Lines.h>
#import <StandardCyborgFusion/SceneKit+Mat3x3.h>
#import <StandardCyborgFusion/SceneKit+Mat3x4.h>
#import <StandardCyborgFusion/SceneKit+Plane.h>
#import <StandardCyborgFusion/SceneKit+Polyline.h>

#import "NodeToNodeLineNode.h"

#import "ScenekitTestbedViewController.h"

using namespace standard_cyborg;

@implementation ScenekitTestbedViewController {
    __weak IBOutlet SCNView *_sceneView;
    SCNNode *_modelNode;
    sc3d::Geometry _geometry;
}


// MARK: - UIViewController

- (void)reloadScene
{
    NSString *testCaseName = @"TestCase-Hen";
    NSString *PLYPath = [[NSBundle mainBundle] pathForResource:@"Expected-meshed"
                                                        ofType:@"ply"
                                                   inDirectory:testCaseName];
    
    io::ply::ReadGeometryFromPLYFile(_geometry, [PLYPath UTF8String]);
    
    _modelNode = [SCNNode nodeFromGeometry:_geometry withDefaultTransform:NO];
    _modelNode.name = testCaseName;
    
    [self addNode:_modelNode];
    
    
    {
        math::Mat3x3 m = {
            1.0, 0.0, 0.0,
            0.0, 1.0, 0.0,
            0.0, 0.0, 1.0
        };
        
        SCNNode *node = [SCNNode nodeFromMat3x3:m withScale:0.1];
        node.name = @"Slice 3x3";
        [self addNode:node];
    }
    
    {
        math::Mat3x4 m = {
            -0.853553, -0.146447, -0.500002,  0.1,
             0.5,      -0.5,      -0.707103, -0.2,
            -0.146447, -0.853553,  0.500002, -0.2
        };
        
        SCNNode* node = [SCNNode nodeFromMat3x4:m withScale:0.1];
        node.name = @"Slice";
        [self addNode:node];
    }
    
    
    {
        sc3d::Plane plane;
        plane.normal = math::Vec3(-0.853553, 0.5, -0.146447);
        plane.position = math::Vec3(0.1, -0.2, -0.2);
        
        SCNNode *node = [SCNNode nodeFromPlane:plane
                                       ofWidth:0.08
                                        height:0.08
                                         color:[UIColor colorWithRed:0.4 green:0 blue:0 alpha:1]
                                       opacity:0.4];
        node.name = @"Slice";
        [self addNode:node];
    }
    
    
    {
        sc3d::Plane plane;
        plane.normal = math::Vec3(1.0, 0.0, 0.0);
        plane.position = math::Vec3(0.0, 0.0, 0.0);
        
        SCNNode *node = [SCNNode nodeFromPlane:plane
                                       ofWidth:0.08
                                        height:0.08
                                         color:[UIColor colorWithRed:0.4 green:0 blue:0 alpha:1]
                                       opacity:0.4];
        node.name = @"Slice";
        [self addNode:node];
    }
    
    sc3d::BoundingBox3 bb(_geometry);
    {
        SCNNode *node = [SCNNode nodeFromBoundingBox3:bb];
        node.name = @"bb3";
        [self addNode:node];
    }
    
    {
        std::vector<math::Vec3> vs;
        vs.push_back(bb.upper);
        vs.push_back(math::Vec3(+0.1, -0.2, -0.2));
        vs.push_back(math::Vec3(+0.0, +0.0, +0.0));
        vs.push_back(math::Vec3(+0.1, +0.2, +0.2));
        
        sc3d::Polyline polyline(vs);
        
        SCNNode *node = [SCNNode nodeFromPolyline:polyline];
        node.name = @"polyline";
        [self addNode:node];
    }
    
    {
        std::vector<sc3d::Line> lines;
        std::vector<math::Vec3> colors;
        
        math::Vec3 p(+0.1, -0.2, -0.2);
        for (int ii = 0; ii < 20; ++ii) {
            float theta = (ii / 20.0f) * 2.0 * M_PI;
            float r = 0.1;
            lines.push_back(sc3d::Line{p, p + r * math::Vec3(sin(theta), cos(theta), 0.0)});
            
            colors.push_back(ii % 2 == 0 ? math::Vec3(0.0, 1.0, 1.0) : math::Vec3(1.0, 0.0, 0.0));
        }
        
        SCNNode *node = [SCNNode nodeFromLines:lines withColors:colors];
        node.name = @"lines";
        [self addNode:node];
    }
}

// MARK: - BaseTestViewController

- (SCNView *)sceneView
{
    _sceneView.opaque = false;
    return _sceneView;
}

// MARK: - IBActions

- (IBAction)lunchMeat:(UIButton *)sender
{
}

@end
