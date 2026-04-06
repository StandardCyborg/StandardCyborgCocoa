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
#include <cmath>

#ifndef CHECK_NEAR
#define CHECK_NEAR(a, b, tol) CHECK(std::abs(static_cast<double>(a) - static_cast<double>(b)) <= static_cast<double>(tol))
#endif

#include <sstream>

#include <standard_cyborg/sc3d/Geometry.hpp>
#include <standard_cyborg/io/ply/GeometryFileIO_PLY.hpp>

#include <standard_cyborg/math/Vec3.hpp>

#include <standard_cyborg/test_helpers/TestHelpers.hpp>

using namespace standard_cyborg::sc3d;
using namespace standard_cyborg::math;
using namespace standard_cyborg::io::ply;

TEST_CASE("FileIOTests.testReading") {
    {
        std::string PLYPath = standard_cyborg::getTestCasesPath() + std::string("Expected.ply");
        
        Geometry geometry;
        CHECK(standard_cyborg::io::ply::FragileReadGeometryFromPLYFile(geometry, PLYPath));
        CHECK_EQ(geometry.vertexCount(), 45428);
        CHECK_EQ(geometry.faceCount(), 0);
    }
    
    // mesh with only normals and positions.
    {
        std::string PLYPath = standard_cyborg::getTestCasesPath() + std::string("test-plane.ply");
        
        Geometry geometry;
        CHECK(standard_cyborg::io::ply::FragileReadGeometryFromPLYFile(geometry, PLYPath));
        CHECK_EQ(geometry.vertexCount(), 41);
        CHECK_EQ(geometry.faceCount(), 64);
    }
}

TEST_CASE("FileIOTests.testEncodingSurfelRadius") {
    Geometry geometry {
        std::vector<Vec3>{{1, 2, 3}, {4, 5, 6}},
        std::vector<Vec3>{{3, 4, 0}, {5, 12, 0}}
    };
    
    std::stringstream stream;
    
    // Write and read, confirming surfel radius was not encoded
    Geometry readGeometry;
    WriteGeometryToPLYStream(stream, geometry);
    ReadGeometryFromPLYStream(readGeometry, stream);
    
    CHECK_EQ(readGeometry.vertexCount(), 2);
    CHECK(readGeometry.hasPositions());
    CHECK(readGeometry.hasNormals());
    
    // False if no surfel_radius column present
    CHECK_FALSE(readGeometry.normalsEncodeSurfelRadius());
    
    // The normals in this case are written and read without modification
    CHECK_EQ(readGeometry.getNormals()[0], Vec3(3, 4, 0));
    CHECK_EQ(readGeometry.getNormals()[1], Vec3(5, 12, 0));
    
    // Switch this flag and try again
    geometry.setNormalsEncodeSurfelRadius(true);
    
    WriteGeometryToPLYStream(stream, geometry);
    ReadGeometryFromPLYStream(readGeometry, stream);
    
    CHECK_EQ(readGeometry.vertexCount(), 2);
    CHECK(readGeometry.hasPositions());
    CHECK(readGeometry.hasNormals());
    
    // This is true upon read only if `surfel_radius` column was present
    CHECK(readGeometry.normalsEncodeSurfelRadius());
    
    // The normals here are separated into surfel_radius, then rescaled
    CHECK_EQ(readGeometry.getNormals()[0], Vec3(3, 4, 0));
    CHECK_EQ(readGeometry.getNormals()[1], Vec3(5, 12, 0));
    
}


TEST_CASE("FileIOTests.testRobustPLYReading") {
    {
        std::string PLYPath = standard_cyborg::getTestCasesPath() + std::string("test-plane.ply");
        
        Geometry geometry;
        ReadGeometryFromPLYFile(geometry, PLYPath);
        CHECK_EQ(geometry.vertexCount(), 41);
        CHECK(geometry.hasPositions());
        CHECK(geometry.hasNormals());
        CHECK_FALSE(geometry.hasColors());
        CHECK(geometry.hasFaces());
        CHECK_EQ(geometry.faceCount(), 64);
    }
    {
        std::string PLYPath = standard_cyborg::getTestCasesPath() + std::string("Expected.ply");
        
        Geometry geometry;
        ReadGeometryFromPLYFile(geometry, PLYPath);
        CHECK_EQ(geometry.vertexCount(), 45428);
        CHECK_EQ(geometry.faceCount(), 0);
        CHECK(geometry.hasPositions());
        CHECK(geometry.hasNormals());
        CHECK(geometry.hasColors());
        CHECK_FALSE(geometry.hasFaces());
    }
}


TEST_CASE("FileIOTests.testWriting") {
    std::string PLYPath = std::string("/tmp/test.ply");
    
    std::vector<Vec3> positions, normals, colors;
    std::vector<Face3> faces;
    
    positions.push_back(Vec3(-1, -1, 2));
    positions.push_back(Vec3( 1, -1, 2));
    positions.push_back(Vec3( 1,  1, 2));
    positions.push_back(Vec3(-1,  1, 2));
    
    normals.push_back(Vec3(-1, -2, -3));
    normals.push_back(Vec3(-2, -1, -3));
    normals.push_back(Vec3(-3, -1, -2));
    normals.push_back(Vec3(-3, -2, -1));
    
    colors.push_back(Vec3(0.2, 0.3, 0.5));
    colors.push_back(Vec3(0.3, 0.5, 0.2));
    colors.push_back(Vec3(0.5, 0.2, 0.3));
    colors.push_back(Vec3(0.5, 0.3, 0.2));
    
    faces.push_back(Face3(0, 1, 2));
    faces.push_back(Face3(1, 2, 3));
    
    Geometry geometry(positions, normals, colors, faces);
    CHECK(WriteGeometryToPLYFile(PLYPath, geometry));
    
    // Now read it back
    Geometry readGeometry;
    ReadGeometryFromPLYFile(readGeometry, PLYPath);
    CHECK_EQ(readGeometry.vertexCount(), positions.size());
    CHECK_EQ(readGeometry.faceCount(), faces.size());
    
    Vec3 readPosition1 = readGeometry.getPositions()[1];
    Vec3 readNormal1 = readGeometry.getNormals()[1];
    Vec3 readColor1 = readGeometry.getColors()[1];
    Face3 readFace1 = readGeometry.getFaces()[1];
    CHECK_EQ(readPosition1, positions[1]);
    
    CHECK_NEAR(readNormal1.x, -2, 1e-4);
    CHECK_NEAR(readNormal1.y, -1, 1e-4);
    CHECK_NEAR(readNormal1.z, -3, 1e-4);
    
    // Due to reading and writing as 8-bit values, we only have an accuracy range of 1/256
    CHECK_NEAR(readColor1.x, colors[1].x, 1.0/256.0);
    CHECK_NEAR(readColor1.y, colors[1].y, 1.0/256.0);
    CHECK_NEAR(readColor1.z, colors[1].z, 1.0/256.0);
    CHECK_EQ(readFace1, faces[1]);
}
