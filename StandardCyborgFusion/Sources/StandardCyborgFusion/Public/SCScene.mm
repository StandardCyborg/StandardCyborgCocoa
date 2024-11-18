//  SCScene.mm
//
//  Created by Aaron Thompson on 12/30/19.
//

#import <standard_cyborg/scene_graph/SceneGraph.hpp>
#import <standard_cyborg/io/gltf/SceneGraphFileIO_GLTF.hpp>
#import <fstream>
#import <iostream>
#import <memory>

#import "GeometryHelpers.hpp"
#import "SCMesh+Geometry.h"
#import "SCPointCloud+Geometry.h"
#import "SceneKit+StandardCyborgNode.h"

#import "SCScene.h"

using namespace standard_cyborg;
namespace sg = standard_cyborg::scene_graph;

@implementation SCScene {
    std::shared_ptr<sg::Node> _scene;
}

+ (void)_readContentsOfPath:(NSString *)path intoString:(std::string&)stringOut
{
    std::ifstream inputStream([path UTF8String]);
    
    inputStream.seekg(0, std::ios::end);
    stringOut.reserve(inputStream.tellg());
    inputStream.seekg(0, std::ios::beg);
    
    stringOut.assign(std::istreambuf_iterator<char>(inputStream),
                     std::istreambuf_iterator<char>());
}

- (instancetype)initWithPointCloud:(SCPointCloud *)pointCloud mesh:(SCMesh *)mesh
{
    return [self initWithPointCloud:pointCloud mesh:mesh transform:matrix_identity_float4x4];
}

- (instancetype)initWithPointCloud:(SCPointCloud *)pointCloud
                              mesh:(SCMesh *)mesh
                         transform:(simd_float4x4)transform
{
    self = [super init];
    if (self) {
        _pointCloud = pointCloud;
        _mesh = mesh;
        _scene = std::shared_ptr<sg::Node>(new sg::Node("SCScene"));
        
        if (pointCloud != nil) {
            auto pointCloudNode = std::shared_ptr<sg::GeometryNode>(new sg::GeometryNode("SCPointCloud"));
            [pointCloud toGeometry:pointCloudNode->getGeometry()];
            
            if (!simd_equal(transform, matrix_identity_float4x4)) {
                pointCloudNode->getGeometry().transform(toMat3x4(transform));
            }
            
            _scene->appendChild(pointCloudNode);
        }
        
        if (mesh != nil) {
            auto meshNode = std::shared_ptr<sg::GeometryNode>(new sg::GeometryNode("SCMesh"));
            [mesh toGeometry:meshNode->getGeometry()];
            
            if (!simd_equal(transform, matrix_identity_float4x4)) {
                meshNode->getGeometry().transform(toMat3x4(transform));
            }
            
            _scene->appendChild(meshNode);
        }
        
        _rootNode = [SCNNode nodeFromStandardCyborgNode:_scene withDefaultTransform:NO];
    }
    return self;
}

- (instancetype)initWithGLTFAtPath:(NSString *)GLTFPath
{
    self = [super init];
    if (self) {
        std::string gltfContents;
        [[self class] _readContentsOfPath:GLTFPath intoString:gltfContents];
        
        auto sceneGraph = io::gltf::ReadSceneGraphFromGltf(gltfContents);
        
        if (sceneGraph.size() == 1) {
            _scene = sceneGraph[0];
        } else {
            _scene.reset(new sg::Node("SCScene"));
        }
        
        _rootNode = [SCNNode nodeFromStandardCyborgNode:_scene withDefaultTransform:NO];
    }
    return self;
}

- (void)writeToGLTFAtPath:(NSString *)GLTFPath
{
    io::gltf::WriteSceneGraphToGltf(_scene, [GLTFPath UTF8String]);
}

@end
