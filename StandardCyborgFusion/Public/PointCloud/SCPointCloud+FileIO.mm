//
//  SCPointCloud+FileIO.m
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 12/19/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <ModelIO/ModelIO.h>
#import <standard_cyborg/sc3d/Landmark.hpp>
#import <standard_cyborg/scene_graph/SceneGraph.hpp>
#import <StandardCyborgFusion/PointCloudIO.hpp>
#import <StandardCyborgFusion/SCPointCloud_Private.h>
#import "WriteUSDZCompatibleZip.hpp"
#import <standard_cyborg/io/gltf/SceneGraphFileIO_GLTF.hpp>

#import "SCLandmark3D.h"
#import "SCPointCloud+FileIO.h"
#import "SCPointCloud+Geometry.h"

using namespace standard_cyborg;

@implementation SCPointCloud (FileIO)

- (instancetype)initWithPLYPath:(NSString *)PLYPath
{
    return [self initWithPLYPath:PLYPath normalizeNormals:NO];
}

- (instancetype)initWithPLYPath:(NSString *)PLYPath normalizeNormals:(BOOL)normalizeNormals
{
    Surfels surfels;
    
    if (PointCloudIO::ReadSurfelsFromPLYFile(surfels, [PLYPath UTF8String], normalizeNormals)) {
        NSData *surfelData = [NSData dataWithBytes:surfels.data() length:surfels.size() * sizeof(Surfel)];
        simd_float3 gravity = simd_make_float3(0.0f, -1.0f, 0.0f); // default gravity value.
        self = [self initWithSurfelData:surfelData gravity:gravity];
    } else {
        self = nil;
    }
    
    return self;
}

- (instancetype)initWithOBJPath:(NSString *)OBJPath
{
    Surfels surfels;
    
    if (PointCloudIO::ReadSurfelsFromOBJFile(surfels, [OBJPath UTF8String])) {
        NSData *surfelData = [NSData dataWithBytes:surfels.data() length:surfels.size() * sizeof(Surfel)];
        simd_float3 gravity = simd_make_float3(0.0f, -1.0f, 0.0f); // default gravity value.
        self = [self initWithSurfelData:surfelData gravity:gravity];
    } else {
        self = nil;
    }
    
    return self;
}


- (BOOL)writeToPLYAtPath:(NSString *)PLYPath
{
    const Surfel *surfels = (const Surfel *)[[self pointsData] bytes];
    size_t surfelCount = [self pointCount];
    
    return (BOOL)PointCloudIO::WriteSurfelsToPLYFile(surfels,
                                                     surfelCount,
                                                     Eigen::Vector3f([self gravity].x, [self gravity].y, [self gravity].z),
                                                     [PLYPath UTF8String]);
}

- (BOOL)writeToOBJAtPath:(NSString *)OBJPath
{
    const Surfel *surfels = (const Surfel *)[[self pointsData] bytes];
    size_t surfelCount = [self pointCount];
    return (BOOL)PointCloudIO::WriteSurfelsToOBJFile(surfels, surfelCount, [OBJPath UTF8String]);
}

- (BOOL)writeToUSDZAtPath:(NSString *)USDZPath
{
    BOOL success = YES;
    NSFileManager *fileManager = [NSFileManager defaultManager];
    
    NSString *basePath = [USDZPath stringByDeletingPathExtension];
    NSString *USDAPath = [basePath stringByAppendingPathExtension:@"usda"];
    NSString *USDCPath = [basePath stringByAppendingPathExtension:@"usdc"];
    NSString *texturePath = [[basePath stringByAppendingString:@"_Albedo"] stringByAppendingPathExtension:@"png"];
    
    NSURL *USDAURL = [NSURL fileURLWithPath:USDAPath];
    NSURL *USDCURL = [NSURL fileURLWithPath:USDCPath];
    
    if ([fileManager fileExistsAtPath:USDAPath]) { [fileManager removeItemAtPath:USDAPath error:NULL]; }
    if ([fileManager fileExistsAtPath:USDCPath]) { [fileManager removeItemAtPath:USDCPath error:NULL]; }
    if ([fileManager fileExistsAtPath:USDZPath]) { [fileManager removeItemAtPath:USDZPath error:NULL]; }
    
    const Surfel *surfels = (const Surfel *)[[self pointsData] bytes];
    size_t surfelCount = [self pointCount];
    
    success = PointCloudIO::WriteSurfelsToUSDAFile(surfels,
                                                   surfelCount,
                                                   std::string([USDAPath UTF8String]),
                                                   std::string([texturePath UTF8String]));
    
    @autoreleasepool {
        if (success) {
            MDLAsset *asset = [[MDLAsset alloc] initWithURL:USDAURL];
            success = [asset exportAssetToURL:USDCURL];
        }
    }
    
    if (success) {
        std::vector<std::string> inputPaths;
        inputPaths.push_back([USDCPath UTF8String]);
        
        WriteUSDZCompatibleZip([USDZPath UTF8String], inputPaths);
    }
    
    return YES;
}

- (BOOL)writeToSceneGraphGLTFAtPath:(NSString *)GLTFPath
                          landmarks:(NSSet<SCLandmark3D *> * _Nullable)landmarks
{
    sc3d::Geometry geometry;
    geometry.setNormalsEncodeSurfelRadius(true);
    [self toGeometry:geometry];
    
    std::shared_ptr<scene_graph::GeometryNode> pointCloudNode = std::make_shared<scene_graph::GeometryNode>("Point Cloud", geometry);

    for (SCLandmark3D *landmark3D in landmarks) {
        std::string name([landmark3D.label UTF8String]);
        math::Vec3 color(landmark3D.color.x, landmark3D.color.y, landmark3D.color.z);
        math::Vec3 position(landmark3D.position.x, landmark3D.position.y, landmark3D.position.z);
        
        sc3d::Landmark landmark{name, position};
        
        std::shared_ptr<scene_graph::LandmarkNode> landmarkNode(new scene_graph::LandmarkNode(name, landmark));
        landmarkNode->getMaterial().objectColor = color;
        
        pointCloudNode->appendChild(landmarkNode);
    }
    
    return (BOOL)io::gltf::WriteSceneGraphToGltf(pointCloudNode, [GLTFPath UTF8String]);
}

@end
