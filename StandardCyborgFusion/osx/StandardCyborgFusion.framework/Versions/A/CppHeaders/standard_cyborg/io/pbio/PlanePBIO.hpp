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

#include "standard_cyborg/sc3d/Plane.hpp"
#include "standard_cyborg/proto/sc3d/plane.pb.h"

#include "standard_cyborg/io/pbio/CorePBIO.hpp"

namespace standard_cyborg {
namespace io {
namespace pbio {

extern inline Result<sc3d::Plane> FromPB(const ::standard_cyborg::proto::sc3d::Plane &msg) {
    
    auto maybe_position = FromPB(msg.position());
    if (!maybe_position.IsOk()) { return {.error = maybe_position.error}; }
    
    auto maybe_normal = FromPB(msg.normal());
    if (!maybe_normal.IsOk()) { return {.error = maybe_normal.error}; }
    
    return {.value = sc3d::Plane{
        .position=*maybe_position.value,
        .normal=*maybe_normal.value,
        .frame=msg.frame()
    }
    };
}

extern inline Result<::standard_cyborg::proto::sc3d::Plane> ToPB(const sc3d::Plane pt) {
    ::standard_cyborg::proto::sc3d::Plane msg;
    
    auto maybe_position = ToPB(pt.position);
    if (!maybe_position.IsOk()) { return {.error = maybe_position.error}; }
    *msg.mutable_position() = *maybe_position.value;
    
    auto maybe_normal = ToPB(pt.normal);
    if (!maybe_normal.IsOk()) { return {.error = maybe_normal.error}; }
    *msg.mutable_normal() = *maybe_normal.value;
    
    msg.set_frame(pt.getFrame());
    
    return {.value = msg};
}

} // namespace pbio
} // namespace io
} // namespace standard_cyborg
