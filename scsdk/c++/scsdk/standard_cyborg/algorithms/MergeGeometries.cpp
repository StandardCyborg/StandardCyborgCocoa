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

#include "standard_cyborg/algorithms/MergeGeometries.hpp"

#include "standard_cyborg/sc3d/Geometry.hpp"

using standard_cyborg::math::Vec3;
using standard_cyborg::sc3d::Geometry;


namespace standard_cyborg {

namespace algorithms {


/** Averages two normals together, using the normalized distances for the direction,
    but using the average of the original lengths for the resulting length */
static Vec3 mergeNormals(Vec3 normal1, Vec3 normal2)
{
    // Compute the length of the exising surfel normal
    float normal1Length = normal1.norm();
    float normal2Length = normal2.norm();
    Vec3 normalizedNormal1 = normal1 / normal1Length;
    Vec3 normalizedNormal2 = normal2 / normal2Length;
    
    // Use the average of the normal length as the target length
    float targetNormalLength = 0.5 * (normal1Length + normal2Length);
    
    // Average the normalized normals to compute the new direction
    Vec3 newNormal = 0.5 * (normalizedNormal1 + normalizedNormal2);
    newNormal.normalize();
    newNormal *= targetNormalLength;
    
    return newNormal * targetNormalLength;
}

std::unique_ptr<Geometry> mergeGeometries(const Geometry& first,
                                                          const Geometry& second,
                                                          float maxMergeDistance)
{
    const float maxMergeDistanceSquared = maxMergeDistance * maxMergeDistance;
    
    auto firstPositions = first.getPositions();
    auto firstNormals = first.getNormals();
    auto firstColors = first.getColors();
    
    auto secondVertexCount = second.vertexCount();
    auto secondPositions = second.getPositions();
    auto secondNormals = second.getNormals();
    auto secondColors = second.getColors();
    
    // Since we're merging the second into the first, we start by adding the first to the result
    std::vector<Vec3> resultPositions = firstPositions;
    std::vector<Vec3> resultNormals = firstNormals;
    std::vector<Vec3> resultColors = firstColors;
    
    
    for (int secondVertexIndex = 0; secondVertexIndex < secondVertexCount; ++secondVertexIndex) {
        Vec3 secondPosition = secondPositions[secondVertexIndex];
        Vec3 secondNormal = secondNormals[secondVertexIndex];
        Vec3 secondColor = secondColors[secondVertexIndex];
        
        int firstVertexIndex = first.getClosestVertexIndex(secondPosition);
        Vec3 firstPosition = firstPositions[firstVertexIndex];
        
        if (Vec3::squaredDistanceBetween(firstPosition, secondPosition) > maxMergeDistanceSquared) {
            resultPositions.push_back(secondPosition);
            resultNormals.push_back(secondNormal);
            resultColors.push_back(secondColor);
        } else {
            Vec3 firstNormal = firstNormals[firstVertexIndex];
            Vec3 firstColor = firstColors[firstVertexIndex];
            
            Vec3 mergedPosition = 0.5 * (firstPosition + secondPosition);
            Vec3 mergedNormal = mergeNormals(firstNormal, secondNormal);
            Vec3 mergedColor = 0.5 * (firstColor + secondColor);
            
            resultPositions.push_back(mergedPosition);
            resultNormals.push_back(mergedNormal);
            resultColors.push_back(mergedColor);
        }
    }
    
    return std::make_unique<Geometry>(resultPositions, resultNormals, resultColors);
}

}

} // namespace standard_cyborg
