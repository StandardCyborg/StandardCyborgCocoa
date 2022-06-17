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

#pragma once

#include <ostream>
#include <memory>

namespace standard_cyborg {

namespace math {
struct Vec2;
struct Vec3;
struct Vec4;

struct Mat3x4;
struct Mat3x3;
struct Mat4x4;

struct Quaternion;

}

namespace sc3d {
namespace MeshTopology {
struct Edge;
struct FaceEdges;
class MeshTopology;
}

struct BoundingBox3;
class DepthImage;
struct Face3;
class Geometry;
class ColorImage;

struct Plane;
class Polyline;

struct Point2D;
struct Size2D;
struct Rect2D;
class VertexSelection;
class PerspectiveCamera;
}

namespace scene_graph {
class Node;
struct GeometryNode;
struct ColorImageNode;
struct DepthImageNode;
struct PerspectiveCameraNode;
struct LandmarkNode;
struct PlaneNode;
struct PolylineNode;
struct CoordinateFrameNode;
struct PointNode;
}

} // namespace standard_cyborg


std::ostream& operator<<(std::ostream& os, const standard_cyborg::math::Vec2& v);
std::ostream& operator<<(std::ostream& os, const standard_cyborg::math::Vec3& v);
std::ostream& operator<<(std::ostream& os, const standard_cyborg::math::Vec4& v);
std::ostream& operator<<(std::ostream& os, const standard_cyborg::math::Quaternion& v);
std::ostream& operator<<(std::ostream& os, const standard_cyborg::math::Mat4x4& v);
std::ostream& operator<<(std::ostream& os, const standard_cyborg::math::Mat3x4& v);
std::ostream& operator<<(std::ostream& os, const standard_cyborg::math::Mat3x3& v);

std::ostream& operator<<(std::ostream& os, const standard_cyborg::sc3d::Point2D& v);
std::ostream& operator<<(std::ostream& os, const standard_cyborg::sc3d::Size2D& v);
std::ostream& operator<<(std::ostream& os, const standard_cyborg::sc3d::Rect2D& v);

std::ostream& operator<<(std::ostream& os, const standard_cyborg::sc3d::Face3& v);

std::ostream& operator<<(std::ostream& os, const standard_cyborg::sc3d::Geometry& g);
std::ostream& operator<<(std::ostream& os, const standard_cyborg::sc3d::ColorImage& i);
std::ostream& operator<<(std::ostream& os, const standard_cyborg::sc3d::DepthImage& i);
std::ostream& operator<<(std::ostream& os, const standard_cyborg::sc3d::VertexSelection& selection);
std::ostream& operator<<(std::ostream& os, const standard_cyborg::sc3d::Plane& plane);
std::ostream& operator<<(std::ostream& os, const standard_cyborg::sc3d::MeshTopology::Edge& edge);
std::ostream& operator<<(std::ostream& os, const standard_cyborg::sc3d::MeshTopology::FaceEdges& edge);
std::ostream& operator<<(std::ostream& os, const standard_cyborg::sc3d::MeshTopology::MeshTopology& topology);
std::ostream& operator<<(std::ostream& os, const standard_cyborg::sc3d::Polyline& polyline);
std::ostream& operator<<(std::ostream& os, const standard_cyborg::sc3d::BoundingBox3& bbox);
std::ostream& operator<<(std::ostream& os, const standard_cyborg::sc3d::PerspectiveCamera& camera);

// Scene graph debugging
std::ostream& operator<<(std::ostream& os, const standard_cyborg::scene_graph::Node& v);



