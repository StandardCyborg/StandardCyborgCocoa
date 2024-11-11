//
//  EarToEarTests.m
//  StandardCyborgFusionTests
//
//  Created by eric on 2019-04-26.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <XCTest/XCTest.h>
#import <cmath>
#import <iostream>

#import <standard_cyborg/util/DataUtils.hpp>
#import <standard_cyborg/util/DebugHelpers.hpp>
#import <standard_cyborg/sc3d/Face3.hpp>
#import <standard_cyborg/sc3d/Geometry.hpp>
#import <standard_cyborg/util/IncludeEigen.hpp>
#import <standard_cyborg/math/Vec3.hpp>
#import <standard_cyborg/sc3d/VertexSelection.hpp>
#import <StandardCyborgFusion/MetalDepthProcessor.hpp>
#import <StandardCyborgFusion/PBFModel.hpp>
#import <StandardCyborgFusion/PointCloudIO.hpp>
#import <StandardCyborgFusion/StandardCyborgFusion.h>
#import <standard_cyborg/io/ply/GeometryFileIO_PLY.hpp>

#import "Helpers/PathHelpers.h"
#import "Helpers/ReconstructionHelpers.h"


using namespace standard_cyborg;

void AveragePosNormal(const sc3d::Geometry& geo,
                      const sc3d::VertexSelection& selection,
                      math::Vec3& averagePos,
                      math::Vec3& averageNormal)
{
    const std::vector<math::Vec3> positions = geo.getPositions();
    const std::vector<math::Vec3> normals = geo.getNormals();
    
    math::Vec3 posSum(0,0,0);
    math::Vec3 normalSum(0,0,0);
    
    for (auto index : selection) {
        posSum += math::Vec3(positions[index].x, positions[index].y, positions[index].z);;
        math::Vec3 n(normals[index].x, normals[index].y, normals[index].z);
        normalSum += n.normalize();
    }
    
    posSum = (float)(1.0f / selection.size()) * posSum;
    normalSum = (float)(1.0f / selection.size()) * normalSum;
    normalSum = normalSum.normalize();
    
    averagePos = math::Vec3{posSum.x, posSum.y, posSum.z};
    averageNormal = math::Vec3{normalSum.x, normalSum.y, normalSum.z};
    
}

// https://stackoverflow.com/questions/39680320/printing-debugging-libc-stl-with-xcode-lldb
template struct std::vector<Eigen::VectorXf>;

@interface EarToEarTests : XCTestCase

@end

@implementation EarToEarTests

- (void)testPBF {
    ICPConfiguration icpConfig;
    
    PBFConfiguration pbfConfig;
    SurfelFusionConfiguration surfelFusionConfig;
    
    surfelFusionConfig.maxDepth = 0.75;
    surfelFusionConfig.minDepth = 0.0;
    pbfConfig.icpDownsampleFraction = 0.1f;
    
    NSArray *testCases = [NSArray arrayWithObjects:@"sven-ear-to-ear-lo-res", nil];
    
    for (size_t iTest = 0; iTest < [testCases count]; ++iTest) {
        NSString *testCase = testCases[iTest];
        
        // get the test case paths from the bundle.
        std::string expectedPLYPath;
        NSString *depthFramesDir;
        {
            NSString *testCasePath = [[PathHelpers testCasesPath] stringByAppendingPathComponent:testCase];
            depthFramesDir = [testCasePath stringByAppendingPathComponent:@"DepthFrames"];
            NSString *expectedPLYPathNS = [testCasePath stringByAppendingPathComponent:@"Expected.ply"];
            expectedPLYPath = std::string([expectedPLYPathNS UTF8String]);
        }
        
        std::unique_ptr<PBFModel> pbf(assimilatePointCloud(depthFramesDir, icpConfig, pbfConfig, surfelFusionConfig));
        Surfels surfels = pbf->getSurfels();
        
        std::vector<math::Vec3> positions;
        std::vector<math::Vec3> normals;
        std::vector<math::Vec3> colors;
        
        for (int ii = 0; ii < surfels.size(); ++ii) {
            Surfel surfel = surfels[ii];
            
            positions.push_back(toVec3(surfel.position));
            normals.push_back(toVec3(surfel.normal));
            colors.push_back(toVec3(surfel.color));
        }
        
        sc3d::Geometry geo(positions, normals, colors);
        
        sc3d::VertexSelection nonPink;
        
        for (int index = 0; index < (int)surfels.size(); ++index) {
            Surfel surfel = surfels[index];
            Eigen::Vector3f pinkColor(0.9658, 0.05818717747, 0.31576274373);
            
            if ((surfel.color - pinkColor).norm() > 0.20)
            {
                nonPink.insertValue((int)index);
            }
        }
        
        geo.deleteVertices(nonPink);
        
        std::vector<std::unique_ptr<sc3d::VertexSelection>> partitions; // one partition for each 'x'.
        std::set<int> traversed; // contains vertices that have been added to a partition.
        
        while (partitions.size() < 3) {
            // find seed vertex of partition.
            int iseed = 0;
            for (int ii = 0; ii < geo.vertexCount(); ++ii) {
                if (traversed.count(ii) == 0) {
                    iseed = ii;
                    break;
                }
            }
            
            std::unique_ptr<sc3d::VertexSelection> partition;
            partition.reset(new sc3d::VertexSelection());
            
            math::Vec3 vseed = geo.vertexCount() > 0 ? geo.getPositions()[iseed] : math::Vec3(0, 0, 0);
            
            for (int ii = 0; ii < geo.vertexCount(); ++ii) {
                if (traversed.count(ii) != 0) {
                    continue; // already traversed.
                }
                
                math::Vec3 p = geo.getPositions()[ii];
                math::Vec3 diff = p - vseed;
                
                if (diff.norm() < 0.06) {
                    traversed.insert(ii);
                    partition->insertValue(ii);
                }
            }
            
            partitions.push_back(std::move(partition));
        }
        
        math::Vec3 p0;
        math::Vec3 n0;
        AveragePosNormal(geo, *partitions[0], p0, n0);
        
        math::Vec3 p1;
        math::Vec3 n1;
        AveragePosNormal(geo, *partitions[1], p1, n1);
        
        math::Vec3 p2;
        math::Vec3 n2;
        AveragePosNormal(geo, *partitions[2], p2, n2);
        
        float d01 = math::Vec3::dot(n0, n1);
        float d02 = math::Vec3::dot(n0, n2);
        float d12 = math::Vec3::dot(n1, n2);
        
        float earToEatDist = 0.0f;
        
        // if the normals point away from each other a lot, they probably
        // belong to the x:es, on the left and right ears. Since their
        // normals point away from each other.
        if (d02 < d01 && d02 < d12) {
            earToEatDist = (p0 - p2).norm();
        }
        else if (d01 < d02 && d01 < d12) {
            earToEatDist = (p0 - p1).norm();
        }
        else if (d12 < d01 && d12 < d02) {
            earToEatDist = (p0 - p1).norm();
        }
        
        // TODO: what kind of test do we put here?!?!
        printf("ear-to-ear distance: %f\n", earToEatDist);
    }
    
}

@end
