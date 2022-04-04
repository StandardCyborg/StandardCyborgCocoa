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

#include "standard_cyborg/math/Vec3.hpp"
#include <string>

namespace standard_cyborg {
namespace sc3d {

// A plane *defined* by
//
//   dot(x - position, normal) == 0
//
// This definition is fairly redundant since only three numbers are required
// to represent a plane, but what we lose in redundancy we gain in simplicity.
struct Plane {
    std::string getFrame() const { return frame; }
    void setFrame(const std::string &f) { frame = f; }
    
    math::Vec3 position;
    math::Vec3 normal;
    
    std::string frame;

};

inline bool operator==(const Plane& lhs, const Plane& rhs) {
    return lhs.position == rhs.position && lhs.normal == rhs.normal;
}

inline bool operator!=(const Plane& lhs, const Plane& rhs) {
    return  !(lhs == rhs);
}


} // namespace sc3d
} // namespace standard_cyborg
