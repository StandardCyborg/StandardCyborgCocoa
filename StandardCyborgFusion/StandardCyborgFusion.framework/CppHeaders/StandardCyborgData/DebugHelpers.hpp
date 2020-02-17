//
//  DebugHelpers.hpp
//  StandardCyborgGeometry
//
//  Created by Ricky Reusser on 3/31/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <ostream>
#include <memory>

namespace StandardCyborg {

namespace MeshTopology {
class Edge;
class FaceEdges;
class MeshTopology;
}

struct BoundingBox3;
class DepthImage;
class Face3;
class Geometry;
class ColorImage;
struct Mat3x4;
struct Mat3x3;
struct Mat4x4;
struct Plane;
class Polyline;
class Vec2;
class Vec3;
class Vec4;
struct Quaternion;
struct Point2D;
struct Size2D;
struct Rect2D;
class VertexSelection;
class PerspectiveCamera;

struct Node;
struct GeometryNode;
struct ColorImageNode;
struct DepthImageNode;
struct PerspectiveCameraNode;
struct LabelsNode;
struct LandmarkNode;
struct PlaneNode;
struct PolylineNode;
struct ValueFieldNode;
struct CoordinateFrameNode;
struct PointNode;

std::ostream& operator<<(std::ostream& os, const Point2D& v);
std::ostream& operator<<(std::ostream& os, const Size2D& v);
std::ostream& operator<<(std::ostream& os, const Rect2D& v);
std::ostream& operator<<(std::ostream& os, const Vec2& v);
std::ostream& operator<<(std::ostream& os, const Vec3& v);
std::ostream& operator<<(std::ostream& os, const Vec4& v);
std::ostream& operator<<(std::ostream& os, const Quaternion& v);
std::ostream& operator<<(std::ostream& os, const Face3& v);
std::ostream& operator<<(std::ostream& os, const Mat4x4& v);
std::ostream& operator<<(std::ostream& os, const Mat3x4& v);
std::ostream& operator<<(std::ostream& os, const Mat3x3& v);
std::ostream& operator<<(std::ostream& os, const Geometry& g);
std::ostream& operator<<(std::ostream& os, const ColorImage& i);
std::ostream& operator<<(std::ostream& os, const DepthImage& i);
std::ostream& operator<<(std::ostream& os, const VertexSelection& selection);
std::ostream& operator<<(std::ostream& os, const Plane& plane);
std::ostream& operator<<(std::ostream& os, const MeshTopology::Edge& edge);
std::ostream& operator<<(std::ostream& os, const MeshTopology::FaceEdges& edge);
std::ostream& operator<<(std::ostream& os, const MeshTopology::MeshTopology& topology);
std::ostream& operator<<(std::ostream& os, const Polyline& polyline);
std::ostream& operator<<(std::ostream& os, const BoundingBox3& bbox);
std::ostream& operator<<(std::ostream& os, const PerspectiveCamera& camera);

// Scene graph debugging
std::ostream& operator<<(std::ostream& os, const Node& v);

} // using namespace StandardCyborg
