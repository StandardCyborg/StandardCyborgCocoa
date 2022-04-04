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

#include <google/protobuf/util/time_util.h>

#include "standard_cyborg/util/Result.hpp"

namespace standard_cyborg {
namespace io {
namespace pbio {

// Helps convert a `NSTimeInterval` instance to a protobuf `Timestamp` without
// having to include the Protobuf ObjectiveC library.  Based upon that code:
// https://github.com/protocolbuffers/protobuf/blob/d9ccd0c0e6bbda9bf4476088eeb46b02d7dcd327/objectivec/GPBWellKnownTypes.m#L49
extern inline ::google::protobuf::Timestamp NSTimeIntervalToPB(
                                                               double NSTimeInterval_timestamp) {
    
    double seconds;
    double nanos = std::modf(NSTimeInterval_timestamp, &seconds);
    
    // Nanos must be non-negative
    if (nanos < 0) {
        seconds -= 1;
        nanos = 1. + nanos;
        // use this for fp stability
    }
    
    ::google::protobuf::Timestamp msg;
    msg.set_seconds(seconds);
    msg.set_nanos(nanos);
    
    return msg;
}


// Helps convert a `CMTime` instance to a protobuf `Timestamp` without
// having to include the Protobuf ObjectiveC library.  Based upon that code:
// https://github.com/protocolbuffers/protobuf/blob/d9ccd0c0e6bbda9bf4476088eeb46b02d7dcd327/objectivec/GPBWellKnownTypes.m#L49
extern inline ::google::protobuf::Timestamp CMTimeToPB(
                                                       double CMTime_seconds) {
    
    double seconds;
    double nanos = std::modf(CMTime_seconds, &seconds);
    
    // Nanos must be non-negative
    if (nanos < 0) {
        seconds -= 1;
        nanos = 1. + nanos;
        // use this for fp stability
    }
    
    ::google::protobuf::Timestamp msg;
    msg.set_seconds(seconds);
    msg.set_nanos(nanos);
    
    return msg;
}


} // namespace pbio
} // namespace io
} // namespace standard_cyborg
