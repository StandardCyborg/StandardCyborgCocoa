//
//  SparseSurfelLandmarksIndexTest.mm
//  StandardCyborgFusionTests
//
//  Created by Ricky Reusser on 4/22/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <XCTest/XCTest.h>

#import <StandardCyborgFusion/StandardCyborgFusion.h>
#include <standard_cyborg/util/IncludeEigen.hpp>
#include <StandardCyborgFusion/MetalSurfelIndexMap.hpp>
#include <StandardCyborgFusion/PointCloudIO.hpp>
#include <StandardCyborgFusion/SparseSurfelLandmarksIndex.hpp>
#include <StandardCyborgFusion/MetalDepthProcessor.hpp>
#include <StandardCyborgFusion/PBFModel.hpp>
#include "Helpers/PathHelpers.h"
#include "Helpers/ReconstructionHelpers.h"
#include <iostream>

@interface SparseSurfelLandmarksTest : XCTestCase
@end

@implementation SparseSurfelLandmarksTest

- (void)testAddHit {
    SparseSurfelLandmarksIndex index;
    
    index.addHit(17, 2);

    XCTAssertEqual(index.getHitCount(17, 2), 1);
    XCTAssertEqual(index.getHitCount(16, 2), 0);
    XCTAssertEqual(index.getHitCount(17, 3), 0);
    
    index.addHit(17, 2);
    
    XCTAssertEqual(index.getHitCount(17, 2), 2);
    XCTAssertEqual(index.getHitCount(16, 2), 0);
    XCTAssertEqual(index.getHitCount(17, 3), 0);
    
    index.addHit(17, 2);
    
    XCTAssertEqual(index.getHitCount(17, 2), 3);
    XCTAssertEqual(index.getHitCount(16, 2), 0);
    XCTAssertEqual(index.getHitCount(17, 3), 0);
    
    index.addHit(17, 3);
    
    XCTAssertEqual(index.getHitCount(17, 2), 3);
    XCTAssertEqual(index.getHitCount(16, 2), 0);
    XCTAssertEqual(index.getHitCount(17, 3), 1);
}

- (void)testRemoveHit {
    SparseSurfelLandmarksIndex index;
    
    index.addHit(17, 2);
    index.addHit(17, 2);
    index.addHit(17, 2);
    
    XCTAssertTrue(index.removeHit(17, 2));

    XCTAssertEqual(index.getHitCount(17, 2), 2);
    XCTAssertEqual(index.getHitCount(16, 2), 0);
    XCTAssertEqual(index.getHitCount(17, 3), 0);
    
    XCTAssertTrue(index.removeHit(17, 2));
    
    XCTAssertEqual(index.getHitCount(17, 2), 1);
    XCTAssertEqual(index.getHitCount(16, 2), 0);
    XCTAssertEqual(index.getHitCount(17, 3), 0);
    
    XCTAssertTrue(index.removeHit(17, 2));
    
    XCTAssertEqual(index.getHitCount(17, 2), 0);
    XCTAssertEqual(index.getHitCount(16, 2), 0);
    XCTAssertEqual(index.getHitCount(17, 3), 0);
    
    XCTAssertFalse(index.removeHit(17, 2));
    
    XCTAssertEqual(index.getHitCount(17, 2), 0);
    XCTAssertEqual(index.getHitCount(16, 2), 0);
    XCTAssertEqual(index.getHitCount(17, 3), 0);
}

- (void)testSize {
    SparseSurfelLandmarksIndex index;
    index.addHit(17, 3);
    index.addHit(17, 4);
    index.addHit(15, 4);
    
    XCTAssertEqual(index.size(), 2);
}

- (void)testDeleteVertices {
    SparseSurfelLandmarksIndex index;
    
    index.addHit(1, 1);
    index.addHit(2, 1);
    index.addHit(4, 1);
    index.addHit(4, 2);
    index.addHit(6, 1);
    index.addHit(8, 1);
    index.addHit(16, 1);
    
    index.deleteSurfelLandmarksAndRenumber(std::vector<int>{4, 5, 17, 18, 19});
    
    XCTAssertEqual(index.size(), 5);
    XCTAssertEqual(index.getHitCount(1, 1), 1);
    XCTAssertEqual(index.getHitCount(2, 1), 1);
    XCTAssertEqual(index.getHitCount(4, 1), 1);
    XCTAssertEqual(index.getHitCount(4, 2), 0);
    XCTAssertEqual(index.getHitCount(6, 1), 1);
    XCTAssertEqual(index.getHitCount(7, 1), 0);
    XCTAssertEqual(index.getHitCount(8, 1), 0);
    XCTAssertEqual(index.getHitCount(14, 1), 1);
    XCTAssertEqual(index.getHitCount(15, 1), 0);
    XCTAssertEqual(index.getHitCount(16, 1), 0);
}

// In the refactoring of the fusion code into a module, I opted not to
// include the landmarks in this refactoring in order to save time,
// but instead commented out all that code.
// it will be commented out again, once that has been fixed.

