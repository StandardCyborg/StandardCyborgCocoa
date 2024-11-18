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

#include <memory>
#include <string>
#include <vector>

namespace standard_cyborg {

namespace scene_graph {
class Node;
}

namespace io {
namespace gltf {

namespace sg = standard_cyborg::scene_graph;

/** Read from a string formatted as gltf-file. */
std::vector<std::shared_ptr<sg::Node>> ReadSceneGraphFromGltf(const std::string& gltfSource);

/** Write a Scene graph to a path. */
bool WriteSceneGraphToGltf(std::vector<std::shared_ptr<sg::Node>> sceneGraph, const std::string& path);
bool WriteSceneGraphToGltf(std::shared_ptr<sg::Node> sceneGraph, const std::string& path);
bool WriteSceneGraphToGltf(sg::Node* sceneGraph, const std::string& path);

} // namespace gltf
} // namespace io
} // namespace standard_cyborg
