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

#include "standard_cyborg/math/Quaternion.hpp"
#include "standard_cyborg/math/Vec2.hpp"
#include "standard_cyborg/math/Vec3.hpp"
#include "standard_cyborg/proto/math/core.pb.h"

#include "standard_cyborg/util/Result.hpp"

namespace standard_cyborg {
namespace io {
namespace pbio {

// Vec2 =======================================================================

extern inline Result<math::Vec2> FromPB(
                                        const ::standard_cyborg::proto::math::Vec2 &v) {
    return {.value = math::Vec2(v.x(), v.y())};
}

extern inline Result<::standard_cyborg::proto::math::Vec2> ToPB(
                                                                const math::Vec2 &v) {
    ::standard_cyborg::proto::math::Vec2 msg;
    msg.set_x(v.x);
    msg.set_y(v.y);
    return {.value = msg};
}


// Vec3 =======================================================================

extern inline Result<math::Vec3> FromPB(const ::standard_cyborg::proto::math::Vec3 &v) {
    return {.value = math::Vec3(v.x(), v.y(), v.z())};
}

extern inline Result<::standard_cyborg::proto::math::Vec3> ToPB(const math::Vec3 &v) {
    ::standard_cyborg::proto::math::Vec3 msg;
    msg.set_x(v.x);
    msg.set_y(v.y);
    msg.set_z(v.z);
    return {.value = msg};
}


// Quaternion =================================================================

extern inline Result<math::Quaternion> FromPB(const ::standard_cyborg::proto::math::Quaternion &v) {
    return {.value = math::Quaternion(v.x(), v.y(), v.z(), v.w())};
}

extern inline Result<::standard_cyborg::proto::math::Quaternion> ToPB(const math::Quaternion &v) {
    ::standard_cyborg::proto::math::Quaternion msg;
    msg.set_x(v.x);
    msg.set_y(v.y);
    msg.set_z(v.z);
    msg.set_w(v.w);
    return {.value = msg};
}


} // namespace pbio
} // namespace io
} // namespace standard_cyborg
