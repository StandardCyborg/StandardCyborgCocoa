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

#include "standard_cyborg/algorithms/Centroid.hpp"

#include "standard_cyborg/sc3d/Face3.hpp"
#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/sc3d/Polyline.hpp"
#include "standard_cyborg/math/Vec3.hpp"
#include "standard_cyborg/sc3d/VertexSelection.hpp"

namespace standard_cyborg {
namespace algorithms {

using math::Vec3;
using namespace standard_cyborg::sc3d;

Vec3 computeCentroid(const std::vector<Vec3>& positions)
{
    int n = static_cast<int>(positions.size());
    Vec3 centroidSum{0.0f, 0.0f, 0.0f};
    
    for (int i = 0; i < n; i++) {
        centroidSum += positions[i];
    }
    
    return centroidSum / n;
}

Vec3 computeCentroid(const std::vector<Vec3>& positions, const VertexSelection& selection)
{
    Vec3 centroidSum{0.0f, 0.0f, 0.0f};

    for (auto index : selection) {
        centroidSum += positions[index];
    }
    
    return centroidSum / selection.size();
}

Vec3 computeCentroid(const Polyline& polyline)
{
    Vec3 centroidSum{0.0f, 0.0f, 0.0f};
    float lengthSum = 0.0f;
    std::vector<Vec3> positions = polyline.getPositions();
    
    Vec3 positionA = positions[0];
    Vec3 positionB;
    
    for (int i = 1; i < positions.size(); i++) {
        positionB = positions[i];
        float length = (positionB - positionA).norm();
        Vec3 centroid = 0.5 * (positionA + positionB);
        lengthSum += length;
        centroidSum += centroid * length;
        
        positionA = positionB;
    }
    return centroidSum / lengthSum;
}

Vec3 computeCentroid(const Geometry& geometry)
{
    Vec3 centroidSum{0.0f, 0.0f, 0.0f};
    
    if (!geometry.hasPositions()) {
        return centroidSum;
    }
    
    const std::vector<Vec3>& positions = geometry.getPositions();
    
    if (geometry.hasFaces()) {
        const std::vector<Face3>& faces = geometry.getFaces();
        float areaSum = 0.0f;
        for (int faceIndex = 0; faceIndex < faces.size(); faceIndex++) {
            const Face3& face = faces[faceIndex];
            const Vec3& pA = positions[face[0]];
            const Vec3& pB = positions[face[1]];
            const Vec3& pC = positions[face[2]];
            
            // Strictly speaking this is twice the area, but it's just a weight and so
            // a constant in front (e.g. 0.5) makes no difference here
            float area = Vec3::cross(pB - pA, pC - pA).norm();
            Vec3 centroid = (pA + pB + pC) * (1.0 / 3.0);
            
            centroidSum += area * centroid;
            areaSum += area;
        }
        
        return centroidSum / areaSum;
    }
    
    // If there are no faces, return the centroid of the positions
    return computeCentroid(positions);
}

} // namespace algorithms
} // namespace standard_cyborg
