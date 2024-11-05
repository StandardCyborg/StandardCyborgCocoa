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
#include "standard_cyborg/sc3d/Polyline.hpp"
#include "standard_cyborg/scene_graph/SceneGraph.hpp"


namespace standard_cyborg {
namespace scene_graph {

using sc3d::Polyline;

PolylineNode::PolylineNode() :
    Node()
{
    polyline.reset(new Polyline());
}


PolylineNode::PolylineNode(const std::string& name_, std::shared_ptr<Polyline> polyline_) :
    Node()
{
    name = name_;
    if (polyline_ == nullptr) {
        polyline = std::shared_ptr<Polyline>(new Polyline());
    } else {
        polyline = polyline_;
    }
}

PolylineNode::PolylineNode(const std::string& name_, const Polyline& polyline_) :
    Node()
{
    name = name_;
    polyline = std::shared_ptr<Polyline>(new Polyline());
    polyline->copy(polyline_);
}

SGNodeType PolylineNode::getType() const { return SGNodeType::Polyline; }

int PolylineNode::approximateSizeInBytes() const
{
    int totalSize = polyline->getPositions().size() == 0 ? 0 : (int)(polyline->getPositions().size() * sizeof(polyline->getPositions()[0]));
    
    return totalSize;
}

PolylineNode* PolylineNode::copy() const
{
    PolylineNode* node = new PolylineNode();
    
    copyToTarget(node);
    
    node->polyline = std::shared_ptr<Polyline>(this->polyline);
    
    return node;
}

PolylineNode* PolylineNode::deepCopy() const
{
    PolylineNode* node = new PolylineNode();
    
    copyToTarget(node);
    
    node->id = xg::newGuid();
    node->polyline.reset(new Polyline());
    *node->polyline = *this->polyline;
    
    return node;
}

Polyline& PolylineNode::getPolyline()
{
    return *polyline;
}

const Polyline& PolylineNode::getPolyline() const
{
    return *polyline;
}

void PolylineNode::setPolyline(std::shared_ptr<Polyline> polyline_)
{
    SCASSERT(polyline_ != nullptr, "Shared pointer argument to PolylineNode::setPolyline may not be null");
    polyline = polyline_;
}

void PolylineNode::setPolyline(const Polyline& polyline_)
{
    polyline = std::shared_ptr<Polyline>(new Polyline());
    polyline->copy(polyline_);
}

#ifdef EMBIND_ONLY
Polyline* PolylineNode::getPolylinePtr()
{
    return polyline.get();
}
#endif // EMBIND_ONLY

} // namespace scene_graph
} // namespace standard_cyborg
