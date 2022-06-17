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

#include "standard_cyborg/math/Vec3.hpp"

#include "standard_cyborg/algorithms/KMeans.hpp"

namespace standard_cyborg {

namespace algorithms {

struct Vec3KMeansAdaptor {
    typedef float DistanceType;
    typedef standard_cyborg::math::Vec3 PointType;
    
    /*
     * Lloyd's algorithm for k-means only needs distance for the sake of comparison.
     * For this part, we can avoid some square roots.
     */
    static inline float distanceComparisonMetric(const standard_cyborg::math::Vec3& a, const standard_cyborg::math::Vec3& b)
    {
        float d = a.x - b.x;
        float sum = d * d;
        d = a.y - b.y;
        sum += d * d;
        d = a.z - b.z;
        return sum + d * d;
    };
    
    /*
     * k-means++ initialization uses a distribution with respect to distance in order
     * to initialize the solution. For this step we actually need a meaningful distance
     * metric and not just an ordinal metric.
     */
    static inline float distanceMetric(const standard_cyborg::math::Vec3& a, const standard_cyborg::math::Vec3& b)
    {
        float d = a.x - b.x;
        float sum = d * d;
        d = a.y - b.y;
        sum += d * d;
        d = a.z - b.z;
        return std::sqrt(sum + d * d);
    };
};

typedef KMeans<Vec3KMeansAdaptor> Vec3KMeans;

}

}
