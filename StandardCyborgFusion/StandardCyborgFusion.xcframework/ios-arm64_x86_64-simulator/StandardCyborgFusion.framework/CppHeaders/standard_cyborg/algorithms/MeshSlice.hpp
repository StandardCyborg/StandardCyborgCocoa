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

#include "standard_cyborg/sc3d/MeshTopology.hpp"

#include <functional>
#include <vector>

namespace standard_cyborg {

namespace sc3d {
class Geometry;
struct Plane;
class Polyline;
}

namespace algorithms {

std::vector<sc3d::Polyline> sliceMesh(const sc3d::Geometry& geometry,
                                const std::function<float(int index, math::Vec3 position)>& isolevelFunction,
                                const sc3d::MeshTopology::MeshTopology& optionalTopology = sc3d::MeshTopology::MeshTopology());

std::vector<sc3d::Polyline> sliceMesh(const sc3d::Geometry& geometry,
                                const sc3d::Plane& plane,
                                const sc3d::MeshTopology::MeshTopology& optionalTopology = sc3d::MeshTopology::MeshTopology());

}

} // namespace StandardCyborg
