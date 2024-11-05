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
#include "standard_cyborg/sc3d/DepthImage.hpp"
#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/scene_graph/SceneGraph.hpp"

#include <algorithm>

namespace standard_cyborg {
namespace scene_graph {

using sc3d::DepthImage;
using sc3d::Geometry;
using sc3d::Face3;

DepthImageNode::DepthImageNode()
    : Node()
{
    image.reset(new DepthImage());
}

DepthImageNode::DepthImageNode(const std::string& name_, std::shared_ptr<DepthImage> image_)
    : Node()
{
    name = name_;
    if (image_ == nullptr) {
        image = std::shared_ptr<DepthImage>(new DepthImage());
    } else {
        image = image_;
    }
}

DepthImageNode::DepthImageNode(const std::string& name_, const DepthImage& image_)
    : Node()
{
    name = name_;
    image = std::shared_ptr<DepthImage>(new DepthImage());
    image->copy(image_);
}

DepthImageNode::DepthImageNode(const std::string& name_, DepthImage&& image_)
    : Node()
{
    name = name_;
    image = std::shared_ptr<DepthImage>(new DepthImage());
    image->move(std::move(image_));
}

DepthImageNode::~DepthImageNode() {}

SGNodeType DepthImageNode::getType() const { return SGNodeType::DepthImage; }

int DepthImageNode::approximateSizeInBytes() const
{
    int totalSize = 0;
    
    totalSize += this->image->getSizeInBytes();
    
    return totalSize;
}

DepthImageNode* DepthImageNode::copy() const
{
    DepthImageNode* imageNode = new DepthImageNode();
    
    copyToTarget(imageNode);
    
    imageNode->image = std::shared_ptr<DepthImage>(this->image);
    
    return imageNode;
}

DepthImageNode* DepthImageNode::deepCopy() const
{
    DepthImageNode* imageNode = new DepthImageNode();
    
    copyToTarget(imageNode);
    
    imageNode->id = xg::newGuid();
    
    imageNode->image.reset(new DepthImage());
    imageNode->image->copy(*this->image);
    
    return imageNode;
}

DepthImage& DepthImageNode::getDepthImage()
{
    return *image;
}

const DepthImage& DepthImageNode::getDepthImage() const
{
    return *image;
}

void DepthImageNode::setDepthImage(std::shared_ptr<DepthImage> image_)
{
    SCASSERT(image_ != nullptr, "Shared pointer argument to DepthImageNode::setImage may not be null");
    image = image_;
}

void DepthImageNode::setDepthImage(const DepthImage& image_)
{
    image = std::shared_ptr<DepthImage>(new DepthImage());
    image->copy(image_);
}

bool DepthImageNode::hasRepresentationGeometry() const
{
    return true;
}

std::shared_ptr<Geometry> DepthImageNode::getRepresentationGeometry() const
{
    float aspectInv = (float)std::max(1, image->getHeight()) / std::max(1, image->getWidth());

    std::shared_ptr<Geometry> geometry(new Geometry(
        // Positions
        std::vector<math::Vec3>{
            {-1.0, -aspectInv, 0.0},
            {1.0, -aspectInv, 0.0},
            {1.0, aspectInv, 0.0},
            {-1.0, aspectInv, 0.0}
        },
        // Normals
        std::vector<math::Vec3>{
            {0.0, 0.0, 1.0},
            {0.0, 0.0, 1.0},
            {0.0, 0.0, 1.0},
            {0.0, 0.0, 1.0}
        },
        // Depths
        std::vector<math::Vec3>{
            {1.0, 1.0, 1.0},
            {1.0, 1.0, 1.0},
            {1.0, 1.0, 1.0},
            {1.0, 1.0, 1.0}
        },
        // Faces
        std::vector<Face3>{
            {0, 1, 2},
            {0, 2, 3}
        }
    ));

    geometry->setTexCoords({
        {0.0, 0.0},
        {1.0, 0.0},
        {1.0, 1.0},
        {0.0, 1.0}
    });
    
    return geometry;
}

#ifdef EMBIND_ONLY
DepthImage* DepthImageNode::getDepthImagePtr()
{
    return image.get();
}
#endif // EMBIND_ONLY


} // namespace scene_graph
} // namespace standard_cyborg
