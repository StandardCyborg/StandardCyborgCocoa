//
//  PlaneRemovalViewController.mm
//  StandardCyborgGeometryTestbed
//
//  Created by Ricky Reusser on 4/10/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <standard_cyborg/algorithms/EstimatePlane.hpp>
#import <standard_cyborg/util/DataUtils.hpp>
#import <standard_cyborg/util/DebugHelpers.hpp>
#import <standard_cyborg/sc3d/Geometry.hpp>
#import <standard_cyborg/util/IncludeEigen.hpp>
#import <standard_cyborg/sc3d/VertexSelection.hpp>
#import <standard_cyborg/io/ply/GeometryFileIO_PLY.hpp>
#import <StandardCyborgFusion/SceneKit+Geometry.h>


#import <iostream>

#import "PlaneRemovalViewController.h"

#import "NodeToNodeLineNode.h"


using namespace standard_cyborg;

@implementation PlaneRemovalViewController {
    __weak IBOutlet SCNView *sceneView;
    sc3d::Geometry _sampleGeometry;
    Eigen::Vector3f _planeNormalInitialSeed;
    
    SCNNode *_theNode;
}

- (void)reloadScene
{
    NSString *PLYPath = [[NSBundle mainBundle] pathForResource:@"foot"
                                                        ofType:@"ply"
                                                   inDirectory:@"Scans"];
    io::ply::ReadGeometryFromPLYFile(_sampleGeometry, [PLYPath UTF8String]);
    
    _sampleGeometry.normalizeNormals();

    _sampleGeometry.mutateColorsWithFunction([](const int index, const math::Vec3 position, const math::Vec3 normal, const math::Vec3 color) {
        Eigen::Vector3f c {
            normal.x * normal.x,
            normal.y * normal.y,
            normal.z * normal.z
        };
        c.normalize();
        return toVec3(c);
    });
    
    _planeNormalInitialSeed = Eigen::Vector3f {-0.3f, 0.7f, 0.5f};
    _planeNormalInitialSeed.normalize();
    
    auto selection = [self _selectVerticesOfGeometry:_sampleGeometry byNormal:_planeNormalInitialSeed withDegreeTolerance:20.0f];
    
    NSLog(@"selection size = %d", selection->size());

    std::unique_ptr<sc3d::Geometry> geoWithColor = std::make_unique<sc3d::Geometry> ();
    geoWithColor->copy(_sampleGeometry);
    geoWithColor->mutateColorsWithFunction([](const int index, const math::Vec3 position, const math::Vec3 normal, const math::Vec3 color) {
        return math::Vec3 {1.0f, 0.0f, 0.0f};
    }, *selection);

    _theNode = [SCNNode nodeFromGeometry:*geoWithColor withDefaultTransform:NO];
    _theNode.name = @"THE HEN, THE HEN";
    [self addNode:_theNode];
    
    SCNNode *origin = [self _makePointNodeWithName:@"origin" color:[UIColor redColor]];
    SCNNode *xaxis = [self _makePointNodeWithName:@"xaxis" color:[UIColor redColor]];
    SCNNode *yaxis = [self _makePointNodeWithName:@"yaxis" color:[UIColor greenColor]];
    SCNNode *zaxis = [self _makePointNodeWithName:@"zaxis" color:[UIColor blueColor]];
    
    SCNNode *normal = [self _makePointNodeWithName:@"normal" color:[UIColor purpleColor]];
    
    [origin setPosition:SCNVector3Make(0.0f, 0.0f, 0.0f)];
    [xaxis setPosition:SCNVector3Make(1.0f, 0.0f, 0.0f)];
    [yaxis setPosition:SCNVector3Make(0.0f, 1.0f, 0.0f)];
    [zaxis setPosition:SCNVector3Make(0.0f, 0.0f, 1.0f)];
    [normal setPosition:SCNVector3Make(_planeNormalInitialSeed[0], _planeNormalInitialSeed[1], _planeNormalInitialSeed[2])];
    
    SCNNode *xaxisVector = [[NodeToNodeLineNode alloc] initWithStartNode:origin endNode:xaxis thickness:0.003 color:[UIColor redColor]];
    SCNNode *yaxisVector = [[NodeToNodeLineNode alloc] initWithStartNode:origin endNode:yaxis thickness:0.003 color:[UIColor greenColor]];
    SCNNode *zaxisVector = [[NodeToNodeLineNode alloc] initWithStartNode:origin endNode:zaxis thickness:0.003 color:[UIColor blueColor]];
    SCNNode *normalVector = [[NodeToNodeLineNode alloc] initWithStartNode:origin endNode:normal thickness:0.003 color:[UIColor purpleColor]];
    
    [self addNode:origin];
    
    [self addNode:xaxis];
    [self addNode:xaxisVector];
    
    [self addNode:yaxis];
    [self addNode:yaxisVector];
    
    [self addNode:zaxis];
    [self addNode:zaxisVector];
    
    [self addNode:normal];
    [self addNode:normalVector];
}

