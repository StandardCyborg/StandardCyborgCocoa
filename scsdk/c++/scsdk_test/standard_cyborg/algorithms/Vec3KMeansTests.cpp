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

#include "standard_cyborg/algorithms/Vec3KMeans.hpp"

#import <cmath>
#import <vector>

//#import <StandardCyborgData/DataUtils.hpp>
//#import <StandardCyborgData/DebugHelpers.hpp>

#include "standard_cyborg/math/Vec3.hpp"

using standard_cyborg::math::Vec3;

TEST(Vec3KMeansTests, testKMeans) {
    // Create two well-separated clusters
    Vec3 point1 {1.0f, 2.0f, 4.0f};
    Vec3 point2 {-7.0f, -3.0f, 1.0};
    
    int n = 100;
    std::vector<Vec3> positions {};
    for (int i = 0; i < n; i++) {
        float t = i * 1.0 / n;
        positions.push_back(
                            (i % 3 == 0 ? point1 : point2) +
                            Vec3({std::cosf(t * 10.0f) * t, std::sinf(t * 10.0f) * t, 0.0f})
                            );
    }
    
    auto result = standard_cyborg::algorithms::Vec3KMeans::compute(positions, 2);
    
    EXPECT_LT((result.centroids[0] - point2).norm(), 0.2);
    EXPECT_LT((result.centroids[1] - point1).norm(), 0.2);
    EXPECT_EQ(result.assignments.size(), n);
    EXPECT_TRUE(result.converged);
    EXPECT_EQ(result.iterations, 2);
    EXPECT_EQ(result.counts, std::vector<int>({66, 34}));
}

TEST(Vec3KMeansTests, testKMeansWithUnspecifiedK) {
    // Create two well-separated clusters
    Vec3 point1 {1.0f, 2.0f, 4.0f};
    Vec3 point2 {-7.0f, -3.0f, 1.0};
    
    int n = 100;
    std::vector<Vec3> positions {};
    for (int i = 0; i < n; i++) {
        float t = i * 1.0 / n;
        positions.push_back(
                            (i % 3 == 0 ? point1 : point2) +
                            Vec3({std::cosf(t * 10.0f) * t, std::sinf(t * 10.0f) * t, 0.0f})
                            );
    }
    
    auto result = standard_cyborg::algorithms::Vec3KMeans::compute(positions);
    
    EXPECT_EQ(result.centroids.size(), 7);
    EXPECT_EQ(result.assignments.size(), n);
    EXPECT_TRUE(result.converged);
    EXPECT_EQ(result.iterations, 8);
    EXPECT_EQ(result.counts, std::vector<int>({40, 7, 8, 12, 21, 6, 6}));
}
