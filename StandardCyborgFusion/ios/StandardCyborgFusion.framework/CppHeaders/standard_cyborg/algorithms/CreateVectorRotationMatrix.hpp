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

namespace standard_cyborg {

namespace math {

struct Vec3;
struct Mat3x3;
}

namespace algorithms {


/*
 Creates a rotation matrix, that rotates the unit vector v0 into the unit vector v1.
 The returned matrix will fulfill the below property
 
 assert(createVectorRotationMatrix(v0, v1) * v0 == v1)
 */

math::Mat3x3 createVectorRotationMatrix(const math::Vec3& v0, const math::Vec3& v1);

} // algorithms

} // namespace standard_cyborg
