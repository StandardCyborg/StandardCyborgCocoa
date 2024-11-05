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

#include "standard_cyborg/math/Mat3x4.hpp"
#include "standard_cyborg/math/Mat3x3.hpp"
#include "standard_cyborg/sc3d/Geometry.hpp"

#include "standard_cyborg/algorithms/TransformGeometry.hpp"


namespace standard_cyborg {
namespace algorithms {

void transformGeometry(const sc3d::Geometry& geometry, const math::Mat3x4& mat)
{
    int vertexCount = geometry.vertexCount();
    
    if (geometry.hasPositions()) {
        std::vector<math::Vec3>& positions = const_cast<std::vector<math::Vec3>&>(geometry.getPositions());
        
        for (int i = 0; i < vertexCount; ++i) {
            positions[i] = mat * positions[i];
        }
    }
    
    if (geometry.hasNormals()) {
        math::Mat3x3 normalMatrix(math::Mat3x3::normalMatrix(mat));
        std::vector<math::Vec3>& normals = const_cast<std::vector<math::Vec3>&>(geometry.getNormals());
        
        for (int i = 0; i < vertexCount; ++i) {
            normals[i] = normalMatrix * normals[i];
        }
    }
}

} // namespace algorithms
} // namespace StandardCyborg
