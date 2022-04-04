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

#include <cmath>
#include <limits>
#include <string>

#include "standard_cyborg/math/Quaternion.hpp"
#include "standard_cyborg/math/Vec3.hpp"

namespace standard_cyborg {
namespace math {

struct Mat3x4;

struct Transform {
    math::Quaternion rotation;
    math::Vec3 translation;
    math::Vec3 scale = math::Vec3{1.0f, 1.0f, 1.0f};
    math::Vec3 shear;
    
    std::string srcFrame;
    std::string destFrame;

    static Transform fromMat3x4(const math::Mat3x4& matrix);
    static Transform fromMat3x4(const math::Mat3x4& matrix, std::string srcFrame, std::string destFrame);

    /** Return an inverted copy of the transform */
    Transform inverse() const;
};

inline bool operator==(const Transform& lhs, const Transform& rhs) {
    return lhs.rotation == rhs.rotation &&
    lhs.translation == rhs.translation &&
    lhs.scale == rhs.scale &&
    lhs.shear == rhs.shear;
}

inline bool operator!=(const Transform& lhs, const Transform& rhs) {
    return !(lhs == rhs);
}

} // namespace math
} // namespace standard_cyborg
