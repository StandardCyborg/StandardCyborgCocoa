/*
 Copyright 2020 Standard Cyborg
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */


/*
 #import <XCTest/XCTest.h>
 #include <iostream>
 
 #include <StandardCyborgData/DebugHelpers.hpp>
 #include <StandardCyborgData/DepthImage.hpp>
 
 using namespace StandardCyborg;
 
 @interface DepthImageTests : XCTestCase
 
 @end
 
 @implementation DepthImageTests
 */

#include <doctest/doctest.h>

#include <standard_cyborg/sc3d/DepthImage.hpp>

using namespace standard_cyborg::sc3d;

TEST_CASE("DepthImageTests.testConstructor") {
    standard_cyborg::sc3d::DepthImage frame;
    CHECK_EQ(frame.getWidth(), 0);
    CHECK_EQ(frame.getHeight(), 0);
    CHECK_EQ(frame.getData().size(), 0);
}

TEST_CASE("DepthImageTests.testConstructorWithSize") {
    standard_cyborg::sc3d::DepthImage frame (2, 2);
    CHECK_EQ(frame.getWidth(), 2);
    CHECK_EQ(frame.getHeight(), 2);
    CHECK_EQ(frame.getData().size(), 4);
}

TEST_CASE("DepthImageTests.testConstructorWithSizeAndData") {
    standard_cyborg::sc3d::DepthImage frame (1, 1, {1});
    CHECK_EQ(frame.getWidth(), 1);
    CHECK_EQ(frame.getHeight(), 1);
    //CHECK(Vec4::almostEqual(frame.getData()[0], {1, 0, 1, 0}));
}

TEST_CASE("DepthImageTests.testGetPixel") {
    standard_cyborg::sc3d::DepthImage frame (2, 3, {0.0, 0.5, 0.75, 1.0, 0.0, 0.5});
    
    CHECK_EQ(frame.getPixelAtColRow(0, 0), 0.0);
    CHECK_EQ(frame.getPixelAtColRow(1, 0), 0.5);
    
    CHECK_EQ(frame.getPixelAtColRow(0, 1), 0.75);
    CHECK_EQ(frame.getPixelAtColRow(1, 1), 1.0);
    
    CHECK_EQ(frame.getPixelAtColRow(0, 2), 0.0);
    CHECK_EQ(frame.getPixelAtColRow(1, 2), 0.5);
}

TEST_CASE("DepthImageTests.testGetPixelConst") {
    const standard_cyborg::sc3d::DepthImage frame (2, 3, {0.0, 0.5, 0.75, 1.0, 0.0, 0.5});
    
    CHECK_EQ(frame.getPixelAtColRow(0, 0), 0.0);
    CHECK_EQ(frame.getPixelAtColRow(1, 0), 0.5);
    
    CHECK_EQ(frame.getPixelAtColRow(0, 1), 0.75);
    CHECK_EQ(frame.getPixelAtColRow(1, 1), 1.0);
    
    CHECK_EQ(frame.getPixelAtColRow(0, 2), 0.0);
    CHECK_EQ(frame.getPixelAtColRow(1, 2), 0.5);
}

TEST_CASE("DepthImageTests.testGetTexCoordAtColRow") {
    standard_cyborg::sc3d::DepthImage frame (2, 4);
    
    CHECK(frame.getTexCoordAtColRow(1, 3) == standard_cyborg::math::Vec2((1.5f) / 2.0f, 3.5f / 4.0f));
}

TEST_CASE("DepthImageTests.testSetPixel") {
    standard_cyborg::sc3d::DepthImage frame (2, 3);
    
    frame.setPixelAtColRow(0, 0, 1);
    frame.setPixelAtColRow(1, 0, 2);
    
    frame.setPixelAtColRow(0, 1, 3);
    frame.setPixelAtColRow(1, 1, 4);
    
    frame.setPixelAtColRow(0, 2, 5);
    frame.setPixelAtColRow(1, 2, 6);
    
    
    std::vector<float> expectedPixels {1, 2, 3, 4, 5, 6};
    
    for (int i = 0; i < frame.getWidth() * frame.getHeight(); i++) {
        CHECK_EQ(frame.getData()[i], expectedPixels[i]);
    }
}

