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

#include <string>

#include "standard_cyborg/util/Result.hpp"
#include "standard_cyborg/scene_graph/SceneGraph.hpp"

namespace standard_cyborg {
namespace io {
namespace pbio {


// Read the entire protobag at `path` into a single SceneGraph and
// return the root node
Result<std::shared_ptr<scene_graph::Node>>
ReadSceneGraphFromProtobag(const std::string &path);


// Read a protobag as a series of Scene Graphs: one graph per bundle
// of sensor data at a single time step.
struct IterSceneGraphsFromProtobag {
    
    // A SceneGraph root node (in a sequence of nodes) or 
    // an end of sequence sentinel
    struct MaybeSG : public Result<std::shared_ptr<scene_graph::Node>> {
        static MaybeSG EndOfSequence() { return Err("EndOfSequence"); }
        bool IsEndOfSequence() const { return error == "EndOfSequence"; }
        
        static MaybeSG Err(const std::string &s) {
            MaybeSG m; m.error = s; return m;
        }
        
        static MaybeSG Ok(std::shared_ptr<scene_graph::Node> &&v) {
            MaybeSG m; m.value = std::move(v); return m;
        }
    };
    
    
    explicit IterSceneGraphsFromProtobag(const std::string &_path);
    
    MaybeSG GetNext();
    
    struct Impl;
    std::shared_ptr<Impl> impl;
    std::string path;
};

} // namespace pbio
} // namespace io
} // namespace standard_cyborg

