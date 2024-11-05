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

#include "standard_cyborg/algorithms/Centroid.hpp"
#include "standard_cyborg/math/Vec3.hpp"
#include "standard_cyborg/sc3d/Face3.hpp"
#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/sc3d/Polyline.hpp"

#include <vector>

namespace math = standard_cyborg::math;
using math::Vec3;


TEST(CentroidTests, testVectorOfVec3Centroid) {
    std::vector<Vec3> positions {
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 2.0f},
        {0.0f, 2.0f, 4.0f},
        {2.0f, 5.0f, 4.0f},
    };
    
    Vec3 centroid = standard_cyborg::algorithms::computeCentroid(positions);
    
    EXPECT_NEAR(centroid.x, 3.0f / 4.0f, FLT_EPSILON);
    EXPECT_NEAR(centroid.y, 7.0f / 4.0f, FLT_EPSILON);
    EXPECT_NEAR(centroid.z, 5.0f / 2.0f, FLT_EPSILON);
}

TEST(CentroidTests, testGeometryCentroidWithFaces) {
    std::vector<Vec3> positions {
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 2.0f},
        {0.0f, 2.0f, 4.0f},
        {2.0f, 5.0f, 4.0f},
    };
    
    std::vector<standard_cyborg::sc3d::Face3> faces {
        { 0, 1, 2 },
        { 0, 1, 3 }
    };
    
    standard_cyborg::sc3d::Geometry geometry (positions, faces);
    
    Vec3 centroid = standard_cyborg::algorithms::computeCentroid(geometry);
    
    EXPECT_NEAR(centroid.x, 0.767176f, 1e-6f);
    EXPECT_NEAR(centroid.y, 1.317430f, 1e-6f);
    EXPECT_NEAR(centroid.z, 2.0f, 1e-6f);
}


TEST(CentroidTests, testGeometryCentroidWithoutFaces) {
    std::vector<Vec3> positions {
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 2.0f},
        {0.0f, 2.0f, 4.0f},
        {2.0f, 5.0f, 4.0f},
    };
    
    standard_cyborg::sc3d::Geometry geometry (positions);
    
    Vec3 centroid = standard_cyborg::algorithms::computeCentroid(geometry);
    
    // Reduces to the equivalent above result for a point cloud
    EXPECT_NEAR(centroid.x, 3.0f / 4.0f, FLT_EPSILON);
    EXPECT_NEAR(centroid.y, 7.0f / 4.0f, FLT_EPSILON);
    EXPECT_NEAR(centroid.z, 5.0f / 2.0f, FLT_EPSILON);
}

TEST(CentroidTests, testGeometryCentroidEmpty) {
    std::vector<Vec3> positions { };
    
    standard_cyborg::sc3d::Geometry geometry (positions);
    
    Vec3 centroid = standard_cyborg::algorithms::computeCentroid(geometry);
    
    // Reduces to the equivalent above result for a point cloud
    EXPECT_NEAR(centroid.x, 0.0f, FLT_EPSILON);
    EXPECT_NEAR(centroid.y, 0.0f, FLT_EPSILON);
    EXPECT_NEAR(centroid.z, 0.0f, FLT_EPSILON);
}

TEST(CentroidTests, testCentroidPointCloud) {
    std::vector<Vec3> positions {
        {+1.0f, +2.0f, +3.0f},
        {+6.0f, +7.0f, +8.0f},
        {-2.0f, -1.0f, -6.0f},
        {-9.0f, +4.0f, +1.0f},
    };
    
    standard_cyborg::sc3d::Geometry tri(positions);
    
    Vec3 centroid = standard_cyborg::algorithms::computeCentroid(tri.getPositions());
    
    EXPECT_NEAR(centroid.x, -4.0f / 4.0f, FLT_EPSILON);
    EXPECT_NEAR(centroid.y, 12.0f / 4.0f, FLT_EPSILON);
    EXPECT_NEAR(centroid.z, 6.00f / 4.0f, FLT_EPSILON);
}

TEST(CentroidTests, testPolylineCentroid) {
    // By symmetry this set of positions has the centroid at the origin if interpreted
    // as a closed curve but not if interpreted as a point cloud (due to the repeated
    // endpoint)
    std::vector<Vec3> positions {
        {-2.0f, -2.0f, 0.0f},
        { 1.0f, -1.0f, 0.0f},
        { 2.0f,  2.0f, 0.0f},
        {-1.0f,  1.0f, 0.0f},
        {-2.0f, -2.0f, 0.0f}
    };
    
    standard_cyborg::sc3d::Polyline curve (positions);
    
    Vec3 centroid = standard_cyborg::algorithms::computeCentroid(curve);
    
    EXPECT_NEAR(centroid.x, 0.0f, FLT_EPSILON);
    EXPECT_NEAR(centroid.y, 0.0f, FLT_EPSILON);
    EXPECT_NEAR(centroid.z, 0.0f, FLT_EPSILON);
}

TEST(CentroidTests, testCentroidPointCloudSelection) {
    std::vector<Vec3> positions {
        {+1.0f, +2.0f, +3.0f},
        {+6.0f, +7.0f, +8.0f},
        {-2.0f, -1.0f, -6.0f},
        {-9.0f, +4.0f, +1.0f},
    };
    
    standard_cyborg::sc3d::Geometry points(positions);
    
    Vec3 centroid = standard_cyborg::algorithms::computeCentroid(points.getPositions(), standard_cyborg::sc3d::VertexSelection(4, {1, 3}));
    
    EXPECT_NEAR(centroid.x, -3.0f / 2.0f, FLT_EPSILON);
    EXPECT_NEAR(centroid.y, 11.0f / 2.0f, FLT_EPSILON);
    EXPECT_NEAR(centroid.z, 9.0f / 2.0f, FLT_EPSILON);
}
