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

namespace standard_cyborg {
namespace sc3d {

struct Face3 {
    std::array<int, 3> vertexIndex;
    
    Face3() : vertexIndex({-1, -1, -1}) {}
    
    Face3(int i0_, int i1_, int i2_) : vertexIndex({i0_, i1_, i2_}) {}
    
    inline int operator[](int i) const;
    
    inline int& operator[](int i);
    
    inline int getIndex(int i) const
    {
        if (i < 0 || i > 2) {
            return -1;
        }
        return vertexIndex[i];
    }
    
    inline int offsetOf(int i) const;
    
    inline int nextVertexIndex(int currentVertexIndex) const;
    
    inline int previousVertexIndex(int currentVertexIndex) const;
};

inline bool operator==(const Face3& lhs, const Face3& rhs)
{
    return lhs[0] == rhs[0] &&
           lhs[1] == rhs[1] &&
           lhs[2] == rhs[2];
}

inline bool operator!=(const Face3& lhs, const Face3& rhs)
{
    return !(lhs == rhs);
}

int Face3::operator[](int i) const
{
    return vertexIndex[i];
}

int& Face3::operator[](int i)
{
    return vertexIndex[i];
}

int Face3::offsetOf(int i) const
{
    if (i == vertexIndex[0]) {
        return 0;
    } else if (i == vertexIndex[1]) {
        return 1;
    } else if (i == vertexIndex[2]) {
        return 2;
    }
    return -1;
}

int Face3::nextVertexIndex(int currentVertexIndex) const
{
    int position = offsetOf(currentVertexIndex);
    if (position == -1) return -1;
    return vertexIndex[(position + 1) % 3];
}

int Face3::previousVertexIndex(int currentVertexIndex) const
{
    int position = offsetOf(currentVertexIndex);
    if (position == -1) return -1;
    return vertexIndex[(position + 2) % 3];
}

} // namespace sc3d
} // namespace standard_cyborg
