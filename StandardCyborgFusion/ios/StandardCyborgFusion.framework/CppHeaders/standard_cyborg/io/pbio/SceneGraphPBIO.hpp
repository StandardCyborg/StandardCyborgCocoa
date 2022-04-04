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

#include <standard_cyborg/math/TransformRegistry.hpp>

#include "standard_cyborg/proto/scene_graph/scene_graph_node.pb.h"
#include "standard_cyborg/util/Result.hpp"

#include "standard_cyborg/scene_graph/SceneGraph.hpp"
#include "standard_cyborg/proto/scene_graph/scene_graph.pb.h"

namespace standard_cyborg {

namespace sc3d {
}

namespace io {
namespace pbio {


class SceneGraph {
public:
    std::shared_ptr<standard_cyborg::scene_graph::Node> sceneGraph;
    math::TransformRegistry transformRegistry;
};

extern Result<SceneGraph> FromPB(const standard_cyborg::proto::scene_graph::SceneGraph &msg);

extern Result<standard_cyborg::proto::scene_graph::SceneGraph>  ToPB(const SceneGraph& sceneGraph);


extern Result<bool> WriteSceneGraphToPB(std::shared_ptr<standard_cyborg::scene_graph::Node> sceneGraph, const std::string& path);

extern Result<std::shared_ptr<standard_cyborg::scene_graph::Node> > ReadSceneGraphFromPBStream(std::istream& inStream);

} // namespace pbio
} // namespace io
} // namespace standard_cyborg
