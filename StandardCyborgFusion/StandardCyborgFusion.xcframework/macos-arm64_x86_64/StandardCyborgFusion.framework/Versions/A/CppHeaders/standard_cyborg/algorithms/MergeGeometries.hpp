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

#include <memory>

namespace standard_cyborg {

namespace sc3d {
class Geometry;

}

namespace algorithms {

/** Creates a new Geometry that is the result of merging `second` into `first`.
    Points in `second` that are < maxMergeDistance from `first` will be averaged in,
    otherwise they will be added without modification.
 
    Only works on point clouds, i.e. only the position, normal, and color attributes
    of the incoming Geometry instances. */
std::unique_ptr<standard_cyborg::sc3d::Geometry> mergeGeometries(const standard_cyborg::sc3d::Geometry& first,
                                          const standard_cyborg::sc3d::Geometry& second,
                                          float maxMergeDistanceMeters);

}

}
