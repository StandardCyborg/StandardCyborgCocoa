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

#include "standard_cyborg/algorithms/EstimatePlane.hpp"
#include "standard_cyborg/sc3d/Plane.hpp"
#include "standard_cyborg/sc3d/Face3.hpp"
#include "standard_cyborg/util/DataUtils.hpp"

#include <random>
#include <iostream>

using standard_cyborg::sc3d::Geometry;
using standard_cyborg::sc3d::Polyline;
using standard_cyborg::sc3d::Face3;

using namespace standard_cyborg;
using math::Vec3;

TEST(PlaneEstimationTests, testEstimatePlane) {
    std::vector<Vec3> positions {
        {4.0f, 2.0f, 0.2f},
        {3.0f, 3.0f, 0.2f},
        {2.0f, 2.0f, 0.1f},
        {3.0f, 1.0f, 0.3f},
        {3.0f, 1.0f, 0.1f},
        {3.0f, 1.1f, 0.0f},
        {3.1f, 1.0f, 0.4f},
    };
    
    // We need to make this test deterministic across platforms, and it covers
    // a very small set of points, so we seed the initial set manually.
    standard_cyborg::sc3d::VertexSelection planeVertices;
    planeVertices.insertValue(3);
    planeVertices.insertValue(4);
    planeVertices.insertValue(6);
    standard_cyborg::algorithms::EstimatePlaneResult result = standard_cyborg::algorithms::estimatePlane(positions, planeVertices);
    
    EXPECT_NEAR(result.plane.position.x, 3.03, 0.1);
    EXPECT_NEAR(result.plane.position.y, 1.0, 0.1);
    EXPECT_NEAR(result.plane.position.z, 0.266, 0.1);
    
    // Dot the result with the expected value and compare the absolute value to 1 to ensure they're roughly identical.
    EXPECT_NEAR(std::abs(standard_cyborg::toVector3f(result.plane.normal).transpose() * Eigen::Vector3f(0.0, 1.0, 0.0)), 1.0, 0.01);
    
    // It's a bit of a degenerate case since there aren't more points. It collapses to a plane
    // and the error is almost (but not exactly) zero
    EXPECT_NEAR(result.rmsProjectedDistance, 0.0f, 1e-6);
    
    EXPECT_TRUE(result.converged);
    
    EXPECT_TRUE(*result.planeVertices == standard_cyborg::sc3d::VertexSelection(7, {3, 4, 6}));
}

TEST(PlaneEstimationTests, testEstimatePlaneFromPointCloud) {
    std::vector<Vec3> positions;
    int numPts = 10000;
    
    std::uniform_real_distribution<double> uniform(-1.0, 1.0);
    std::default_random_engine re;
    re.seed(0);
    
    Vec3 position(uniform(re), uniform(re), uniform(re));
    Vec3 normal(Vec3(uniform(re), uniform(re), uniform(re)).normalize());
    Vec3 tangent(Vec3::cross(Vec3(1, 0 ,0), normal).normalize());
    Vec3 bitangent(Vec3::cross(normal, tangent).normalize());
    
    for (int i = 0; i < numPts; i++) {
        positions.push_back(position
                            + normal * uniform(re) * 0.3
                            + tangent * uniform(re)
                            + bitangent * uniform(re));
    }
    
    standard_cyborg::algorithms::EstimatePlaneResult result = standard_cyborg::algorithms::estimatePlane(positions);
    
    float projectedDistanceFromActualPositionToEstimatedPlane = std::abs(Vec3::dot(result.plane.normal, position - result.plane.position));
    
    EXPECT_LT(projectedDistanceFromActualPositionToEstimatedPlane, 1e-3);
    
    // It could go either way, and this happens to detect the opposite normal. I believe maybe this
    // is determined by the relative orientation of the first and second left singular vectors, but
    // it's pretty irrelevant (things shouldn't depend on this!) so I've simply negated.
    EXPECT_LT(Vec3::angleBetween(-result.plane.normal, normal), 1e-2);
}
