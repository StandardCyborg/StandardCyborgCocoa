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

#include <standard_cyborg/sc3d/MeshTopology.hpp>

using namespace standard_cyborg::sc3d;

//namespace math = standard_cyborg::math;
//using math::Vec4;

/*
 #import <XCTest/XCTest.h>
 #define _USE_MATH_DEFINES
 #include <cmath>
 #include <iostream>
 
 #include <StandardCyborgData/DebugHelpers.hpp>
 #include <StandardCyborgData/Face3.hpp>
 #include <StandardCyborgData/MeshTopology.hpp>
 
 
 using namespace StandardCyborg;
 
 @interface MeshTopologyTests : XCTestCase
 
 @end
 
 @implementation MeshTopologyTests
 */

TEST_CASE("MeshTopologyTests.testSimpleMesh") {
    // 3 ----3---- 2
    // |         / | \
    // |  1    /   |   6
    // 4     2     1 2  4
    // |   /   0   |   5
    // | /         | /
    // 0 ----0---- 1
    
    std::vector<Face3> faces {
        {0, 1, 2},
        {0, 2, 3},
        {1, 4, 2}
    };
    
    MeshTopology::MeshTopology topology (faces);
    
    CHECK_EQ(topology.getFaceEdges().size(), 3);
    CHECK_EQ(topology.getVertexEdges().size(), 5);
    CHECK_EQ(topology.getEdges().size(), 7);
    
    CHECK_EQ(topology.getNumFaceEdges(), 3);
    CHECK_EQ(topology.getNumVertexEdges(), 5);
    CHECK_EQ(topology.getNumEdges(), 7);
    
    CHECK_EQ(topology.getEdges(), std::vector<MeshTopology::Edge>({
        // {vertex0, vertex1, face0, face1}
        {0, 1, 0, -1},
        {1, 2, 0, 2},
        {2, 0, 0, 1},
        {2, 3, 1, -1},
        {3, 0, 1, -1},
        {1, 4, 2, -1},
        {4, 2, 2, -1}
    }));
    
    CHECK_EQ(topology.getFaceEdges(), std::vector<MeshTopology::FaceEdges>({
        // {edgeAB, edgeBC, edgeCA}
        {0, 1, 2},
        {2, 3, 4},
        {5, 6, 1}
    }));
    
    CHECK_EQ(topology.getVertexEdges(), std::vector<MeshTopology::VertexEdges>({
        {0, 2, 4},
        {0, 1, 5},
        {1, 2, 3, 6},
        {3, 4},
        {5, 6},
    }));
}

TEST_CASE("MeshTopologyTests.testMeshTopologyConstructor") {
    
    std::vector<MeshTopology::Edge> edges({
        {0, 1, 0, -1},
        {1, 2, 0, 2},
        {2, 0, 0, 1},
        {2, 3, 1, -1},
        {3, 0, 1, -1},
        {1, 4, 2, -1},
        {4, 2, 2, -1}
    });
    
    std::vector<MeshTopology::FaceEdges> faceEdges({
        {0, 1, 2},
        {2, 3, 4},
        {5, 6, 1}
    });
    
    std::vector<MeshTopology::VertexEdges> vertexEdges({
        {0, 2, 4},
        {0, 1, 5},
        {1, 2, 3, 6},
        {3, 4},
        {5, 6},
    });
    
    MeshTopology::MeshTopology topology (edges, faceEdges, vertexEdges);
    
    CHECK_EQ(topology.getEdges(), edges);
    
    CHECK_EQ(topology.getFaceEdges(), faceEdges);
    
    CHECK_EQ(topology.getVertexEdges(), vertexEdges);
}

TEST_CASE("MeshTopologyTests.testDefaultConstructor") {
    MeshTopology::MeshTopology topology;
    
    CHECK_EQ(topology.getFaceEdges().size(), 0);
    CHECK_EQ(topology.getVertexEdges().size(), 0);
    CHECK_EQ(topology.getEdges().size(), 0);
}

