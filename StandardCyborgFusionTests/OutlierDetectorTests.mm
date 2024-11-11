//
//  OutlierDetectorTests.m
//  StandardCyborgFusionTests
//
//  Created by Ricky Reusser on 8/13/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <XCTest/XCTest.h>
#include <StandardCyborgFusion/OutlierDetector.hpp>
#include <iostream>
#include <random>

using namespace StandardCyborg;

@interface OutlierDetectorTests : XCTestCase
@end

@implementation OutlierDetectorTests

- (void)testInitialization {
    OutlierDetector detector (0.25, 0.5, 4.0, true);
    
    // Create a random number generator
    std::mt19937 prng(0);
    std::uniform_real_distribution<float> randfloat(-1.0f, 1.0f);
    std::uniform_real_distribution<float> outlierSelector(0.0f, 1.0f);
    
    float dt = 1.0 / 30.0;
    int outliers = 0;
    OutlierDetectorResult result;
    for (int i = 0; i < 1000; i++) {
        bool isOutlier = i > 1 && (i % 100 == 0);
        float y = isOutlier ? 1e10 : randfloat(prng);
        result = detector.assimilateSample(i * dt, y);
        if (result.isOutlier) outliers++;
    }
    
    XCTAssertEqualWithAccuracy(result.mean, 0.0149101, 1e-4);
    XCTAssertEqualWithAccuracy(result.variance, 0.354912, 1e-4);
    XCTAssertEqual(outliers, 9);
}

@end
