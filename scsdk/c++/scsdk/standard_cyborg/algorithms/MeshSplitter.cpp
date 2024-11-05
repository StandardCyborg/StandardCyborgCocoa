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

#include <map>
#include <queue>

#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/algorithms/PrincipalAxes.hpp"

using standard_cyborg::sc3d::Geometry;
using standard_cyborg::sc3d::Face3;

namespace standard_cyborg {

namespace algorithms {

// See header for documentation
// Original implementation by Eric Arneback
std::vector<std::shared_ptr<Geometry>> splitMeshIntoPieces(const Geometry& geometry)
{
    using math::Vec3;
    
    if (geometry.vertexCount() == 0 || !geometry.hasFaces()) {
        return std::vector<std::shared_ptr<Geometry>>();
    }
    
    std::vector<std::set<int>> adjacencies;
    
    for (int ii = 0; ii < geometry.getPositions().size(); ++ii) {
        adjacencies.push_back(std::set<int>());
    }
    
    for (int it = 0; it < geometry.getFaces().size(); ++it) {
        standard_cyborg::sc3d::Face3 f = geometry.getFaces()[it];
        
        adjacencies[f[0]].insert(f[1]);
        adjacencies[f[1]].insert(f[0]);
        
        adjacencies[f[1]].insert(f[2]);
        adjacencies[f[2]].insert(f[1]);
        
        adjacencies[f[2]].insert(f[0]);
        adjacencies[f[0]].insert(f[2]);
    }
    
    std::vector<bool> traversed;
    
    for (int jj = 0; jj < geometry.getPositions().size(); ++jj) {
        traversed.push_back(false);
    }
    int countTraversed = 0;
    
    std::vector<std::vector<int>> splittedIndices;
    
    while (countTraversed != geometry.getPositions().size()) {
        std::queue<int> queue;
        
        // find one seed vertex that hasnt been traversed yet.
        for (int ii = 0; ii < geometry.getPositions().size(); ++ii) {
            if (traversed[ii] == false) {
                queue.push(ii);
                break;
            }
        }
        
        std::vector<int> island;
        
        while (queue.size() > 0) {
            int elem = queue.front(); queue.pop();
            
            if (traversed[elem] == true) {
                continue;
            }
            
            island.push_back(elem);
            traversed[elem] = true;
            countTraversed++;
            
            for (int neighbour : adjacencies[elem]) {
                if (traversed[neighbour] == false) {
                    queue.push(neighbour);
                }
            }
        }
        
        // dont include single points
        if (island.size() > 1) {
            splittedIndices.push_back(island);
        }
    }
    
    std::vector<int> indexToSubmeshMapping;
    
    for (int ii = 0; ii < geometry.vertexCount(); ++ii) {
        indexToSubmeshMapping.push_back(-1);
    }
    
    int iSubmesh = 0;
    for (std::vector<int> submesh : splittedIndices) {
        for (int index : submesh) {
            indexToSubmeshMapping[index] = iSubmesh;
        }
        
        ++iSubmesh;
    }
    
    std::vector<std::vector<Vec3>> subPositions;
    std::vector<std::vector<Vec3>> subNormals;
    std::vector<std::vector<Vec3>> subColors;
    std::vector<std::vector<Face3>> subFaces;
    std::vector<int> freeIndex;
    
    for (int ii = 0; ii < splittedIndices.size(); ++ii) {
        // std::shared_ptr<Geometry> geo(new Geometry());
        subPositions.push_back(std::vector<Vec3>());
        subNormals.push_back(std::vector<Vec3>());
        subColors.push_back(std::vector<Vec3>());
        subFaces.push_back(std::vector<Face3>());
        
        freeIndex.push_back(0);
    }
    
    std::map<int, int> indexMapping;

    for (Face3 face : geometry.getFaces()) {
        int iSubmesh = indexToSubmeshMapping[face[0]];
        
        Face3 newFace;
        
        for (int iCorner = 0; iCorner < 3; ++iCorner) {
        
            // if an index is unencoutered, then add to list of positions in mesh.
            if (indexMapping.count(face[iCorner]) != 0) {
                newFace[iCorner] = indexMapping[face[iCorner]];
            } else {
                int newIndex = freeIndex[iSubmesh]++;
                
                if (geometry.hasPositions()) {
                    Vec3 position = geometry.getPositions()[face[iCorner]];
                    subPositions[iSubmesh].push_back(position);
                }
                if (geometry.hasNormals()) {
                    Vec3 normal = geometry.getNormals()[face[iCorner]];
                    subNormals[iSubmesh].push_back(normal);
                }
                
                if (geometry.hasColors()) {
                    Vec3 color = geometry.getColors()[face[iCorner]];
                    subColors[iSubmesh].push_back(color);
                }
                
                newFace[iCorner] = newIndex;
                
                indexMapping[face[iCorner]] = newIndex;
            }
        }
        
        subFaces[iSubmesh].push_back(newFace);
    }
    
    std::vector<std::shared_ptr<Geometry>> splittedMeshes;
    
    for (int iSubmesh = 0; iSubmesh < splittedIndices.size(); ++iSubmesh) {
        std::shared_ptr<Geometry> geo(new Geometry(subPositions[iSubmesh],
                                                   subNormals[iSubmesh],
                                                   subColors[iSubmesh],
                                                   subFaces[iSubmesh]));
        
        splittedMeshes.push_back(geo);
    }
    
    return splittedMeshes;
}

}

} // namespace StandardCyborg
