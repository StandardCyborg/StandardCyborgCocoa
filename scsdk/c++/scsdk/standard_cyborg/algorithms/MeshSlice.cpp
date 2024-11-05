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

#include "standard_cyborg/algorithms/MeshSlice.hpp"

#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/sc3d/Plane.hpp"
#include "standard_cyborg/sc3d/Polyline.hpp"
#include "standard_cyborg/sc3d/MeshTopology.hpp"

#include <unordered_map>

#include <list>
#include <algorithm>
#include <set>
#include <unordered_map>

#define MAX_EDGE_MARCH_STEPS 100000

namespace standard_cyborg {

using math::Vec3;
using sc3d::Polyline;
using sc3d::Geometry;
using sc3d::Plane;

namespace algorithms {

std::vector<sc3d::Polyline> sliceMesh(const sc3d::Geometry& geometry,
                                const sc3d::Plane& plane,
                                const sc3d::MeshTopology::MeshTopology& topology)
{
    float offset = math::Vec3::dot(plane.position, plane.normal);
    math::Vec3 normal = plane.normal;
    
    // clang-format off
    return sliceMesh(geometry, [offset, normal](int index, math::Vec3 position) {
        return math::Vec3::dot(position, normal) - offset;
    }, topology);
    // clang-format on
}

std::vector<sc3d::Polyline> sliceMesh(const sc3d::Geometry& geometry,
                                const std::function<float(int index, math::Vec3 position)>& isolevelFunction,
                                const sc3d::MeshTopology::MeshTopology& topology)
{
    const std::vector<Vec3> positions = geometry.getPositions();
    const std::vector<sc3d::Face3>& faces = geometry.getFaces();
    
    // Convert lists of positions to Polylines (std::vector)
    std::vector<Polyline> outputPolylines;
    
    // Not ideal but I don't know how to make topology an optional argument without making it const.
    // The only case where we'd modify the topology is if it's not provided at all so that if you
    // provide topology, it does in fact remain const. Improvements here are welcomed.
    sc3d::MeshTopology::MeshTopology& nonConstTopology = const_cast<sc3d::MeshTopology::MeshTopology&>(topology);
    
    if (topology.getVertexEdges().size() != geometry.vertexCount()) {
        nonConstTopology.compute(geometry.getFaces());
    }
    
    const std::vector<sc3d::MeshTopology::Edge>& edges = nonConstTopology.getEdges();
    const std::vector<sc3d::MeshTopology::FaceEdges>& faceEdges = nonConstTopology.getFaceEdges();
    
    int numEdges = static_cast<int>(edges.size());
    
    std::vector<bool> edgeWasVisited(numEdges);
    std::fill(edgeWasVisited.begin(), edgeWasVisited.end(), false);
    
    // Trace out toward face0
    std::unordered_map<int, std::list<Vec3>> curveStartingAtEdge;
    std::list<Vec3> currentPolyline;

    for (int seedEdgeIndex = 0; seedEdgeIndex < numEdges; seedEdgeIndex++) {
        if (edgeWasVisited[seedEdgeIndex]) continue;
        bool closedCurveFound = false;
        
        for (int startingDirection = 0; startingDirection < 2; startingDirection++) {
            if (closedCurveFound) break;
            
            // Initialize the marching values
            const sc3d::MeshTopology::Edge& edge = edges[seedEdgeIndex];
            
            int edgeIndex0 = startingDirection == 0 ? edge.vertex0 : edge.vertex1;
            int edgeIndex1 = startingDirection == 0 ? edge.vertex1 : edge.vertex0;
            
            Vec3 position0 = positions[edgeIndex0];
            Vec3 position1 = positions[edgeIndex1];
            float value0 = isolevelFunction(edgeIndex0, position0);
            float value1 = isolevelFunction(edgeIndex1, position1);
            int currentEdgeIndex = seedEdgeIndex;
    
            // Note: starting off the marching in the correct direction with respect to the
            // winding number depends upon this line
            int previousFaceIndex = startingDirection == 0 ? edge.face1 : edge.face0;
    
            // We neglect entirely the case where either endpoint is exactly equal to zero!!
            // This is a reasonable enough assumption to get us off the ground with real world
            // scan data where that will never be the case, but clearly needs to be addressed
            // in order to make this function robust.
            if ((value0 >= 0.0f && value1 >= 0.0f) || (value0 <= 0.0f && value1 <= 0.0f)) {
                edgeWasVisited[seedEdgeIndex] = true;
                continue;
            }
            
            if (startingDirection == 0) {
                // Push this point when starting marching in the first direction but not the second
                currentPolyline.push_back(Vec3::lerp(position0, position1, value0 / (value0 - value1)));
            }
            
            int step = 0;
            while (step++ < MAX_EDGE_MARCH_STEPS) {
                edgeWasVisited[currentEdgeIndex] = true;
                const sc3d::MeshTopology::Edge& currentEdge = edges[currentEdgeIndex];
    
                // Step across the previous edge onto the current face
                int currentFaceIndex = currentEdge.face0 == previousFaceIndex ? currentEdge.face1 : currentEdge.face0;

                // If we're at a boundary, we can't step any farther.
                if (currentFaceIndex < 0) break;

                const sc3d::Face3& currentFace = faces[currentFaceIndex];
                const sc3d::MeshTopology::FaceEdges& currentFaceEdges = faceEdges[currentFaceIndex];

                // Figure out the third vertex index. To do this, look up which edge we're on,
                // then move two positions around to get the opposite vertex
                int vertexIndex2 = currentFace[(currentFaceEdges.offsetOf(currentEdgeIndex) + 2) % 3];
                Vec3 position2 = positions[vertexIndex2];
                float value2 = isolevelFunction(vertexIndex2, position2);

                bool candidateEdge12InteriorHit = value1 * value2 < 0.0f;
                bool candidateEdge02InteriorHit = value0 * value2 < 0.0f;
    
                // If both edges are a hit, then something has gone wrong
                if (candidateEdge12InteriorHit && candidateEdge02InteriorHit) break;

                // Get both candidate edges and mark them visited. Visiting doesn't affect this
                // marching, but it will affect subsequent starts.
                int candidateEdge12Index = currentFaceEdges.edgeIndexAfter(currentEdgeIndex);
                int candidateEdge02Index = currentFaceEdges.edgeIndexBefore(currentEdgeIndex);
                edgeWasVisited[candidateEdge12Index] = true;
                edgeWasVisited[candidateEdge02Index] = true;

                if (candidateEdge12InteriorHit) {
                    value0 = value2;
                    position0 = position2;
                    currentEdgeIndex = candidateEdge12Index;
                } else if (candidateEdge02InteriorHit) {
                    value1 = value2;
                    position1 = position2;
                    currentEdgeIndex = candidateEdge02Index;
                }
                previousFaceIndex = currentFaceIndex;

                // Interpolate linearly and push the point along the edge onto the current curve
                Vec3 newPosition(Vec3::lerp(position0, position1, value0 / (value0 - value1)));
                if (startingDirection == 0) {
                    // Marchin in the first direction, we append
                    currentPolyline.push_back(newPosition);
                } else {
                    // Marchin in the second direction, we prepend
                    currentPolyline.push_front(newPosition);
                }
                
                // If we're back to where we started, bail
                if (currentEdgeIndex == seedEdgeIndex) {
                    closedCurveFound = true;
                    break;
                }
            }
        }
        
        if (currentPolyline.size()) {
            outputPolylines.push_back(std::vector<Vec3>({currentPolyline.begin(), currentPolyline.end()}));
            currentPolyline = std::list<Vec3>{};
        }
    }

    return outputPolylines;
}

}

} // namespace StandardCyborg
