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
#include "standard_cyborg/sc3d/ColorImage.hpp"
#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/scene_graph/SceneGraph.hpp"

#include <algorithm>

namespace standard_cyborg {
namespace scene_graph {

using sc3d::ColorImage;
using sc3d::Geometry;
using sc3d::Face3;

ColorImageNode::ColorImageNode()
    : Node()
{
    image.reset(new ColorImage());
}

ColorImageNode::ColorImageNode(const std::string& name_, std::shared_ptr<ColorImage> image_)
    : Node()
{
    name = name_;
    if (image_ == nullptr) {
        image = std::shared_ptr<ColorImage>(new ColorImage());
    } else {
        image = image_;
    }
}

ColorImageNode::ColorImageNode(const std::string& name_, const ColorImage& image_)
    : Node()
{
    name = name_;
    image = std::shared_ptr<ColorImage>(new ColorImage());
    image->copy(image_);
}

ColorImageNode::ColorImageNode(const std::string& name_, ColorImage&& image_)
    : Node()
{
    name = name_;
    image = std::shared_ptr<ColorImage>(new ColorImage());
    image->move(std::move(image_));
}

ColorImageNode::~ColorImageNode() {}

SGNodeType ColorImageNode::getType() const { return SGNodeType::ColorImage; }

int ColorImageNode::approximateSizeInBytes() const
{
    int totalSize = 0;
    
    totalSize += this->image->getSizeInBytes();
    
    return totalSize;
}

ColorImageNode* ColorImageNode::copy() const
{
    ColorImageNode* imageNode = new ColorImageNode();
    
    copyToTarget(imageNode);
    
    imageNode->image = std::shared_ptr<ColorImage>(this->image);
    
    return imageNode;
}

ColorImageNode* ColorImageNode::deepCopy() const
{
    ColorImageNode* imageNode = new ColorImageNode();
    
    copyToTarget(imageNode);
    
    imageNode->id = xg::newGuid();
    
    imageNode->image.reset(new ColorImage());
    imageNode->image->copy(*this->image);
    
    return imageNode;
}

ColorImage& ColorImageNode::getColorImage()
{
    return *image;
}

const ColorImage& ColorImageNode::getColorImage() const
{
    return *image;
}

void ColorImageNode::setColorImage(std::shared_ptr<ColorImage> image_)
{
    SCASSERT(image_ != nullptr, "Shared pointer argument to ColorImageNode::setImage may not be null");
    image = image_;
}

void ColorImageNode::setColorImage(const ColorImage& image_)
{
    image = std::shared_ptr<ColorImage>(new ColorImage());
    image->copy(image_);
}

bool ColorImageNode::hasRepresentationGeometry() const
{
    return true;
}

std::shared_ptr<Geometry> ColorImageNode::getRepresentationGeometry() const
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
        // Colors
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
ColorImage* ColorImageNode::getColorImagePtr()
{
    return image.get();
}
#endif // EMBIND_ONLY

} // namespace scene_graph
} // namespace standard_cyborg
