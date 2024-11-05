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

#include "standard_cyborg/algorithms/EdgeLoopFinder.hpp"

#include "standard_cyborg/sc3d/Geometry.hpp"

#include <unordered_map>

struct Edge {
    int i0, i1;
    
    Edge(int i0_, int i1_)
    {
        i0 = i0_;
        i1 = i1_;
    }
};

struct EdgeEqualUnordered {
    bool operator()(const Edge& lhs, const Edge& rhs) const
    {
        return (lhs.i0 == rhs.i0 && lhs.i1 == rhs.i1) || (lhs.i0 == rhs.i1 && lhs.i1 == rhs.i0);
    }
};

struct EdgeEqualOrdered {
    bool operator()(const Edge& lhs, const Edge& rhs) const
    {
        return (lhs.i0 == rhs.i0 && lhs.i1 == rhs.i1);
    }
};

struct EdgeHash {
    std::size_t operator()(const Edge& k) const
    {
        // clang-format off
        if (k.i0 < k.i1) {
            return ((std::hash<int>()(k.i0)
                  ^ (std::hash<int>()(k.i1) << 1)) >> 1);
        }
        else {
            return ((std::hash<int>()(k.i1)
                  ^ (std::hash<int>()(k.i0) << 1)) >> 1);
        }
        // clang-format on
    }
};

namespace standard_cyborg {


using standard_cyborg::sc3d::Face3;
using standard_cyborg::sc3d::Geometry;

namespace algorithms {


std::vector<std::vector<std::pair<int, int>>> findEdgeLoops(const standard_cyborg::sc3d::Geometry& geometry)
{
    // counts how many times each edge can be found in the geometry.
    std::unordered_map<Edge, int, EdgeHash, EdgeEqualUnordered> edgeCounts;
    
    std::vector<std::set<int>> adjacencies;
    for (int ii = 0; ii < geometry.vertexCount(); ++ii) {
        adjacencies.push_back(std::set<int>());
    }
    
    for (int iface = 0; iface < geometry.getFaces().size(); ++iface) {
        
        Face3 f = geometry.getFaces()[iface];
        
        int* faceIndices = (int*)((void*)&f);
        
        for (int iedge = 0; iedge < 3; ++iedge) {
            Edge edge(faceIndices[(iedge + 0) % 3], faceIndices[(iedge + 1) % 3]);
            
            edgeCounts[edge]++;
            
            adjacencies[edge.i0].insert(edge.i1);
            adjacencies[edge.i1].insert(edge.i0);
        }
    }
    
    std::unordered_map<Edge, bool, EdgeHash, EdgeEqualOrdered> remainingEdges;
    
    for (const auto& it : edgeCounts) {
        if (it.second == 1) {
            remainingEdges.insert({it.first, true});
        }
    }
    
    std::vector<std::vector<Edge>> loops;
    
    while (remainingEdges.size() > 0) {
        std::vector<Edge> loop;
        
        const Edge seedEdge = remainingEdges.begin()->first;
        remainingEdges.erase(remainingEdges.begin());
        loop.push_back(seedEdge);
        
        Edge latestEdge = seedEdge;
        while (seedEdge.i0 != latestEdge.i1) {
            
            std::set<int> adjs = adjacencies[latestEdge.i1];
            
            for (int adj : adjs) {
                Edge attemptEdge(latestEdge.i1, adj);
                
                if (remainingEdges.count(attemptEdge) == 1) {
                    auto iter = remainingEdges.find(attemptEdge);
                    
                    remainingEdges.erase(iter);
                    loop.push_back(attemptEdge);
                    latestEdge = attemptEdge;
                    
                    break;
                }
            }
        }
        
        loops.push_back(loop);
    }
    
    std::vector<std::vector<std::pair<int, int>>> result;
    
    for (const std::vector<Edge>& loop : loops) {
        std::vector<std::pair<int, int>> convertedLoop;
        
        for (const Edge& edge : loop) {
            convertedLoop.push_back(std::pair<int, int>(edge.i0, edge.i1));
        }
        
        result.push_back(convertedLoop);
    }
    
    return result;
}

}

} // namespace StandardCyborg
