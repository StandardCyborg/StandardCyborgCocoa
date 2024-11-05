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

#include "standard_cyborg/algorithms/MeshSlice.hpp"

#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/sc3d/Plane.hpp"
#include "standard_cyborg/sc3d/Polyline.hpp"

using standard_cyborg::sc3d::Geometry;
using standard_cyborg::sc3d::Polyline;
using standard_cyborg::sc3d::Face3;

namespace math = standard_cyborg::math;
using math::Vec3;

TEST(MeshSliceTests, testSimpleOpenPolyline) {
    // 3 ----3---- 2
    // |         / | \
    // |  1    /   |   \
    // 4     2     1     6
    // |   /   0   |   2   \
    // | /         |         \
    // 0 ----0---- 1 ----5---- 4
    
    Geometry geometry (
                       {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {2.0f, 0.0f, 0.0f}},
                       {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
                       {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
                       {{0, 1, 2}, {0, 2, 3}, {1, 4, 2}}
                       );
    
    std::vector<Polyline> polylines (standard_cyborg::algorithms::sliceMesh(geometry, [=](int i, Vec3 p) {
        return p.y - 0.5;
    }));
    
    EXPECT_EQ(polylines.size(), 1);
    EXPECT_EQ(polylines[0].vertexCount(), 4);
    EXPECT_FALSE(polylines[0].isClosed());
}

TEST(MeshSliceTests, testOpenWithDifficultOrderingPolyline) {
    // 3 ----1---- 2
    // |         / | \
    // |  0    /   |   \
    // 2     0     3     5
    // |   /   2   |   1   \
    // | /         |         \
    // 0 ----6---- 1 ----4---- 4
    
    Geometry geometry ({
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {2.0f, 0.0f, 0.0f}
    }, {
        Face3{0, 2, 3},
        Face3{2, 1, 4},
        Face3{0, 1, 2}
    });
    
    std::vector<Polyline> polylines (standard_cyborg::algorithms::sliceMesh(geometry, [](int i, Vec3 p) { return p.y - 0.5; }));
    
    EXPECT_EQ(polylines.size(), 1);
    EXPECT_EQ(polylines[0].vertexCount(), 4);
    EXPECT_FALSE(polylines[0].isClosed());
}

TEST(MeshSliceTests, testSimpleOpenPolylineWithPrecomputedTopology) {
    // 3 ----3---- 2
    // |         / | \
    // |  1    /   |   \
    // 4     2     1     6
    // |   /   0   |   2   \
    // | /         |         \
    // 0 ----0---- 1 ----5---- 4
    
    Geometry geometry (
                       {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {2.0f, 0.0f, 0.0f}},
                       {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
                       {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
                       {{0, 1, 2}, {0, 2, 3}, {1, 4, 2}}
                       );
    
    standard_cyborg::sc3d::MeshTopology::MeshTopology topology (geometry.getFaces());
    
    std::vector<Polyline> polylines (standard_cyborg::algorithms::sliceMesh(geometry, [](int i, Vec3 p) {
        return p.y - 0.5;
    }, topology));
    
    EXPECT_EQ(polylines.size(), 1);
    EXPECT_EQ(polylines[0].vertexCount(), 4);
    EXPECT_FALSE(polylines[0].isClosed());
}

TEST(MeshSliceTests, testSimpleClosedPolyline) {
    
    // 3 ------5------ 2
    // | \           / |
    // |   6   2   4   |
    // |     \   /     |
    // 7  3    4    1  3
    // |     /   \     |
    // |   0   0   2   |
    // | /           \ |
    // 0 ------1------ 1
    
    Geometry geometry (
                       std::vector<Vec3>{{-1.0f, -1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
                       std::vector<Face3>{{4, 0, 1}, {4, 1, 2}, {4, 2, 3}, {4, 3, 0}}
                       );
    
    std::vector<Polyline> polylines (standard_cyborg::algorithms::sliceMesh(geometry, [](int i, Vec3 p) {
        return p.squaredNorm() - 1.0f;
    }));
    
    EXPECT_EQ(polylines.size(), 1);
    EXPECT_EQ(polylines[0].vertexCount(), 5);
    EXPECT_EQ(polylines[0].getPositions(), std::vector<Vec3>({
        {-0.5f, -0.5f, 0.0f},
        { 0.5f, -0.5f, 0.0f},
        { 0.5f,  0.5f, 0.0f},
        {-0.5f,  0.5f, 0.0f},
        {-0.5f, -0.5f, 0.0f}
    }));
    EXPECT_TRUE(polylines[0].isClosed());
}

TEST(MeshSliceTests, testVertexIntersection) {
    Geometry geometry (
                       std::vector<Vec3>{
        {-2.0f, -1.0f, 0.0f},
        {2.0f, -1.0f, 0.0f},
        {2.0f, 1.0f, 0.0f},
        {-2.0f, 1.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f}
    },
                       std::vector<Face3>{
        {0, 1, 5},
        {1, 2, 6},
        {2, 3, 5},
        {0, 4, 3},
        {0, 5, 4},
        {4, 5, 3},
        {1, 6, 5},
        {5, 6, 2}
    }
                       );
    
    std::vector<Polyline> polylines (standard_cyborg::algorithms::sliceMesh(geometry, [](int i, Vec3 p) {
        return p.y;
    }));
    
    //for (auto polyline : polylines) std::cout<<polyline<<std::endl;
    
    //std::cout<<MeshTopology::MeshTopology(geometry.getFaces());
}

