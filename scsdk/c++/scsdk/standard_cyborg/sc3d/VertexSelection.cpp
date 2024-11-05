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

#include <algorithm>

#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/sc3d/Polyline.hpp"
#include "standard_cyborg/sc3d/VertexSelection.hpp"

namespace standard_cyborg {
namespace sc3d {

VertexSelection::VertexSelection()
{}

VertexSelection::VertexSelection(const Geometry& geometry, std::vector<int> initialSelectedVertices) :
    _vertexIndices(std::set<int>(initialSelectedVertices.begin(), initialSelectedVertices.end())),
    _totalVertexCount(geometry.vertexCount())
{}

VertexSelection::VertexSelection(const Polyline& polyline, std::vector<int> initialSelectedVertices) :
    _vertexIndices(std::set<int>(initialSelectedVertices.begin(), initialSelectedVertices.end())),
    _totalVertexCount(polyline.vertexCount())
{}

VertexSelection::VertexSelection(int totalVertexCount, std::vector<int> initialSelectedVertices) :
    _vertexIndices(std::set<int>(initialSelectedVertices.begin(), initialSelectedVertices.end())),
    _totalVertexCount(totalVertexCount)
{}

VertexSelection::const_iterator VertexSelection::begin() const
{
    return _vertexIndices.begin();
}

VertexSelection::const_iterator VertexSelection::end() const
{
    return _vertexIndices.end();
}


std::vector<int> VertexSelection::toVector() const
{
    std::vector<int> indices;
    for (auto it = begin(); it != end(); ++it) {
        indices.push_back(*it);
    }
    
    return indices;
}

int VertexSelection::size() const
{
    return (int)_vertexIndices.size();
}

int VertexSelection::getTotalVertexCount() const
{
    return _totalVertexCount;
}

void VertexSelection::copy(const VertexSelection& that)
{
    _vertexIndices.clear();
    _vertexIndices.insert(that.begin(), that.end());
    _totalVertexCount = that._totalVertexCount;
}

void VertexSelection::insertValue(const int index)
{
    _vertexIndices.insert(index);
}

void VertexSelection::removeValue(const int index)
{
    _vertexIndices.erase(index);
}

bool VertexSelection::contains(int index) const
{
    return _vertexIndices.find(index) != _vertexIndices.end();
}

void VertexSelection::unionWith(const VertexSelection& other)
{
    _vertexIndices.insert(other.begin(), other.end());
}

void VertexSelection::differenceWith(const VertexSelection& other)
{
    auto end = other.end();
    for (auto iter = other.begin(); iter != end; iter++) {
        _vertexIndices.erase(*iter);
    }
}

void VertexSelection::intersectWith(const VertexSelection& other)
{
    auto first = begin();
    auto last = end();
    auto otherFirst = other.begin();
    auto otherLast = other.end();
    
    // This is a slight modification of the algorithm from
    // https://en.cppreference.com/w/cpp/algorithm/set_intersection
    // It has been modified to mutate in place rather than writing
    // values into a new selection
    while (first != last && otherFirst != otherLast) {
        if (*otherFirst < *first) {
            // First2 catches up to first
            ++otherFirst;
        } else {
            if (!(*first < *otherFirst)) {
                // If equal, increment both pointers
                ++otherFirst;
                ++first;
            } else {
                // Otherwise otherFirst is already caught up so we delete first
                first = _vertexIndices.erase(first);
            }
        }
    }
    
    // Cleanup. If we didn't get to the end of vertex indices, then they're
    // not in the intersection set and we erase to the end.
    _vertexIndices.erase(first, last);
}

bool VertexSelection::operator==(const VertexSelection& other) const
{
    if (size() != other.size()) return false;
    return std::equal(begin(), end(), other.begin());
}

bool VertexSelection::operator!=(const VertexSelection& other) const
{
    return !(*this == other);
}

void VertexSelection::clear()
{
    _vertexIndices.clear();
}

std::unique_ptr<VertexSelection> VertexSelection::fromGeometryVertices(
    const Geometry& geometry,
    const std::function<bool(int index, math::Vec3 position, math::Vec3 normal, math::Vec3 color)>& filterFn)
{
    std::unique_ptr<VertexSelection> selection = std::make_unique<VertexSelection>(geometry);
    
    const std::vector<math::Vec3>& positions = geometry.getPositions();
    const std::vector<math::Vec3>& normals = geometry.getNormals();
    const std::vector<math::Vec3>& colors = geometry.getColors();
    
    int vertexCount = geometry.vertexCount();
    for (int index = 0; index < vertexCount; index++) {
        if (filterFn(index, positions[index], normals[index], colors[index])) {
            selection->insertValue(index);
        }
    }
    
    return selection;
}

void VertexSelection::invert()
{
    // Note: we proceed by constructing a new inverse selection and then
    // overwriting the original. This could be done in-place, but that
    // operation is about 50% trickier, so we won't, for now.
    std::set<int> newSelection;
    
    // Iterate through the indices and add any gaps to the new selection
    int previous = -1;
    for (auto index : _vertexIndices) {
        for (int i = previous + 1; i < index; i++) {
            if (i < _totalVertexCount) newSelection.insert(i);
        }
        previous = index;
    }
    
    // Iterate from the last index up to the end
    for (int i = previous + 1; i < _totalVertexCount; i++) {
        newSelection.insert(i);
    }
    
    // Transfer the new selection into the existing
    _vertexIndices = newSelection;
}

} // namespace sc3d
} // namespace standard_cyborg
