//
//  EigenHelperTests.m
//  StandardCyborgFusionTests
//
//  Created by Aaron Thompson on 8/6/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <XCTest/XCTest.h>
#import <StandardCyborgFusion/StandardCyborgFusion.h>

#include <StandardCyborgFusion/EigenHelpers.hpp>

@interface EigenHelperTests : XCTestCase
@end

@implementation EigenHelperTests

- (void)testVec3TransformMat4 {
    Vector3f vector(11, 22, 33);
    Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();
    transform(0, 3) = 1;
    transform(1, 3) = 2;
    transform(2, 3) = 3;
    Vector3f translated = Vec3TransformMat4(vector, transform);
    XCTAssertEqualWithAccuracy(translated.x(), 12, FLT_EPSILON);
    XCTAssertEqualWithAccuracy(translated.y(), 24, FLT_EPSILON);
    XCTAssertEqualWithAccuracy(translated.z(), 36, FLT_EPSILON);
    
    transform.setIdentity();
    transform(0, 0) = 1;
    transform(1, 1) = 2;
    transform(2, 2) = 4;
    Vector3f scaled = Vec3TransformMat4(vector, transform);
    XCTAssertEqualWithAccuracy(scaled.x(), 11*1, FLT_EPSILON);
    XCTAssertEqualWithAccuracy(scaled.y(), 22*2, FLT_EPSILON);
    XCTAssertEqualWithAccuracy(scaled.z(), 33*4, FLT_EPSILON);
}

- (void)testNormalMatrixFromMat4 {
    Eigen::Matrix4f matrix = Eigen::Matrix4f::Identity();
    matrix(0, 0) = 1;
    matrix(1, 1) = 2;
    matrix(2, 2) = 3;
    
    Eigen::Matrix3f result = NormalMatrixFromMat4(matrix);
    XCTAssertEqualWithAccuracy(result(0, 0), 1, FLT_EPSILON);
    XCTAssertEqualWithAccuracy(result(1, 1), 1.0f/2.0f, FLT_EPSILON);
    XCTAssertEqualWithAccuracy(result(2, 2), 1.0f/3.0f, FLT_EPSILON);
}

- (void)testmatrix_float3x3FromMatrix3f {
    Eigen::Matrix3f input;
    input(0, 0) = 1; input(0, 1) = 2; input(0, 2) = 3;
    input(1, 0) = 4; input(1, 1) = 5; input(1, 2) = 6;
    input(2, 0) = 7; input(2, 1) = 8; input(2, 2) = 9;
    
    matrix_float3x3 result = toSimdFloat3x3(input);
    XCTAssertEqual(result.columns[0].x, 1);
    XCTAssertEqual(result.columns[1].x, 2);
    XCTAssertEqual(result.columns[2].x, 3);
    XCTAssertEqual(result.columns[0].y, 4);
    XCTAssertEqual(result.columns[1].y, 5);
    XCTAssertEqual(result.columns[2].y, 6);
    XCTAssertEqual(result.columns[0].z, 7);
    XCTAssertEqual(result.columns[1].z, 8);
    XCTAssertEqual(result.columns[2].z, 9);
}

- (void)testSurfelsAsEigenMap {
    Surfels surfels (2);
    surfels[0].position = Vector3f(1.0f, 2.0f, 3.0f);
    surfels[0].normal = Vector3f(4.0f, 5.0f, 6.0f);
    surfels[0].color = Vector3f(8.0f, 9.0f, 10.0f);
    
    surfels[1].position = Vector3f(11.0f, 12.0f, 13.0f);
    surfels[1].normal = Vector3f(14.0f, 15.0f, 16.0f);
    surfels[1].color = Vector3f(17.0f, 18.0f, 19.0f);
    
    EigenMappedSurfelPositions positions (getEigenMappedSurfelPositions(surfels));
    EigenMappedSurfelNormals normals (getEigenMappedSurfelNormals(surfels));
    EigenMappedSurfelColors colors (getEigenMappedSurfelColors(surfels));
    
    XCTAssertEqual(positions.rows(), 3);
    XCTAssertEqual(positions.cols(), 2);
    
    XCTAssertEqual(normals.rows(), 3);
    XCTAssertEqual(normals.cols(), 2);
    
    XCTAssertEqual(colors.rows(), 3);
    XCTAssertEqual(colors.cols(), 2);
    
    XCTAssertEqual(surfels[0].position, positions.col(0));
    XCTAssertEqual(surfels[0].normal, normals.col(0));
    XCTAssertEqual(surfels[0].color, colors.col(0));
    
    XCTAssertEqual(surfels[1].position, positions.col(1));
    XCTAssertEqual(surfels[1].normal, normals.col(1));
    XCTAssertEqual(surfels[1].color, colors.col(1));
}

/*
- (void)testPerspectiveMatrixFromIntrinsics {
    Matrix3f intrinsic;
    Matrix4f extrinsic;
    
    // Roughly but not numerically exactly what the iPhone tends to give us
    intrinsic << 2866.0,    0.0, 1920.0,
                    0.0, 2866.0, 1089.0,
                    0.0,    0.0,    1.0;
    
    // I think this tends to account for the flipped y axis you always run into where the
    // top left of the screen is (0, 0)
    extrinsic << 1.0, 0.0, 0.0, 0.0,
                 0.0, -1.0, 0.0, 0.0,
                 0.0, 0.0, 1.0, 0.0,
                 0.0, 0.0, 0.0, 1.0;

    Vector2f referenceDimensions(3840, 2160);
    
    Matrix4f perspective = perspectiveMatrixFromIntrinsics(intrinsic, referenceDimensions, 0.01, 10.0);
    XCTAssertEqualWithAccuracy(perspective(0, 0), 2866.0, FLT_EPSILON);
    XCTAssertEqualWithAccuracy(perspective(0, 1), 0.0, FLT_EPSILON);
    XCTAssertEqualWithAccuracy(perspective(0, 2), 1920.0, FLT_EPSILON);
    XCTAssertEqualWithAccuracy(perspective(0, 3), 0.0, FLT_EPSILON);
    
    XCTAssertEqualWithAccuracy(perspective(1, 0), 0.0, FLT_EPSILON);
    XCTAssertEqualWithAccuracy(perspective(1, 1), 2866.0, FLT_EPSILON);
    XCTAssertEqualWithAccuracy(perspective(1, 2), 1089.0, FLT_EPSILON);
    XCTAssertEqualWithAccuracy(perspective(1, 3), 0.0, FLT_EPSILON);
    
    XCTAssertEqualWithAccuracy(perspective(2, 0), 0.0, FLT_EPSILON);
    XCTAssertEqualWithAccuracy(perspective(2, 1), 0.0, FLT_EPSILON);
    XCTAssertEqualWithAccuracy(perspective(2, 2), 0.0, FLT_EPSILON);
    XCTAssertEqualWithAccuracy(perspective(2, 3), 1.0, FLT_EPSILON);
    
    XCTAssertEqualWithAccuracy(perspective(3, 0), 0.0, FLT_EPSILON);
    XCTAssertEqualWithAccuracy(perspective(3, 1), 0.0, FLT_EPSILON);
    XCTAssertEqualWithAccuracy(perspective(3, 2), -1.0, FLT_EPSILON);
    XCTAssertEqualWithAccuracy(perspective(3, 3), 0.0, FLT_EPSILON);
}
*/

@end