- (void)testReconstructionWithLandmarks {
    ICPConfiguration icpConfig;
    PBFConfiguration pbfConfig;
    SurfelFusionConfiguration surfelFusionConfig;

    NSString *testCase = @"sven-ear-to-ear-lo-res";
    NSString *testCasesPath = [PathHelpers testCasesPath];
    NSString *testCasePath = [testCasesPath stringByAppendingPathComponent:testCase];
        
    NSString *depthFramesDirectory = [testCasePath stringByAppendingPathComponent:@"DepthFrames"];
    
    std::unique_ptr<PBFModel> pbf(assimilatePointCloud(depthFramesDirectory, icpConfig, pbfConfig, surfelFusionConfig, [](int frameNumber) {
        std::vector<ScreenSpaceLandmark> landmarks {};
        landmarks.push_back({0.5f, 0.4f, 0});
        landmarks.push_back({0.5f, 0.6f, 1});
        return landmarks;
    }));
    
    Surfels surfels(pbf->getSurfels());
    SparseSurfelLandmarksIndex surfelLandmarks(pbf->getSurfelLandmarksIndex());
    
    for (auto landmarksForSurfelIterator : surfelLandmarks) {
        int surfelIndex = landmarksForSurfelIterator.first;
        
        for (auto landmarkIterator : landmarksForSurfelIterator.second) {
            int landmarkIndex = landmarkIterator.first;
            surfels[surfelIndex].color = landmarkIndex == 0 ? Eigen::Vector3f(1, 0, 0) : Eigen::Vector3f(0, 1, 0);
        }
    }
    
    // For debugging via PLY output
    // std::string path = std::string("/Users/rreusser/full.ply");
    // std::cout << "writing output to \"" << path << "\"" << std::endl;
    // PointCloudIO::WriteSurfelsToPLYFile(surfels.data(), surfels.size(), Eigen::Vector3f(0.0f, 0.0f, 0.0f), path);

    // The results of this test appear to be platform-dependent, depending on whether the integrated
    // or discrete GPU is used, which means it probably depends the subtle pixel-level details of
    // rasterization.
    // which means, we add a bit of a margin to the tests.
    
    printf("surfelLandmarks %d\n", surfelLandmarks.size());
    XCTAssertLessThan(abs((float)surfelLandmarks.size() - 148.0f), 25.0f);
}

- (void)testIterateHits {
    SparseSurfelLandmarksIndex index;
    
    index.addHit(17, 5);
    index.addHit(17, 7);
    index.addHit(17, 7);
    index.addHit(13, 9);
    index.addHit(13, 9);
    
    int i = 0;
    int *ii = &i;
    index.iterateHits([self, ii](int surfelIndex, int landmarkIndex, int hitCount) {
        switch (*ii) {
            case 0:
                XCTAssertEqual(surfelIndex, 13);
                XCTAssertEqual(landmarkIndex, 9);
                XCTAssertEqual(hitCount, 2);
                break;
            case 1:
                XCTAssertEqual(surfelIndex, 17);
                XCTAssertEqual(hitCount, landmarkIndex == 5 ? 1 : 2);
                break;
            case 2:
                XCTAssertEqual(surfelIndex, 17);
                XCTAssertEqual(hitCount, landmarkIndex == 5 ? 1 : 2);
                break;
            default:
                XCTFail(@"Iterated too many times (%d)", *ii);
                break;
        }
        
        ++(*ii);
    });
    
    XCTAssertEqual(i, 3, @"Should have iterated 4 times");
}

static Surfel _surfelWithPosition(float x, float y, float z) {
    Surfel surfel;
    surfel.position = Vector3f(x, y, z);
    surfel.weight = 0;
    surfel.lifetime = 0;
    surfel.surfelSize = 0;
    return surfel;
}

static bool Vector3fAlmostEqual(Vector3f a, Vector3f b) {
    return
        fabs(a.x() - b.x()) < FLT_EPSILON &&
        fabs(a.y() - b.y()) < FLT_EPSILON &&
        fabs(a.z() - b.z()) < FLT_EPSILON;
}

- (void)testComputeCentroids {
    Surfels surfels;
    surfels.push_back(_surfelWithPosition(-10, 10, 0));
    surfels.push_back(_surfelWithPosition(-16, 10, 0));
    surfels.push_back(_surfelWithPosition( 10, 10, 0));
    surfels.push_back(_surfelWithPosition( 10, 16, 0));

    SparseSurfelLandmarksIndex index;
    
    index.addHit(0, 5);
    index.addHit(0, 5);
    index.addHit(1, 5);
    
    index.addHit(2, 9);
    index.addHit(2, 9);
    index.addHit(3, 9);
    
    auto centroids = index.computeCentroids(surfels.data());
    auto centroidLabel5 = centroids[5];
    auto centroidLabel9 = centroids[9];
    XCTAssertTrue(Vector3fAlmostEqual(centroidLabel5, Vector3f(-12, 10, 0)), @"Calculated centroid for label 5 was %f, %f, %f", centroidLabel5.x(), centroidLabel5.y(), centroidLabel5.z());
    XCTAssertTrue(Vector3fAlmostEqual(centroidLabel9, Vector3f( 10, 12, 0)), @"Calculated centroid for label 9 was %f, %f, %f", centroidLabel9.x(), centroidLabel9.y(), centroidLabel9.z());
}

@end
