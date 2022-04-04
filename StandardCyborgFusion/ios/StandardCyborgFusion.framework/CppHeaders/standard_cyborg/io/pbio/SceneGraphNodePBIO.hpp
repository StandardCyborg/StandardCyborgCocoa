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

#include "standard_cyborg/proto/scene_graph/scene_graph_node.pb.h"
#include "standard_cyborg/util/Result.hpp"

#include "standard_cyborg/scene_graph/SceneGraph.hpp"

namespace standard_cyborg {

namespace sc3d {
}

namespace io {
namespace pbio {

extern Result<std::shared_ptr<scene_graph::Node>> FromPB(const standard_cyborg::proto::scene_graph::SceneGraphNode &msg);

extern Result<standard_cyborg::proto::scene_graph::SceneGraphNode> ToPB(const std::shared_ptr<scene_graph::Node>& sceneGraph);

} // namespace pbio
} // namespace io
} // namespace standard_cyborg