// MARK: - BaseTestViewController

- (SCNView *)sceneView
{
    return sceneView;
}

// MARK: - IBActions

- (IBAction)selectPlane:(UIButton *)sender
{
    auto planeVerticesGuess = [self _selectVerticesOfGeometry:_sampleGeometry
                                                    byNormal:_planeNormalInitialSeed
                                         withDegreeTolerance:20.0f];
    
    algorithms::EstimatePlaneResult result = algorithms::estimatePlane(_sampleGeometry.getPositions(), *planeVerticesGuess);
    
    sc3d::Plane plane = result.plane;
    
    auto planeVertices = sc3d::VertexSelection::fromGeometryVertices(_sampleGeometry, [&plane](int index, math::Vec3 p, math::Vec3 n, math::Vec3 c) {
        float dx = p.x - plane.position.x;
        float dy = p.y - plane.position.y;
        float dz = p.z - plane.position.z;
        float projectedDistance = dx * plane.normal.x + dy * plane.normal.y + dz * plane.normal.z;
        
        return projectedDistance < 0.005 && projectedDistance > -0.03;
    });
    
    _sampleGeometry.deleteVertices(*planeVertices);
    
    NSLog(@"sampleGeometry post-delete size: %d", _sampleGeometry.vertexCount());
    
    [_theNode removeFromParentNode];
    _theNode = [SCNNode nodeFromGeometry:_sampleGeometry withDefaultTransform:NO];
    _theNode.name = @"THE HEN, THE HEN";
    [self addNode:_theNode];

    //std::cout<<"estimated plane = "<<result.plane<<std::endl;
}

// MARK: - Private

- (std::unique_ptr<sc3d::VertexSelection>)_selectVerticesOfGeometry:(sc3d::Geometry&)geometry byNormal:(Eigen::Vector3f)referenceNormal withDegreeTolerance:(float)toleranceDegrees
{
    float cosAngleThreshold = std::cos(toleranceDegrees * M_PI / 180.0);
    Eigen::Vector3f normalSeed = referenceNormal;
    return sc3d::VertexSelection::fromGeometryVertices(geometry, [cosAngleThreshold, &normalSeed](int index, math::Vec3 position, math::Vec3 normal, math::Vec3 color) {
        float norm = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
        float dot = normal.x * normalSeed.x() + normal.y * normalSeed.y() + normal.z * normalSeed.z();
        return dot / norm > cosAngleThreshold;
    });
}

- (SCNNode *)_makePointNodeWithName:(NSString *)name color:(UIColor *)color
{
    SCNNode *node = [SCNNode node];
    node.name = name;
    node.geometry = [SCNSphere sphereWithRadius:0.005];
    node.geometry.firstMaterial.diffuse.contents = color;
    
    return node;
}

@end
