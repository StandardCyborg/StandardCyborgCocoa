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


#include <gtest/gtest.h>

#include <iostream>

#include "standard_cyborg/algorithms/PrincipalAxes.hpp"

#include "standard_cyborg/math/Vec3.hpp"
#include "standard_cyborg/sc3d/Geometry.hpp"

#include "standard_cyborg/util/DebugHelpers.hpp"

#include "standard_cyborg/io/ply/GeometryFileIO_PLY.hpp"

#include "standard_cyborg/test_helpers/TestHelpers.hpp"


using namespace standard_cyborg::io::ply;
using namespace standard_cyborg::math;

bool ReadPLY(standard_cyborg::sc3d::Geometry& geometryOut, const std::string& filename) {
    std::string PLYPath = standard_cyborg::getTestCasesPath() + filename;
    return ReadGeometryFromPLYFile(geometryOut, PLYPath);
}

TEST(PrincipalAxesTests, testNormalwisePrincipalAxes) {
    
    standard_cyborg::sc3d::Geometry geometry;
    EXPECT_TRUE(ReadPLY(geometry, "TestCase-mannequin-leg/mannequin-leg-meshed.ply"));
    
    Mat3x4 alignmentMatrix = standard_cyborg::algorithms::computeNormalwisePrincipalAxes(geometry);
    
    EXPECT_TRUE(Mat3x4::almostEqual(alignmentMatrix, Mat3x4({
        -0.998805,     0.0350284,    -0.0340911,     0.0207896,
        0.0408923,      0.980892,     -0.190207,    -0.0251242,
        0.0267771,     -0.191373,     -0.981152,     -0.336577
    }), 1.0e-6, 1.0e-6));
    
}

TEST(PrincipalAxesTests, testPointwisePrincipalAxes) {
    standard_cyborg::sc3d::Geometry geometry;
    EXPECT_TRUE(ReadPLY(geometry, "TestCase-mannequin-leg/mannequin-leg-meshed.ply"));
    
    Mat3x4 alignmentMatrix = standard_cyborg::algorithms::computePointwisePrincipalAxes(geometry);
    
    EXPECT_TRUE(Mat3x4::almostEqual(alignmentMatrix, Mat3x4({
        -0.999851,     0.0136953,      0.010522,     0.0207896,
        0.0137762,      0.999876,    0.00765232,    -0.0251242,
        -0.0104159,    0.00779613,     -0.999916,     -0.336577
    }), 1.0e-6, 1.0e-6));
    
}

TEST(PrincipalAxesTests, testPointwisePrincipalAxesWRTVertices) {
    
    std::vector<Vec3> positions {
        {-1, -0.1, -1},
        {1, -0.1, -1},
        {0, 0.1, -1}
    };
    
    
    Mat3x4 alignment = standard_cyborg::algorithms::computePointwisePrincipalAxes(positions);
    
    EXPECT_TRUE(Mat3x4::almostEqual(alignment, Mat3x4({
        1, 0, 0,          0,
        0, 1, 0, -0.0333333,
        0, 0, 1,         -1
    }), 1.0e-5, 1.0e-5));
    
}
