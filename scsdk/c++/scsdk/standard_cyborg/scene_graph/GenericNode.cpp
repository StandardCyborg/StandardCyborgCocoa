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

#include "standard_cyborg/scene_graph/SceneGraph.hpp"

namespace standard_cyborg {
namespace scene_graph {

Node::~Node()
{
   // printf("== delete '%s'\n", name.c_str());
    allocatedResources.erase(resourceId);
}

SGNodeType Node::getType() const { return SGNodeType::Generic; }

int Node::approximateSizeInBytes() const
{
    return 0;
}

Node* Node::copy() const
{
    Node* node = new Node();
    
    copyToTarget(node);
    
    return node;
}

Node* Node::deepCopy() const
{
    Node* node = new Node();
    copyToTarget(node);
    node->id = xg::newGuid();
    
    return node;
}

std::shared_ptr<sc3d::Geometry> Node::getRepresentationGeometry() const {
    return nullptr;
}

bool Node::hasRepresentationGeometry() const {
    return false;
}

} // namespace scene_graph
} // namespace standard_cyborg
