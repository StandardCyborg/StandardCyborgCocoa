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

#include <vector>

namespace standard_cyborg {

namespace math {
struct Vec3;
}

namespace algorithms {

class DBScan {
public:
    /*
     * Perform dbscan
     * References:
     *     github (heavily refactored from): https://github.com.cnpmjs.org/bowbowbow/DBSCAN
     
     For explanation of the meaning of minPoints(minPts) and epsilon, see
     https://en.wikipedia.org/wiki/DBSCAN
     
     Returns: A list (length n) of the centroid index to which a particular point is assigned
     -1, means that the point was classified as noise.
     */
    static std::vector<int> compute(const std::vector<math::Vec3>& points,
                                    size_t minPoints = 200,
                                    float epsilon = 0.5);
};

}

}