TEST_CASE("MeshTopologyTests.testFacesEdgesEqualityOperator")
{
    
    CHECK(MeshTopology::FaceEdges({1, 2, 3}) == MeshTopology::FaceEdges({1, 2, 3}));
    
    CHECK(MeshTopology::FaceEdges({1, 2, 3}) != MeshTopology::FaceEdges({0, 2, 3}));
    CHECK(MeshTopology::FaceEdges({1, 2, 3}) != MeshTopology::FaceEdges({1, 0, 3}));
    CHECK(MeshTopology::FaceEdges({1, 2, 3}) != MeshTopology::FaceEdges({1, 2, 9}));
    CHECK_FALSE(MeshTopology::FaceEdges({1, 2, 3}) != MeshTopology::FaceEdges({1, 2, 3}));
}

TEST_CASE("MeshTopologyTests.testEdgeEqualityOperator") {
    
    CHECK(MeshTopology::Edge({1, 2, 3, 4}) == MeshTopology::Edge({1, 2, 3, 4}));
    CHECK_FALSE(MeshTopology::Edge({1, 2, 3, 4}) == MeshTopology::Edge({1, 0, 3, 4}));
    
    CHECK(MeshTopology::Edge({1, 2, 3, 4}) != MeshTopology::Edge({1, 2, 3, 9}));
    CHECK(MeshTopology::Edge({1, 2, 3, 4}) != MeshTopology::Edge({1, 0, 3, 4}));
    CHECK(MeshTopology::Edge({1, 2, 3, 4}) != MeshTopology::Edge({1, 2, 7, 4}));
    CHECK(MeshTopology::Edge({1, 2, 3, 4}) != MeshTopology::Edge({8, 2, 3, 4}));
    CHECK_FALSE(MeshTopology::Edge({1, 2, 3, 4}) != MeshTopology::Edge({1, 2, 3, 4}));
}

TEST_CASE("MeshTopologyTests.testFaceEdgesOffsetOf") {
    MeshTopology::FaceEdges fe({1, 2, 3});
    
    CHECK_EQ(fe.offsetOf(1), 0);
    CHECK_EQ(fe.offsetOf(2), 1);
    CHECK_EQ(fe.offsetOf(3), 2);
    CHECK_EQ(fe.offsetOf(10), -1);
    CHECK_EQ(fe.offsetOf(-2), -1);
}

TEST_CASE("MeshTopologyTests.testFaceEdgesEdgeIndexAfter") {
    MeshTopology::FaceEdges fe({5, 10, 2});
    
    CHECK_EQ(fe.edgeIndexAfter(5), 10);
    CHECK_EQ(fe.edgeIndexAfter(10), 2);
    CHECK_EQ(fe.edgeIndexAfter(2), 5);
}

TEST_CASE("MeshTopologyTests.testFaceEdgesEdgeIndexBefore") {
    MeshTopology::FaceEdges fe({5, 10, 2});
    
    CHECK_EQ(fe.edgeIndexBefore(5), 2);
    CHECK_EQ(fe.edgeIndexBefore(10), 5);
    CHECK_EQ(fe.edgeIndexBefore(2), 10);
}

/*
 struct Edge {
 int vertex0 = -1;
 int vertex1 = -1;
 int face0 = -1;
 int face1 = -1;
 };
 */


/*
 // Uncomment to ensure non-manifold tests fail assertion. This should be properly tested
 // be creating new test schemes or something of the sort so that the assert helper is an
 // exception rather than an assertion and can be caught.
 TEST(MeshTopologyTests, testNonManifoldMesh {
 std::vector<Face3> faces {
 {0, 1, 2},
 {1, 0, 3},
 {0, 1, 4}
 };
 
 MeshTopology::MeshTopology topology (faces);
 }
 */

