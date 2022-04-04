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

#include "standard_cyborg/math/Transform.hpp"
#include "standard_cyborg/proto/math/transform.pb.h"

#include "standard_cyborg/io/pbio/CorePBIO.hpp"

namespace standard_cyborg {
namespace io {
namespace pbio {

extern inline Result<math::Transform> FromPB(const ::standard_cyborg::proto::math::Transform &msg) {
    
    auto maybe_rotation = FromPB(msg.rotation());
    if (!maybe_rotation.IsOk()) { return {.error = maybe_rotation.error}; }
    
    auto maybe_translation = FromPB(msg.translation());
    if (!maybe_translation.IsOk()) { return {.error = maybe_translation.error}; }
    
    auto maybe_scale = FromPB(msg.scale());
    if (!maybe_scale.IsOk()) { return {.error = maybe_scale.error}; }
    
    auto maybe_shear = FromPB(msg.shear());
    if (!maybe_shear.IsOk()) { return {.error = maybe_shear.error}; }
    
    return {.value = math::Transform{
        .rotation=*maybe_rotation.value,
        .translation=*maybe_translation.value,
        
        .shear=*maybe_shear.value,
        .scale=*maybe_scale.value,
        
        .srcFrame=msg.src_frame(),
        .destFrame=msg.dest_frame(),
    }
    };
}

extern inline Result<::standard_cyborg::proto::math::Transform> ToPB(const math::Transform &t) {
    ::standard_cyborg::proto::math::Transform msg;
    
    auto maybe_rotation = ToPB(t.rotation);
    if (!maybe_rotation.IsOk()) { return {.error = maybe_rotation.error}; }
    
    auto maybe_translation = ToPB(t.translation);
    if (!maybe_translation.IsOk()) { return {.error = maybe_translation.error}; }
    
    auto maybe_scale = ToPB(t.scale);
    if (!maybe_scale.IsOk()) { return {.error = maybe_scale.error}; }
    
    auto maybe_shear = ToPB(t.shear);
    if (!maybe_shear.IsOk()) { return {.error = maybe_shear.error}; }
    
    *msg.mutable_rotation() = *maybe_rotation.value;
    *msg.mutable_translation() = *maybe_translation.value;
    
    *msg.mutable_shear() = *maybe_shear.value;
    *msg.mutable_scale() = *maybe_scale.value;
    
    msg.set_src_frame(t.srcFrame);
    msg.set_dest_frame(t.destFrame);
    return {.value = msg};
}

} // namespace pbio
} // namespace io
} // namespace standard_cyborg
