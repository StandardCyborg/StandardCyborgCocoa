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


#include <doctest/doctest.h>

#include <standard_cyborg/io/ply/RawFrameDataIO_PLY.hpp>

#include <standard_cyborg/sc3d/ColorImage.hpp>
#include <standard_cyborg/sc3d/DepthImage.hpp>
#include <standard_cyborg/sc3d/PerspectiveCamera.hpp>

#include <standard_cyborg/math/Mat3x3.hpp>

#include <standard_cyborg/test_helpers/TestHelpers.hpp>

/*
 #import <StandardCyborgIO/RawFrameDataIO_PLY.hpp>
 
 #import "DebugHelpers.hpp"
 #import "TestHelpers/PathHelpers.h"
 */



using namespace standard_cyborg::sc3d;
using namespace standard_cyborg::math;

TEST_CASE("BPLYFrameFileIOTests.testReading") {
    std::string testFrame = standard_cyborg::getTestCasesPath() + std::string("frame-000.ply");
    
    ColorImage image;
    DepthImage depth;
    PerspectiveCamera camera;
    
    bool ok = standard_cyborg::io::ply::ReadRawFrameDataFromPLYFile(image, depth, camera, testFrame);
    CHECK(ok);
    
    CHECK_EQ(image.getWidth(), 320);
    CHECK_EQ(image.getHeight(), 240);
    
    CHECK_EQ(depth.getWidth(), 320);
    CHECK_EQ(depth.getHeight(), 240);
    
    CHECK(Mat3x3::almostEqual(
                                    camera.getIntrinsicMatrix(),
                                    Mat3x3{
        2881.16,       0, 1536.59,
        0, 2881.16, 1149.33,
        0,       0,       1
    },
                                    1.0e-5, 1.0e-5
                                    ));
    
    CHECK_EQ(camera.getIntrinsicMatrixReferenceSize().x, 3088.0);
    CHECK_EQ(camera.getIntrinsicMatrixReferenceSize().y, 2316.0);
}
