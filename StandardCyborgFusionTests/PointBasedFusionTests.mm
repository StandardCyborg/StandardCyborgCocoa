//
//  StandardCyborgFusionTests.mm
//  StandardCyborgFusionTests
//
//  Created by Aaron Thompson on 7/10/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <XCTest/XCTest.h>
#import <StandardCyborgFusion/StandardCyborgFusion.h>

#include <standard_cyborg/util/IncludeEigen.hpp>
#include <standard_cyborg/util/DataUtils.hpp>
#include <StandardCyborgFusion/MathHelpers.h>
#include <StandardCyborgFusion/MetalDepthProcessor.hpp>
#include <StandardCyborgFusion/PBFModel.hpp>

#include "Helpers/PathHelpers.h"
#include "Helpers/ReconstructionHelpers.h"

#include <iostream>
#include <cmath>

#import <StandardCyborgFusion/PointCloudIO.hpp>
#import <standard_cyborg/math/Mat3x4.hpp>
#import <standard_cyborg/math/Mat3x3.hpp>
#import <standard_cyborg/sc3d/PerspectiveCamera.hpp>


// https://stackoverflow.com/questions/39680320/printing-debugging-libc-stl-with-xcode-lldb
template struct std::vector<Eigen::VectorXf>;
using namespace standard_cyborg;

@interface StandardCyborgFusionTests : XCTestCase

@end

@implementation StandardCyborgFusionTests

- (sc3d::PerspectiveCamera)_realPerspectiveCamera
{
    sc3d::PerspectiveCamera camera;
    camera.setNominalIntrinsicMatrix(math::Mat3x3(2876.34839, 0, 1911.88721,
                                            0, 2876.34839, 1078.8269,
                                            0, 0, 1));
    camera.setOrientationMatrix(math::Mat3x4(1, 0, 0, 0,
                                       0, 1, 0, 0,
                                       0, 0, 1, 0));
    camera.setIntrinsicMatrixReferenceSize(math::Vec2(3840, 2160));
    //intrinsics.imageWidth = 320;
    //intrinsics.imageHeight = 180;

    return camera;
}

- (void)testMetalDepthProcessor
{
    sc3d::PerspectiveCamera camera = [self _realPerspectiveCamera];
    
    size_t width = 320;
    size_t height = 180;
    size_t pixelCount = width * height;
    size_t middlePixelIndex = width * height / 2 + width / 2;
    float testDepth = 0.2;
    std::vector<float> depths(width* height, testDepth);
    
    std::vector<math::Vec3> colors(width * height, math::Vec3());
    
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    id<MTLCommandQueue> commandQueue = [device newCommandQueue];
    MetalDepthProcessor depthProcessor(device, commandQueue);
    
    RawFrame rawFrame(camera, width, height, depths, colors, 0.0f);
    ProcessedFrame frame(rawFrame);
    depthProcessor.computeFrameValues(frame, rawFrame, false);
    
    XCTAssertGreaterThan(frame.inputConfidences[0], 0.1);
    XCTAssertGreaterThan(frame.inputConfidences[middlePixelIndex], 0.95);
    XCTAssertGreaterThan(frame.inputConfidences[pixelCount - width * 10 - 10], 0.1);
    
    Vector3f normal = toVector3f(frame.normals[middlePixelIndex]).normalized();
    XCTAssertEqualWithAccuracy(normal.x(), 0, FLT_EPSILON);
    XCTAssertEqualWithAccuracy(normal.y(), 0, FLT_EPSILON);
    XCTAssertEqualWithAccuracy(normal.z(), 1, FLT_EPSILON);
    normal = toVector3f(frame.normals[pixelCount - 10 * width - 10]).normalized();
    XCTAssertEqualWithAccuracy(normal.x(), 0, FLT_EPSILON);
    XCTAssertEqualWithAccuracy(normal.y(), 0, FLT_EPSILON);
    XCTAssertEqualWithAccuracy(normal.z(), 1, FLT_EPSILON);
}


