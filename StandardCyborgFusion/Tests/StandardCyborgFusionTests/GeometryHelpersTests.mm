//
//  GeometryHelpersTests.m
//  StandardCyborgFusionTests
//
//  Created by Ricky Reusser on 8/27/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <XCTest/XCTest.h>

#include <StandardCyborgFusion/GeometryHelpers.hpp>

#include <iostream>

@interface GeometryHelpersTests : XCTestCase

@end

@implementation GeometryHelpersTests

- (void)testRotationFromEulerAngles {
    Eigen::Matrix3f R;
    R << -1, 0, 0,
         0, 1, 0,
         0, 0, -1;
    
    Eigen::Matrix3f rotation = rotationFromEulerAngles(Eigen::Vector3f(+0.00, M_PI, 0.00));
    
    XCTAssertEqualWithAccuracy(R(0,0),  rotation(0,0), FLT_EPSILON);
    XCTAssertEqualWithAccuracy(R(0,1),  rotation(0,1), FLT_EPSILON);
    XCTAssertEqualWithAccuracy(R(0,2),  rotation(0,2), FLT_EPSILON);
    
    XCTAssertEqualWithAccuracy(R(1,0),  rotation(1,0), FLT_EPSILON);
    XCTAssertEqualWithAccuracy(R(1,1),  rotation(1,1), FLT_EPSILON);
    XCTAssertEqualWithAccuracy(R(1,2),  rotation(1,2), FLT_EPSILON);
    
    XCTAssertEqualWithAccuracy(R(2,0),  rotation(2,0), FLT_EPSILON);
    XCTAssertEqualWithAccuracy(R(2,1),  rotation(2,1), FLT_EPSILON);
    XCTAssertEqualWithAccuracy(R(2,2),  rotation(2,2), FLT_EPSILON);
}

@end
