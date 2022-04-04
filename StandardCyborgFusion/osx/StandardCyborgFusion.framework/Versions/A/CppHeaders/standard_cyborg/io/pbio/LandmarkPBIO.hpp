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

#include "standard_cyborg/sc3d/Landmark.hpp"
#include "standard_cyborg/proto/sc3d/landmark.pb.h"

#include "standard_cyborg/io/pbio/CorePBIO.hpp"

namespace standard_cyborg {
namespace io {
namespace pbio {

extern inline Result<sc3d::Landmark> FromPB(const ::standard_cyborg::proto::sc3d::Landmark &msg) {
    
    auto maybe_position = FromPB(msg.position());
    if (!maybe_position.IsOk()) { return {.error = maybe_position.error}; }
    
    
    
    return {.value = sc3d::Landmark{
        .position=*maybe_position.value,
        .name=msg.name(),
        .frame=msg.frame(),
        
    }
    };
}

extern inline Result<::standard_cyborg::proto::sc3d::Landmark> ToPB(const sc3d::Landmark landmark) {
    ::standard_cyborg::proto::sc3d::Landmark msg;
    
    
    auto maybe_position = ToPB(landmark.position);
    if (!maybe_position.IsOk()) { return {.error = maybe_position.error}; }
    *msg.mutable_position() = *maybe_position.value;
    
    msg.set_name(landmark.getName());
    
    msg.set_frame(landmark.getFrame());
    
    return {.value = msg};
}

} // namespace pbio
} // namespace io
} // namespace standard_cyborg
