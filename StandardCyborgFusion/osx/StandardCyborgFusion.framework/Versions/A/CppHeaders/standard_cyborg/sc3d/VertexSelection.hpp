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

#include <functional>
#include <initializer_list>
#include <set>
#include <vector>
#include <memory>

#include "standard_cyborg/math/Vec3.hpp"

namespace standard_cyborg {
namespace sc3d {

class Geometry;
class Polyline;

class VertexSelection {
public:
    /* Empty selection constructor */
    VertexSelection();
    VertexSelection(const Geometry& geometry, std::vector<int> initialSelectedVertices = std::vector<int>());
    VertexSelection(const Polyline& polyline, std::vector<int> initialSelectedVertices = std::vector<int>());
    VertexSelection(int totalVertexCount, std::vector<int> initialSelectedVertices = std::vector<int>());
    
    /* Make this selection a copy of that */
    void copy(const VertexSelection& that);
    
    /* Number of vertices in selection */
    int size() const;
    
    int getTotalVertexCount() const;
    
    /* Insert and erase a single index */
    void insertValue(int index);
    void removeValue(int index);
    bool contains(int index) const;
    
    /* Remove all values */
    void clear();
    
    void invert();
    
    /* Boolean operations with a second selection */
    void unionWith(const VertexSelection& other);
    void differenceWith(const VertexSelection& other);
    void intersectWith(const VertexSelection& other);
    
    /* Const iterators.
     * Note: non-const iterators are not implemented since they're fairly strongly
     * discouraged for sets, which will break if you start modifying values and
     * messing with the guarantee that entries are sorted.
     */
    typedef std::set<int>::const_iterator const_iterator;
    const_iterator begin() const;
    const_iterator end() const;
    
    std::vector<int> toVector() const;
    
    void print() const;
    
    
    /* Compare against other vertex selections */
    bool operator==(const VertexSelection& other) const;
    bool operator!=(const VertexSelection& other) const;
    
    /* Selects all vertices from the given geometry where filterFn returns true */
    static std::unique_ptr<VertexSelection> fromGeometryVertices(
        const Geometry& geometry,
        const std::function<bool(int index, math::Vec3 position, math::Vec3 normal, math::Vec3 color)>& filterFn);
    
private:
    std::set<int> _vertexIndices;
    
    // This class maintains a subset of the vertices of the geometry or polyline it was instantiated with
    // Maintaining the original count allows us to invert the selection and make correctness assertions
    int _totalVertexCount = 0;
};

//static_assert(!std::is_copy_constructible<VertexSelection>::value, "Copy constructor is disabled");

} // namespace sc3d
} // namespace standard_cyborg
