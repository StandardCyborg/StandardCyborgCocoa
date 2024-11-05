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


#include "standard_cyborg/sc3d/Polyline.hpp"

namespace standard_cyborg {
namespace sc3d {

using math::Vec3;

Polyline::Polyline() : _positions()
{}

Polyline::Polyline(const std::vector<Vec3>& positions)
    : _positions(positions)
{}

Polyline::~Polyline() {}

int Polyline::vertexCount() const { return static_cast<int>(_positions.size()); }

const std::vector<Vec3>& Polyline::getPositions() const { return _positions; }

void Polyline::setPositions(const std::vector<Vec3>& positions)
{
    _positions = positions;
}

void Polyline::copy(const Polyline& that)
{
    _positions = that.getPositions();
}

bool Polyline::isClosed(float relativeTolerance, float absoluteTolerance) const
{
    if (_positions.size() == 0) { return false; }
    
    return Vec3::almostEqual(_positions.front(), _positions.back(), relativeTolerance, absoluteTolerance);
}

float Polyline::length() const
{
    float sum = 0.0f;
    
    if (_positions.size() == 0) { return sum; }
    
    Vec3 positionA = _positions[0];
    Vec3 positionB;
    
    for (int i = 1; i < _positions.size(); i++) {
        positionB = _positions[i];
        sum += (positionB - positionA).norm();
        positionA = positionB;
    }
    
    return sum;
}

bool operator==(const Polyline& lhs, const Polyline& rhs) {
    return lhs.getPositions() == rhs.getPositions();
}


} // namespace sc3d
} // namespace standard_cyborg
