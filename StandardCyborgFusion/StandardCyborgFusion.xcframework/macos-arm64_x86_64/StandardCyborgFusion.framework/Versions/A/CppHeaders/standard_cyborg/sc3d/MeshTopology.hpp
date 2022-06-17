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

#include <array>
#include <set>
#include <vector>

#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/sc3d/Face3.hpp"

namespace standard_cyborg {
namespace sc3d {
namespace MeshTopology {

struct Edge {
    int vertex0 = -1;
    int vertex1 = -1;
    int face0 = -1;
    int face1 = -1;
};

struct FaceEdges {
    std::array<int, 3> edgeIndex;
    
    FaceEdges() : edgeIndex({-1, -1, -1}) {}

    FaceEdges(int i0_, int i1_, int i2_) : edgeIndex({i0_, i1_, i2_}) {}
    
    inline int operator[](int i) const;
    
    inline int& operator[](int i);

    inline int offsetOf(int i) const;
    
    inline int edgeIndexAfter(int currentEdgeIndex) const;
    
    inline int edgeIndexBefore(int currentEdgeIndex) const;
};

typedef std::set<int> VertexEdges;

class MeshTopology {
public:
    MeshTopology(const std::vector<Edge>& edges,
                 const std::vector<FaceEdges>& faceEdges,
                 const std::vector<VertexEdges>& vertexEdges);
    
    MeshTopology(const std::vector<Face3>& faces);
    
    MeshTopology() {}
    
    void compute(const std::vector<Face3>& faces);
    
    const std::vector<Edge> getEdges() const;
    const std::vector<FaceEdges> getFaceEdges() const;
    const std::vector<VertexEdges> getVertexEdges() const;
    
    int getNumEdges() const;
    int getNumFaceEdges() const;
    int getNumVertexEdges() const;

    MeshTopology(MeshTopology&&) = delete;
    MeshTopology& operator=(MeshTopology&&) = delete;
    MeshTopology(MeshTopology const& other) = delete;
    MeshTopology& operator=(MeshTopology const& other) = delete;
    
private:
    std::vector<Edge> _edges;
    std::vector<FaceEdges> _faceEdges;
    std::vector<VertexEdges> _vertexEdges;
};

/* Return true if two faces are identical */
inline bool operator==(const FaceEdges& lhs, const FaceEdges& rhs)
{
    return lhs[0] == rhs[0] &&
           lhs[1] == rhs[1] &&
           lhs[2] == rhs[2];
}

inline bool operator!=(const FaceEdges& lhs, const FaceEdges& rhs)
{
    return !(lhs == rhs);
}

/* Return true if two faces are identical */
inline bool operator==(const Edge& lhs, const Edge& rhs)
{
    return lhs.vertex0 == rhs.vertex0 &&
           lhs.vertex1 == rhs.vertex1 &&
           lhs.face0 == rhs.face0 &&
           lhs.face1 == rhs.face1;
}

inline bool operator!=(const Edge& lhs, const Edge& rhs)
{
    return !(lhs == rhs);
}

inline int FaceEdges::operator[](int i) const
{
    return edgeIndex[i];
}

inline int& FaceEdges::operator[](int i)
{
    return edgeIndex[i];
}

inline int FaceEdges::offsetOf(int i) const
{
    if (i == edgeIndex[0]) {
        return 0;
    } else if (i == edgeIndex[1]) {
        return 1;
    } else if (i == edgeIndex[2]) {
        return 2;
    }
    return -1;
}

inline int FaceEdges::edgeIndexAfter(int currentEdgeIndex) const
{
    int position = offsetOf(currentEdgeIndex);
    if (position == -1) return -1;
    return edgeIndex[(position + 1) % 3];
}

inline int FaceEdges::edgeIndexBefore(int currentEdgeIndex) const
{
    int position = offsetOf(currentEdgeIndex);
    if (position == -1) return -1;
    return edgeIndex[(position + 2) % 3];
}

} 

} // namespace sc3d
} // namespace standard_cyborg::MeshTopology
