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
#include "standard_cyborg/sc3d/Landmark.hpp"
#include "standard_cyborg/scene_graph/SceneGraph.hpp"

namespace standard_cyborg {
namespace scene_graph {

using sc3d::Landmark;

LandmarkNode::LandmarkNode()
    : Node()
{
    landmark.reset(new Landmark());
}

LandmarkNode::LandmarkNode(const std::string& name_, std::shared_ptr<Landmark>landmark_)
    : Node()
{
    name = name_;
    if (landmark_ == nullptr) {
        landmark = std::shared_ptr<Landmark>(new Landmark());
    } else {
        landmark = landmark_;
    }
}

LandmarkNode::LandmarkNode(const std::string& name_, const Landmark& landmark_)
    : Node()
{
    name = name_;
    landmark = std::shared_ptr<Landmark>(new Landmark(landmark_));
}

SGNodeType LandmarkNode::getType() const
{
    return SGNodeType::Landmark;
}

int LandmarkNode::approximateSizeInBytes() const
{
    int totalSize = 0;
    
    totalSize += landmark->name.size() == 0 ? 0 : landmark->getName().size() * sizeof(landmark->getName()[0]);
    totalSize += sizeof(landmark->position);
    
    return totalSize;
}

LandmarkNode* LandmarkNode::copy() const
{
    LandmarkNode* node = new LandmarkNode();
    
    copyToTarget(node);
    
    node->landmark = landmark;
    
    return node;
}

LandmarkNode* LandmarkNode::deepCopy() const
{
    LandmarkNode* node = new LandmarkNode();
    
    copyToTarget(node);
    
    node->id = xg::newGuid();
    node->landmark = landmark;
    
    return node;
}

Landmark& LandmarkNode::getLandmark()
{
    return *landmark;
}

const Landmark& LandmarkNode::getLandmark() const
{
    return *landmark;
}

void LandmarkNode::setLandmark(std::shared_ptr<Landmark> landmark_)
{
    SCASSERT(landmark_ != nullptr, "Shared pointer argument to LandmarkNode::setLandmark may not be null");
    landmark = landmark_;
}

void LandmarkNode::setLandmark(const Landmark& landmark_)
{
    landmark = std::shared_ptr<Landmark>(new Landmark());
    *landmark = landmark_;
}

#ifdef EMBIND_ONLY
Landmark* LandmarkNode::getLandmarkPtr()
{
    return landmark.get();
}
#endif // EMBIND_ONLY

} // namespace scene_graph
} // namespace standard_cyborg
