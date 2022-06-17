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

#pragma once

#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

namespace standard_cyborg {
namespace algorithms {

template <typename PointTypeAdaptor>
class KMeans {
    typedef typename PointTypeAdaptor::DistanceType DistanceType;
    typedef typename PointTypeAdaptor::PointType PointType;
    
    struct Result {
        /* A list (length n) of cluster centers */
        std::vector<PointType> centroids;
        
        /* A list (length k) of the number of points per centroid */
        std::vector<int> counts;
        
        /* A list (length n) of the centroid index to which a particular point is assigned */
        std::vector<int> assignments;
        
        /* Iterations taken to converge */
        int iterations;
        
        /* Whether the algorithm converged successfully or exited (presumably when max iterations
         * was reached
         */
        bool converged;
    };

public:
    /*
     * Perform k-means via Lloyd's algorithm with k-means++ initialization.
     * References:
     *     Lloyd's algorithm: https://en.wikipedia.org/wiki/Lloyd%27s_algorithm
     *     k-means++ initialization: https://en.wikipedia.org/wiki/K-means%2B%2B
     */
    static Result compute(const std::vector<PointType> points, size_t k = 0, size_t maxIterations = 200, int seed = 0)
    {
        int n = static_cast<int>(points.size());
        
        if (k == 0) {
            k = std::sqrt(0.5 * n);
        }

        // Create a random number generator
        std::mt19937 prng(seed);
        std::uniform_int_distribution<int> randint(0, n);
        std::uniform_real_distribution<float> randfloat(0.0f, 1.0f);

        // Allocate results storage
        Result result;
        std::vector<PointType>& centroids = result.centroids;
        std::vector<int>& counts = result.counts;
        std::vector<int>& assignments = result.assignments;
        result.converged = false;

        // Set output sizes
        centroids.resize(k);
        counts.resize(k);
        assignments.resize(n);
        
        std::fill(assignments.begin(), assignments.end(), -1);

        //
        // Perform k-means++ initialization.
        // See: https://en.wikipedia.org/wiki/K-means%2B%2B
        //
        int tries = 2 + static_cast<int>(std::logf(n) + 0.5);
        std::vector<DistanceType> distances(n);
        std::vector<DistanceType> tmpDistances(n);

        //
        // 1. Choose one center uniformly at random from the data points.
        //
        PointType p = centroids[0] = points[randint(prng)];
        assignments[0] = 0;

        //
        // 2. For each data point x, compute D(x), the distance between x and
        //    the nearest center that has already been chosen.
        //
        DistanceType distanceSum = 0.0;
        for (int i = 0; i < n; i++) {
            float distance = PointTypeAdaptor::distanceMetric(p, points[i]);
            distances[i] = distance;
            distanceSum += distance;
        }
        
        //
        // 3. Choose one new data point at random as a new center, using a
        //    weighted probability distribution where a point x is chosen with
        //    probability proportional to D(x)^2. (Repeated until k centers
        //    have been chosen.)
        //
        for (int i = 1; i < k; i++) {
            float bestDistanceSum = INFINITY;
            int bestIndex = -1;
            
            int l = 0;
            for (int j = 0; j < tries; j++) {
                DistanceType randomValue = randfloat(prng) * distanceSum;
                for (l = 0; l < n; l++) {
                    if (randomValue <= distances[l]) {
                        break;
                    } else {
                        randomValue -= distances[l];
                    }
                }
            }
            
            DistanceType tmpDistanceSum = 0.0;
            PointType referencePoint(points[l]);
            for (int m = 0; m < n; m++) {
                DistanceType cmp1 = distances[m];
                DistanceType cmp2 = PointTypeAdaptor::distanceMetric(points[m], referencePoint);
                tmpDistances[m] = cmp1 > cmp2 ? cmp2 : cmp1;
                tmpDistanceSum += tmpDistances[m];
            }
            
            if (tmpDistanceSum < bestDistanceSum) {
                bestDistanceSum = tmpDistanceSum;
                bestIndex = l;
            }
            
            distanceSum = bestDistanceSum;
            
            centroids[i] = points[bestIndex];
            assignments[i] = i;

            for (int j = 0; j < n; j++) {
                DistanceType cmp2 = PointTypeAdaptor::distanceMetric(points[bestIndex], points[j]);
                if (distances[j] > cmp2) {
                    distances[j] = cmp2;
                }
            }
        }
        
        // Iterate using Lloyd's algorithm.
        // See: https://en.wikipedia.org/wiki/Lloyd%27s_algorithm
        //
        bool converged = false;
        int iteration = 0;
        while (!converged && iteration++ < maxIterations) {
            converged = true;
            
            std::fill(counts.begin(), counts.end(), 0);
            
            for (int i = 0; i < n; i++) {
                int bestIndex = 0;
                DistanceType minDist = PointTypeAdaptor::distanceComparisonMetric(centroids[0], points[i]);
                
                for (int j = 1; j < k; j++) {
                    DistanceType dist = PointTypeAdaptor::distanceComparisonMetric(centroids[j], points[i]);
                    if (dist < minDist) {
                        minDist = dist;
                        bestIndex = j;
                    }
                }
                
                // If assignment has changed, then it has not converged
                if (assignments[i] == -1 || assignments[i] != bestIndex) {
                    converged = false;
                }
                
                assignments[i] = bestIndex;
                counts[bestIndex]++;
            }
            
            std::fill(centroids.begin(), centroids.end(), PointType{});
            
            for (int i = 0; i < n; i++) {
                int index = assignments[i];
                centroids[index] += points[i];
            }
            
            for (int i = 0; i < k; i++) {
                if (counts[i] > 0) {
                    centroids[i] *= static_cast<DistanceType>(1.0 / counts[i]);
                }
            }
        }
        
        result.iterations = iteration;
        result.converged = iteration < maxIterations;

        return result;
    }
};

} // namespace algorithms
} // namespace standard_cyborg
