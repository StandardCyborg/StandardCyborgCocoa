//
//  VertexSelection.hpp
//  StandardCyborgGeometry
//
//  Created by Ricky Reusser on 4/9/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <functional>
#include <initializer_list>
#include <set>

#include <StandardCyborgData/Vec3.hpp>

namespace StandardCyborg {

class Geometry;

class VertexSelection {
public:
    /* Empty selection constructor */
    VertexSelection();
    
    /* Makes `VertexSelection selection {1, 2, 3}` work */
    VertexSelection(std::initializer_list<int> initialSelectedVertices);
    
    VertexSelection(int vertexCount);
    
    /* Copy constructor disabled */
    //VertexSelection(const VertexSelection&) = delete;
    
    /* Works if type implements begin and end */
    template <class T>
    VertexSelection(const T& initialSelectedVertices);
    
    /* Make this selection a copy of that */
    void copy(const VertexSelection& that);
    
    /* Number of vertices in selection */
    int size() const;
    
    /* Insert and erase a single index */
    void insertValue(const int index);
    void removeValue(const int index);
    
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
    
    
    void clear();
    
    /* Compare against other vertetx selections*/
    bool operator==(const VertexSelection& other) const;
    bool operator!=(const VertexSelection& other) const;
    
    static std::unique_ptr<VertexSelection> fromGeometryVertices(
        const Geometry& geometry,
        const std::function<bool(int index, Vec3 position, Vec3 normal, Vec3 color)>& filterFn);
    
    void invert(const Geometry& geometry);
    void invert(int totalVertexCount);
    
private:
    std::set<int> _vertexIndices;
};

//static_assert(!std::is_copy_constructible<VertexSelection>::value, "Copy constructor is disabled");

template <class T>
VertexSelection::VertexSelection(const T& initialSelectedVertices)
{
    _vertexIndices.insert(initialSelectedVertices.begin(), initialSelectedVertices.end());
}

} // namespace StandardCyborg
