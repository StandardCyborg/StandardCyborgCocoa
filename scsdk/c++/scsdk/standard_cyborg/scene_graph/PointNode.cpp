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
#include "standard_cyborg/sc3d/Landmark.hpp"

namespace standard_cyborg {
namespace scene_graph {

using sc3d::Landmark;

PointNode::PointNode()
    : Node()
{
    setTransform(math::Transform());
}

PointNode::PointNode(const std::string& name_, const math::Vec3& position_)
    : Node()
{
    name = name_;
    setTransform(math::Mat3x4::fromTranslation(position_));
}

PointNode::PointNode(const Landmark& landmark)
    : Node()
{
    name = landmark.name;
    setTransform(math::Mat3x4::fromTranslation(landmark.position));
}

PointNode::~PointNode() {}

SGNodeType PointNode::getType() const { return SGNodeType::Point; }

int PointNode::approximateSizeInBytes() const { return sizeof(math::Mat3x4); }

PointNode* PointNode::copy() const
{
    PointNode* node = new PointNode();
    
    copyToTarget(node);
    
    return node;
}

PointNode* PointNode::deepCopy() const
{
    PointNode* node = new PointNode();
    
    copyToTarget(node);
    
    node->id = xg::newGuid();

    return node;
}

Landmark PointNode::getAsLandmark() const {
    math::Mat3x4 transform (math::Mat3x4::fromTransform(getTransform()));
    return Landmark {
        getName(),
        { transform.m03, transform.m13, transform.m23 }
    };
}

#ifdef EMBIND_ONLY
Landmark PointNode::getLandmark() const {
    return Landmark { getName(), {0.0f, 0.0f, 0.0f} };
}
#endif

} // namespace scene_graph
} // namespace standard_cyborg
