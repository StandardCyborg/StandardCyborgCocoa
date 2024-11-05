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


#include "standard_cyborg/sc3d/Face3.hpp"
#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/sc3d/PerspectiveCamera.hpp"
#include "standard_cyborg/scene_graph/SceneGraph.hpp"

#include <algorithm>

namespace standard_cyborg {
namespace scene_graph {

using sc3d::PerspectiveCamera;
using sc3d::Face3;
using sc3d::Geometry;

PerspectiveCameraNode::PerspectiveCameraNode()
  : Node()
{
    camera.reset(new PerspectiveCamera());
}

PerspectiveCameraNode::PerspectiveCameraNode(const std::string& name_, std::shared_ptr<PerspectiveCamera>camera_) :
    PerspectiveCameraNode()
{
    name = name_;
    if (camera_ == nullptr) {
        camera = std::shared_ptr<PerspectiveCamera>(new PerspectiveCamera());
    } else {
        camera = camera_;
    }
}

PerspectiveCameraNode::PerspectiveCameraNode(const std::string& name_, const PerspectiveCamera& camera_) :
    PerspectiveCameraNode()
{
    name = name_;
    camera = std::shared_ptr<PerspectiveCamera>(new PerspectiveCamera(camera_));
}

PerspectiveCameraNode::~PerspectiveCameraNode() {}

SGNodeType PerspectiveCameraNode::getType() const { return SGNodeType::PerspectiveCamera; }

int PerspectiveCameraNode::approximateSizeInBytes() const
{
    int totalSize = 0;
    
    //totalSize += this->camera->getSizeInBytes();
    //
    return totalSize;
}

PerspectiveCameraNode* PerspectiveCameraNode::copy() const
{
    PerspectiveCameraNode* cameraNode = new PerspectiveCameraNode();

    copyToTarget(cameraNode);

    cameraNode->camera = std::shared_ptr<PerspectiveCamera>(this->camera);

    return cameraNode;
}

PerspectiveCameraNode* PerspectiveCameraNode::deepCopy() const
{
    PerspectiveCameraNode* cameraNode = new PerspectiveCameraNode();
    
    copyToTarget(cameraNode);
    
    cameraNode->id = xg::newGuid();
    
    cameraNode->camera.reset(new PerspectiveCamera());
    *cameraNode->camera = *this->camera;

    return cameraNode;
}

const PerspectiveCamera& PerspectiveCameraNode::getPerspectiveCamera() const
{
    return *camera;
}

PerspectiveCamera& PerspectiveCameraNode::getPerspectiveCamera()
{
    return *camera;
}

void PerspectiveCameraNode::setPerspectiveCamera(std::shared_ptr<PerspectiveCamera> otherPerspectiveCamera)
{
    camera = otherPerspectiveCamera;
}

void PerspectiveCameraNode::setPerspectiveCamera(const PerspectiveCamera& otherCamera)
{
    camera = std::shared_ptr<PerspectiveCamera>(new PerspectiveCamera());
    camera->copy(otherCamera);
}

bool PerspectiveCameraNode::hasRepresentationGeometry() const
{
    return true;
}

std::shared_ptr<Geometry> PerspectiveCameraNode::getRepresentationGeometry() const
{
    using math::Vec3;
    
    const PerspectiveCamera& camera = getPerspectiveCamera();
    math::Mat4x4 P = camera.getProjectionViewMatrix(0.05, 0.2).inverse();

    Vec3 p000 = P * Vec3{-1, -1, -1};
    Vec3 p100 = P * Vec3{1, -1, -1};
    Vec3 p010 = P * Vec3{-1, 1, -1};
    Vec3 p110 = P * Vec3{1, 1, -1};
    Vec3 p001 = P * Vec3{-1, -1, 1};
    Vec3 p101 = P * Vec3{1, -1, 1};
    Vec3 p011 = P * Vec3{-1, 1, 1};
    Vec3 p111 = P * Vec3{1, 1, 1};

    std::shared_ptr<Geometry> geometry(new Geometry(
        // Positions
        std::vector<Vec3>{
            p000, p100, p010, p110,
            p001, p101, p011, p111
        },
        // Normals
        std::vector<Vec3>{
            p000, p100, p010, p110,
            p001, p101, p011, p111
        },
        // Colors
        std::vector<Vec3>{
            {1.0, 1.0, 1.0},
            {1.0, 1.0, 1.0},
            {1.0, 1.0, 1.0},
            {1.0, 1.0, 1.0},
            {1.0, 1.0, 1.0},
            {1.0, 1.0, 1.0},
            {1.0, 1.0, 1.0},
            {1.0, 1.0, 1.0}
        },
        // Faces
        std::vector<Face3>{
            {2, 0, 1},
            {2, 1, 3},
            {3, 1, 5},
            {3, 5, 7},
            {6, 5, 4},
            {7, 5, 6},
            {4, 0, 2},
            {4 ,2, 6},
            {6, 2, 3},
            {6, 3, 7},
            {1, 0, 4},
            {5, 1, 4}
        }
    ));

    return geometry;
}

#ifdef EMBIND_ONLY
PerspectiveCamera* PerspectiveCameraNode::getPerspectiveCameraPtr()
{
    return camera.get();
}
#endif // EMBIND_ONLY

} // namespace scene_graph
} // namespace standard_cyborg

