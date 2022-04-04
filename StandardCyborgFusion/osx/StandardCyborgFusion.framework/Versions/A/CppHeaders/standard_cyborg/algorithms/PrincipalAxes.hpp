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

#include <vector>

#pragma once

namespace standard_cyborg {

namespace sc3d{
class Geometry;
}

namespace math {
struct Mat3x4;
struct Vec3;
}

namespace algorithms {

standard_cyborg::math::Mat3x4 computeNormalwisePrincipalAxes(const sc3d::Geometry& geometry);

standard_cyborg::math::Mat3x4 computePointwisePrincipalAxes(const sc3d::Geometry& geometry);

standard_cyborg::math::Mat3x4 computePointwisePrincipalAxes(const std::vector<standard_cyborg::math::Vec3>& positions);

}

} // namespace StandardCyborg
