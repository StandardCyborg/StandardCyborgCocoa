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

namespace sc3d {
class Geometry;
class VertexSelection;
class Polyline;
}

namespace algorithms {

math::Vec3 computeCentroid(const std::vector<math::Vec3>& positions);
math::Vec3 computeCentroid(const std::vector<math::Vec3>& positions, const sc3d::VertexSelection& selection);
math::Vec3 computeCentroid(const sc3d::Geometry& geometry);
math::Vec3 computeCentroid(const sc3d::Polyline& polyline);

} // algorithms
} // namespace standard_cyborg