TEST_CASE("DepthImageTests.testCopy") {
    standard_cyborg::sc3d::DepthImage i1 (2, 3, {1, 2, 3, 4, 5, 6});
    
    standard_cyborg::sc3d::DepthImage i2;
    i2.copy(i1);
    
    CHECK_EQ(i2.getWidth(), 2);
    CHECK_EQ(i2.getHeight(), 3);
    
    for (int i = 0; i < i1.getWidth() * i1.getHeight(); i++) {
        CHECK_EQ(i1.getData()[i], i2.getData()[i]);
    }
}

TEST_CASE("DepthImageTests.testGetSizeInBytes") {
    standard_cyborg::sc3d::DepthImage i1 (2, 3, {1, 2, 3, 4, 5, 6});
    
    CHECK_EQ(i1.getSizeInBytes(), 2 * 3 * sizeof(int));
}

TEST_CASE("DepthImageTests.testFlipYOddSize") {
    standard_cyborg::sc3d::DepthImage frame (2, 3, {
        1, 2,
        3, 4,
        5, 6
    });
    
    frame.flipY();
    
    standard_cyborg::sc3d::DepthImage expectedImage (2, 3, {
        5, 6,
        3, 4,
        1, 2
    });
    
    CHECK(frame == expectedImage);
}

TEST_CASE("DepthImageTests.testFlipYEvenSize") {
    standard_cyborg::sc3d::DepthImage frame (2, 4, {
        1, 2,
        3, 4,
        5, 6,
        7, 8
    });
    
    frame.flipY();
    
    standard_cyborg::sc3d::DepthImage expectedImage (2, 4, {
        7, 8,
        5, 6,
        3, 4,
        1, 2
    });
    
    CHECK(frame == expectedImage);
}

TEST_CASE("DepthImageTests.testFlipXEvenSize") {
    standard_cyborg::sc3d::DepthImage frame (2, 3, {
        1, 2,
        3, 4,
        5, 6
    });
    
    frame.flipX();
    
    standard_cyborg::sc3d::DepthImage expectedImage (2, 3, {
        2, 1,
        4, 3,
        6, 5
    });
    
    CHECK(frame == expectedImage);
}

TEST_CASE("DepthImageTests.testFlipXOddSize") {
    standard_cyborg::sc3d::DepthImage frame (3, 1, {
        1, 2, 3
    });
    
    frame.flipX();
    
    standard_cyborg::sc3d::DepthImage expectedImage (3, 1, {
        3, 2, 1
    });
    
    CHECK(frame == expectedImage);
}

TEST_CASE("DepthImageTests.testResetSize") {
    standard_cyborg::sc3d::DepthImage frame;
    
    frame.resetSize(3, 4);
    
    CHECK_EQ(frame.getWidth(), 3);
    CHECK_EQ(frame.getHeight(), 4);
}

TEST_CASE("DepthImageTests.testForEachPixel") {
    standard_cyborg::sc3d::DepthImage frame (2, 4, {
        1, 2,
        3, 4,
        5, 6,
        7, 8
    });
    
    float sum = 0.0;
    
    frame.forEachPixelAtColRow([&](int col, int row, float depth) {
        sum += depth;
    });
    CHECK_EQ(sum, 36);
}

TEST_CASE("DepthImageTests.testMutatePixelsByColRow") {
    standard_cyborg::sc3d::DepthImage frame (2, 3, {
        1, 2,
        3, 4,
        5, 6,
    });
    
    frame.mutatePixelsByColRow([&](int col, int row, float depth) {
        return depth + col + row;
    });
    
    standard_cyborg::sc3d::DepthImage expectedImage (2, 3, {
        1, 3,
        4, 6,
        7, 9,
    });
    
    CHECK(expectedImage == frame);
}
