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

CoordinateFrameNode::CoordinateFrameNode()
    : Node()
{
    setTransform(math::Transform());
}

CoordinateFrameNode::CoordinateFrameNode(const std::string& name_, const math::Mat3x4& axes_)
    : Node()
{
    name = name_;
    setTransform(axes_);
}

CoordinateFrameNode::~CoordinateFrameNode() {}

SGNodeType CoordinateFrameNode::getType() const { return SGNodeType::CoordinateFrame; }

int CoordinateFrameNode::approximateSizeInBytes() const { return sizeof(math::Mat3x4); }

CoordinateFrameNode* CoordinateFrameNode::copy() const
{
    CoordinateFrameNode* node = new CoordinateFrameNode();
    
    copyToTarget(node);
    
    return node;
}

CoordinateFrameNode* CoordinateFrameNode::deepCopy() const
{
    CoordinateFrameNode* node = new CoordinateFrameNode();
    
    copyToTarget(node);
    
    node->id = xg::newGuid();

    return node;
}

} // namespace scene_graph
} // namespace standard_cyborg

