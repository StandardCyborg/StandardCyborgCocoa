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


#include "standard_cyborg/sc3d/MeshTopology.hpp"
#include "standard_cyborg/util/AssertHelper.hpp"

namespace standard_cyborg {
namespace sc3d {
namespace MeshTopology {

MeshTopology::MeshTopology(const std::vector<Edge>& edges,
                           const std::vector<FaceEdges>& faceEdges,
                           const std::vector<VertexEdges>& vertexEdges) :
    _edges(edges),
    _faceEdges(faceEdges),
    _vertexEdges(vertexEdges)
{}


MeshTopology::MeshTopology(const std::vector<Face3>& faces)
{
    compute(faces);
}

void MeshTopology::compute(const std::vector<Face3>& faces)
{
    int numFaces = static_cast<int>(faces.size());
    
    // We don't know in advance how many vertices there will be. Instead, we assume vertex
    // numbering starts at zero and check as we iterate through the face data.
    //
    // We reserve a reasonable amount of space for per-vertex edges lists to reduce incremental
    // resizing. The Euler characteristic of a surface relates the number of faces to the number
    // of edges and vertices. For a triangular mesh (see:
    // https://math.stackexchange.com/questions/425968/eulers-formula-for-triangle-mesh ):
    //
    //   > The number of triangles is twice the number of vertices F ~ 2V
    //
    // Since it's not *exactly* half, we overestimate just a little to prevent resizing a large
    // array of vertex data right before we're done.
    _vertexEdges.reserve(static_cast<int>(numFaces * 0.52));
    
    //std::cout<<"topology estimated capcity = "<<_vertexEdges.capacity()<<std::endl;
    
    // We know exactly the required length here
    _faceEdges.resize(numFaces);
    
    for (int faceId = 0; faceId < numFaces; faceId++) {
        Face3 face = faces[faceId];

        int vertexA = face[0];
        int vertexB = face[1];
        int vertexC = face[2];
        
        // Resize vertexEdges, if necessary
        int maxVertexId = vertexA > vertexB ? (vertexA > vertexC ? vertexA : vertexC) : (vertexC > vertexB ? vertexC : vertexB);
        if (maxVertexId >= _vertexEdges.size()) {
            _vertexEdges.resize(maxVertexId + 1);
        }
        
        // The main task in this code is to find the edge indices:
        int edgeAB = -1;
        int edgeBC = -1;
        int edgeCA = -1;

        // Trace from vertex A to its edges and look for an existing edge with
        // either vertex B or vertex C as the other vertices.
        for (auto edgeIndex : _vertexEdges[vertexA]) {
            Edge& edge = _edges[edgeIndex];

            // Check for an edge that matches either AB or BA
            if ((edge.vertex0 == vertexA && edge.vertex1 == vertexB) || (edge.vertex0 == vertexB && edge.vertex1 == vertexA)) {
                edgeAB = edgeIndex;
                if (edgeCA != -1) break;
            }
            
            // Similarly, check for an edge matching AC or CA
            if ((edge.vertex0 == vertexA && edge.vertex1 == vertexC) || (edge.vertex0 == vertexC && edge.vertex1 == vertexA)) {
                edgeCA = edgeIndex;
                if (edgeAB != -1) break;
            }
        }
        
        for (auto edgeIndex : _vertexEdges[vertexB]) {
            Edge& edge = _edges[edgeIndex];

            // The procedure for vertex B's edges is slightly different. Since we've checked
            // A-C and A-B. we now only need to check for matches with edge B-C.
            if ((edge.vertex0 == vertexB && edge.vertex1 == vertexC) || (edge.vertex0 == vertexC && edge.vertex1 == vertexB)) {
                edgeBC = edgeIndex;
                break;
            }
        }
        
        // Either create new edges or set the second face of an existing edge based on what was found
        if (edgeAB == -1) {
            edgeAB = static_cast<int>(_edges.size());
            _edges.push_back({vertexA, vertexB, faceId, -1});
        } else {

            SCASSERT(_edges[edgeAB].face1 == -1, "Non-manifold edge found in mesh topology");
            _edges[edgeAB].face1 = faceId;
        
        }
        
        if (edgeBC == -1) {
            edgeBC = static_cast<int>(_edges.size());
            _edges.push_back({vertexB, vertexC, faceId, -1});
        } else {
            SCASSERT(_edges[edgeBC].face1 == -1, "Non-manifold edge found in mesh topology");
            _edges[edgeBC].face1 = faceId;
        }
        
        if (edgeCA == -1) {
            edgeCA = static_cast<int>(_edges.size());
            _edges.push_back({vertexC, vertexA, faceId, -1});
        } else {
            SCASSERT(_edges[edgeCA].face1 == -1, "Non-manifold edge found in mesh topology");
            _edges[edgeCA].face1 = faceId;
        }
        
        // Set the three edges for this face
        _faceEdges[faceId][0] = edgeAB;
        _faceEdges[faceId][1] = edgeBC;
        _faceEdges[faceId][2] = edgeCA;
        
        // Add (if not already added) these edges to vertexEdges' adjacency lists
        _vertexEdges[vertexA].insert(edgeCA);
        _vertexEdges[vertexA].insert(edgeAB);
        
        _vertexEdges[vertexB].insert(edgeAB);
        _vertexEdges[vertexB].insert(edgeBC);
        
        _vertexEdges[vertexC].insert(edgeBC);
        _vertexEdges[vertexC].insert(edgeCA);
    }
    
    //std::cout<<"topology actual size = "<<_vertexEdges.size()<<std::endl;
}

const std::vector<Edge> MeshTopology::getEdges() const
{
    return _edges;
}

const std::vector<FaceEdges> MeshTopology::getFaceEdges() const
{
    return _faceEdges;
}

const std::vector<VertexEdges> MeshTopology::getVertexEdges() const
{
    return _vertexEdges;
}

int MeshTopology::getNumEdges() const
{
    return static_cast<int>(_edges.size());
}

int MeshTopology::getNumFaceEdges() const
{
    return static_cast<int>(_faceEdges.size());
}

int MeshTopology::getNumVertexEdges() const
{
    return static_cast<int>(_vertexEdges.size());
}

} // namespace standard_cyborg::MeshTopology
} // namespace sc3d
} // namespace standard_cyborg