- (void)testPBF
{
    ICPConfiguration icpConfig;
    
    PBFConfiguration pbfConfig;
    SurfelFusionConfiguration surfelFusionConfig;
    surfelFusionConfig.maxDepth = 0.75;
    surfelFusionConfig.minDepth = 0.0;
    pbfConfig.icpDownsampleFraction = 0.2f;
    
    NSString *testCasesPath = [PathHelpers testCasesPath];
    NSArray *testCases = [NSArray arrayWithObjects:@"sven-ear-to-ear-lo-res", nil];
    
    for (size_t iTest = 0; iTest < [testCases count]; ++iTest) {
        NSString *testCase = testCases[iTest];
        
        // get the test case paths from the bundle.
        NSString *testCasePath = [testCasesPath stringByAppendingPathComponent:testCase];
        NSString *depthFramesDir = [testCasePath stringByAppendingPathComponent:@"DepthFrames"];
        NSString *expectedPLYPath = [testCasePath stringByAppendingPathComponent:@"Expected.ply"];

        std::unique_ptr<PBFModel> pbf(assimilatePointCloud(depthFramesDir, icpConfig, pbfConfig, surfelFusionConfig));
        Surfels surfels = pbf->getSurfels();

        // read target point cloud.
        Surfels targetSurfels;
        PointCloudIO::ReadSurfelsFromPLYFile(targetSurfels, std::string([expectedPLYPath UTF8String]));

        float sumPosSquaredError = 0.0f;
        float sumColorSquaredError = 0.0f;
        
        std::vector<math::Vec3> positions;
        std::vector<math::Vec3> colors;
        std::vector<math::Vec3> normals;
        
        for (int iSurfel = 0; iSurfel < targetSurfels.size(); ++iSurfel) {
            Surfel s = targetSurfels[iSurfel];
            
            positions.push_back(math::Vec3(s.position.x(), s.position.y(), s.position.z()));
            normals.push_back(math::Vec3(s.normal.x(), s.normal.y(), s.normal.z()));
            colors.push_back(math::Vec3(s.color.x(), s.color.y(), s.color.z()));
        }
        
        sc3d::Geometry target(positions, normals, colors);
        
        // calculuate the similarity between the assimilated surfels, and the target point cloud, using nearest neighbours.
        for (size_t index = 0; index < surfels.size(); ++index) {
        
            Surfel surfel = surfels[index];
            math::Vec3 sourcePosition = math::Vec3(surfel.position.x(), surfel.position.y(), surfel.position.z());

            int nearestRefVertexIndex = target.getClosestVertexIndex(sourcePosition);
        
            float nearestRefVertexDistanceSquared = math::Vec3::squaredDistanceBetween(sourcePosition, target.getPositions()[nearestRefVertexIndex]);
           
            math::Vec3 targetColor = target.getColors()[nearestRefVertexIndex];

            // not dealing with normals for now, since they are being truncated way too much
            // for any meaningful verifications right now.
            sumPosSquaredError += nearestRefVertexDistanceSquared;
            
            sumColorSquaredError += (surfel.color - Vector3f(targetColor.x, targetColor.y, targetColor.z) ).squaredNorm();
        }
        
        #if 0
        // Activate this section to *write* this newly computed point cloud to PLY
        NSString *outputPLYPath = [testCasePath stringByAppendingPathComponent:@"/Expected-tmp.ply"];

        PointCloudIO::WriteSurfelsToPLYFile(surfels.data(),
                                        surfels.size(),
                                        Eigen::Vector3f(0.0, 0.0, 0.0),
                                        std::string([outputPLYPath UTF8String]));
        #endif
        
        float rmsPositionError = std::sqrtf(sumPosSquaredError) / surfels.size();
        float rmsColorError = std::sqrtf(sumColorSquaredError) / surfels.size();
        
        NSLog(@"RMS position error: %f", rmsPositionError);
        NSLog(@"RMS color error: %f", rmsColorError);

        const float maxPosError = 1.0e-5;
        XCTAssertLessThan(rmsPositionError, maxPosError, @"exceeding maxPosError for %@", testCase);
        
        const float maxColorError = 3.0e-4;
        XCTAssertLessThan(rmsColorError, maxColorError, @"exceeding maxColorError for %@", testCase);
        
        // The results of this test appear to be platform-dependent, depending on whether the discrete
        // or integrated GPU is used.
        // for this reason, we have to add a bit of a margin to the tests.
        XCTAssertLessThan(abs((float)surfels.size() - 45428.0f), 350.0f);
    }
}

@end
