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
#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/scene_graph/SceneGraph.hpp"

namespace standard_cyborg {
namespace scene_graph {

using sc3d::Geometry;

GeometryNode::GeometryNode()
    : Node()
{
    geometry.reset(new Geometry());
}

GeometryNode::GeometryNode(const std::string& name_, std::shared_ptr<Geometry> geometry_)
    : Node()
{
    name = name_;
    
    if (geometry_ == nullptr) {
        geometry = std::shared_ptr<Geometry>(new Geometry());
    } else {
        geometry = geometry_;
    }
}

GeometryNode::GeometryNode(const std::string& name_, const Geometry& geometry_)
    : Node()
{
    name = name_;
    geometry = std::shared_ptr<Geometry>(new Geometry());
    geometry->copy(geometry_);
}

GeometryNode::~GeometryNode() {}

SGNodeType GeometryNode::getType() const { return SGNodeType::Geometry; }

int GeometryNode::approximateSizeInBytes() const
{
    int totalSize = 0;
    
    totalSize += this->geometry->getSize();
    
    return totalSize;
}

GeometryNode* GeometryNode::copy() const
{
    GeometryNode* geoNode = new GeometryNode();
    
    copyToTarget(geoNode);
    
    geoNode->geometry = std::shared_ptr<Geometry>(this->geometry);
    
    return geoNode;
}

GeometryNode* GeometryNode::deepCopy() const
{
    GeometryNode* geoNode = new GeometryNode();
    
    copyToTarget(geoNode);
    
    geoNode->id = xg::newGuid();
    
    geoNode->geometry.reset(new Geometry());
    geoNode->geometry->copy(*this->geometry);
    
    return geoNode;
}

Geometry& GeometryNode::getGeometry()
{
    return *geometry;
}

const Geometry& GeometryNode::getGeometry() const
{
    return *geometry;
}

void GeometryNode::setGeometry(std::shared_ptr<Geometry> geometry_)
{
    SCASSERT(geometry != nullptr, "Shared pointer argument to GeometryNode::setGeometry may not be null");
    geometry = geometry_;
}

void GeometryNode::setGeometry(const Geometry& geometry_)
{
    geometry = std::shared_ptr<Geometry>(new Geometry());
    geometry->copy(geometry_);
}

#ifdef EMBIND_ONLY
Geometry* GeometryNode::getGeometryPtr()
{
    return geometry.get();
}
#endif // EMBIND_ONLY

} // namespace scene_graph
} // namespace standard_cyborg
