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

#include "standard_cyborg/util/AssertHelper.hpp"
#include "standard_cyborg/sc3d/Plane.hpp"
#include "standard_cyborg/scene_graph/SceneGraph.hpp"

namespace standard_cyborg {
namespace scene_graph {

using sc3d::Plane;

PlaneNode::PlaneNode()
    : Node()
{
    plane.reset(new Plane());
}

PlaneNode::PlaneNode(const std::string& name_, std::shared_ptr<Plane> plane_)
    : Node()
{
    name = name_;
    if (plane_ == nullptr) {
        plane = std::shared_ptr<Plane>(new Plane());
    } else {
        plane = plane_;
    }
}

PlaneNode::PlaneNode(const std::string& name_, const Plane& plane_)
    : Node()
{
    name = name_;
    plane = std::shared_ptr<Plane>(new Plane(plane_));
}

SGNodeType PlaneNode::getType() const { return SGNodeType::Plane; }

int PlaneNode::approximateSizeInBytes() const { return sizeof(plane); }

PlaneNode* PlaneNode::copy() const
{
    PlaneNode* node = new PlaneNode();
    
    copyToTarget(node);
    
    node->plane = this->plane;
    
    return node;
}

PlaneNode* PlaneNode::deepCopy() const
{
    PlaneNode* node = new PlaneNode();
    
    copyToTarget(node);
    
    node->id = xg::newGuid();
    node->plane = plane;
    
    return node;
}

Plane& PlaneNode::getPlane()
{
    return *plane;
}

const Plane& PlaneNode::getPlane() const
{
    return *plane;
}

void PlaneNode::setPlane(std::shared_ptr<Plane> plane_)
{
    SCASSERT(plane_ != nullptr, "Shared pointer argument to PlaneNode::setPlane may not be null");
    plane = plane_;
}

void PlaneNode::setPlane(const Plane& plane_)
{
    plane = std::shared_ptr<Plane>(new Plane());
    *plane = plane_;
}

math::Vec2 PlaneNode::getExtents() const
{
    return extents;
}

void PlaneNode::setExtents(math::Vec2 newExtents)
{
    extents = newExtents;
}

#ifdef EMBIND_ONLY
Plane* PlaneNode::getPlanePtr()
{
    return plane.get();
}
#endif // EMBIND_ONLY

} // namespace scene_graph
} // namespace standard_cyborg
