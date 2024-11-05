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

#include "standard_cyborg/util/DebugHelpers.hpp"
#include <map>
#include <algorithm>
#include <sstream>

#include "standard_cyborg/math/Mat3x4.hpp"
#include "standard_cyborg/math/Mat4x4.hpp"
#include "standard_cyborg/math/Mat3x3.hpp"
#include "standard_cyborg/math/Vec2.hpp"
#include "standard_cyborg/math/Vec3.hpp"
#include "standard_cyborg/math/Vec4.hpp"
#include "standard_cyborg/math/Quaternion.hpp"
#include "standard_cyborg/sc3d/BoundingBox3.hpp"
#include "standard_cyborg/sc3d/ColorImage.hpp"
#include "standard_cyborg/sc3d/DepthImage.hpp"
#include "standard_cyborg/sc3d/Face3.hpp"
#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/sc3d/MeshTopology.hpp"
#include "standard_cyborg/sc3d/Plane.hpp"
#include "standard_cyborg/sc3d/Polyline.hpp"
#include "standard_cyborg/sc3d/VertexSelection.hpp"
#include "standard_cyborg/sc3d/Point2D.hpp"
#include "standard_cyborg/sc3d/Size2D.hpp"
#include "standard_cyborg/sc3d/Rect2D.hpp"
#include "standard_cyborg/sc3d/PerspectiveCamera.hpp"
#include "standard_cyborg/sc3d/Landmark.hpp"
#include "standard_cyborg/scene_graph/SceneGraph.hpp"

#include <iomanip>

using namespace standard_cyborg;
using namespace standard_cyborg::math;
using namespace standard_cyborg::sc3d;
using namespace standard_cyborg::scene_graph;

// namespace standard_cyborg {

// namespace math {

std::ostream& operator<<(std::ostream& os, const Mat4x4& m)
{
    os << "Mat4x4{\n";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m00 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m01 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m02 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m03 << ",\n";
    
    os << "  " << std::setw(11) << std::setprecision(6) << m.m10 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m11 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m12 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m13 << ",\n";
    
    os << "  " << std::setw(11) << std::setprecision(6) << m.m20 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m21 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m22 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m23 << ",\n";
    
    os << "  " << std::setw(11) << std::setprecision(6) << m.m30 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m31 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m32 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m33 << "\n";
    
    os << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Mat3x4& m)
{
    os << "Mat3x4{\n";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m00 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m01 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m02 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m03 << ",\n";
    
    os << "  " << std::setw(11) << std::setprecision(6) << m.m10 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m11 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m12 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m13 << ",\n";
    
    os << "  " << std::setw(11) << std::setprecision(6) << m.m20 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m21 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m22 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m23 << "\n";
    
    os << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Mat3x3& m)
{
    os << "Mat3x3{\n";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m00 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m01 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m02 << ",\n";
    
    os << "  " << std::setw(11) << std::setprecision(6) << m.m10 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m11 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m12 << ",\n";
    
    os << "  " << std::setw(11) << std::setprecision(6) << m.m20 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m21 << ", ";
    os << "  " << std::setw(11) << std::setprecision(6) << m.m22 << "\n";
    
    os << "}";
    return os;
}

    
std::ostream& operator<<(std::ostream& os, const Vec2& v)
{
    os << "Vec2{" << v.x << "f, " << v.y << "f}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Vec3& v)
{
    os << "Vec3{" << v.x << "f, " << v.y << "f, " << v.z << "f}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Vec4& v)
{
    os << "Vec4{" << v.x << "f, " << v.y << "f, " << v.z << "f, " << v.w << "f}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Quaternion& v)
{
    os << "Quaternion{" << v.x << "f, " << v.y << "f, " << v.z << "f, " << v.w << "f}";
    return os;
}

// } // namespace math


// namespace sc3d {

std::ostream& operator<<(std::ostream& os, const Point2D& v)
{
    os << "Point2D{" << v.x << "f, " << v.y << "f}";
    return os;
    }
    
std::ostream& operator<<(std::ostream& os, const Size2D& v)
{
    os << "Size2D{" << v.width << "f, " << v.height << "f}";
    return os;
    }
    
std::ostream& operator<<(std::ostream& os, const Rect2D& v)
{
    os << "Rect2D{" << v.origin << ", " << v.size << "}";
    return os;
}


std::ostream& operator<<(std::ostream& os, const Face3& f)
{
    os << "Face3{" << f[0] << ", " << f[1] << ", " << f[2] << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Geometry& g)
{
    os << "Geometry ({\n";
    int n = g.vertexCount();
    
    if (g.hasPositions()) {
        os << "  std::vector<Vec3>({ /* positions */\n";
        for (int i = 0; i < n; i++) {
            os << "    " << g.getPositions()[i];
            if (i < n - 1) {
                os << ",\n";
            } else {
                os << "\n";
            }
        }
        os << "  }),\n";
    }
    
    if (g.hasNormals()) {
        os << "  std::vector<Vec3>({ /* normals */\n";
        for (int i = 0; i < n; i++) {
            os << "    " << g.getNormals()[i];
            if (i < n - 1) {
                os << ",\n";
            } else {
                os << "\n";
            }
        }
        os << "  }),\n";
    }
    
    if (g.hasColors()) {
        os << "  std::vector<Vec3>({ /* colors */\n";
        for (int i = 0; i < n; i++) {
            os << "    " << g.getColors()[i];
            if (i < n - 1) {
                os << ",\n";
            } else {
                os << "\n";
            }
        }
        os << "  }),\n";
    }
    
    if (g.hasTexCoords()) {
        os << "  std::vector<Vec2>({ /* texCoords */\n";
        for (int i = 0; i < n; i++) {
            os << "    " << g.getTexCoords()[i];
            if (i < n - 1) {
                os << ",\n";
            } else {
                os << "\n";
            }
        }
        os << "  }),\n";
    }

    if (g.hasFaces()) {
        os << "  std::vector<Face3>({ /* faces */\n";
        n = g.faceCount();
        for (int i = 0; i < n; i++) {
            os << "    " << g.getFaces()[i];
            if (i < n - 1) {
                os << ",\n";
            } else {
                os << "\n";
            }
        }
        os << "  })\n";
    }
    
    os << "})";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ColorImage& image)
{
    os << "ColorImage (" << image.getWidth() << ", " << image.getHeight() << ", {\n";
    int n = image.getWidth() * image.getHeight();
    for (int i = 0; i < n; i++) {
        os << "  " << image.getData()[i];
        if (i < n - 1) os << ",\n";
    }
    os << "})";
    return os;
}

std::ostream& operator<<(std::ostream& os, const DepthImage& frame)
{
    os << "DepthImage (" << frame.getWidth() << ", " << frame.getHeight() << ", {";
    int n = frame.getWidth() * frame.getHeight();
    for (int i = 0; i < frame.getWidth() * frame.getHeight(); i++) {
        os << frame.getData()[i];
        if (i < n - 1) os << ", ";
    }
    os << "})";
    return os;
}


std::ostream& operator<<(std::ostream& os, const VertexSelection& selection)
{
    int size = selection.size();
    os << "VertexSelection{";
    int i = 0;
    
    for (auto index : selection) {
        os << index;
        if (i < size - 1) os << ", ";
        i++;
    }
    
    os << "}";
    
    return os;
}

std::ostream& operator<<(std::ostream& os, const Plane& p)
{
    os << "Plane{position=" << p.position << ", normal=" << p.normal << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const MeshTopology::Edge& edge)
{
    os << "Edge{vertices={" << edge.vertex0 << ", " << edge.vertex1 << "}, faces={" << edge.face0 << ", " << edge.face1 << "}}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const MeshTopology::FaceEdges& faceEdge)
{
    os << "FaceEdges{" << faceEdge[0] << ", " << faceEdge[1] << ", " << faceEdge[2] << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const MeshTopology::MeshTopology& topology)
{
    os << "MeshTopology{\n";
    
    os << "  vector<Edge>={\n";
    for (int i = 0; i < topology.getEdges().size(); i++) {
        os << "    " << i << ": " << topology.getEdges()[i] << ",\n";
    }
    os << "  },\n";
    
    os << "  vector<FaceEdges>={\n";
    for (int i = 0; i < topology.getFaceEdges().size(); i++) {
        os << "    " << i << ": " << topology.getFaceEdges()[i] << ",\n";
    }
    os << "  },\n";
    
    os << "  vector<VertexEdges>={\n";
    for (int j = 0; j < topology.getVertexEdges().size(); j++) {
        auto vertexEdges = topology.getVertexEdges()[j];
        int size = static_cast<int>(vertexEdges.size());
        os << "    " << j << ": set<int>{";
        int i = 0;
        for (auto index : vertexEdges) {
            os << index;
            if (i < size - 1) os << ", ";
            i++;
        }
        os << "},\n";
    }
    os << "  },\n";
    
    os << "}" << std::endl;
    
    return os;
}

std::ostream& operator<<(std::ostream& os, const Polyline& polyline)
{
    os << "Polyline{" << std::endl;
    for (auto point : polyline.getPositions()) {
        os << "  " << point << "," << std::endl;
    }
    os << "}" << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream& os, const BoundingBox3& bbox)
{
    os << "BoundingBox3{" << bbox.lower << ", " << bbox.upper << "}";
    return os;
}
    
std::ostream& operator<<(std::ostream& os, const PerspectiveCamera& camera)
{
    os << "PerspectiveCamera{\n";
    os << camera.getNominalIntrinsicMatrix() << ", // nominal intrinsic matrix\n";
    os << camera.getIntrinsicMatrixReferenceSize() << ", // intrinsic matrix reference size\n";
    os << camera.getFocalLengthScaleFactor() << ", // focal length scale factor\n";
    os << camera.getExtrinsicMatrix() << ", // extrinsic matrix\n";
    os << camera.getOrientationMatrix() << ", // orientation matrix\n";
    os << "}";
    return os;
}

// } // namespace sc3d


// namespace scene_graph {

static int scenegraphDebugIndentationLevel = 0;

std::ostream& operator<<(std::ostream& os, const Node& node)
{
    std::stringstream line;
    
    for (int i = 0; i < scenegraphDebugIndentationLevel; i++) line << "    ";

    line << "\"" << node.getName() << "\" ";
    
    std::map<std::string, std::string> props;

    switch(node.getType()) {
        case SGNodeType::ColorImage: {
            line << "<ColorImage>";
            const sc3d::ColorImage& image = node.asColorImageNode()->getColorImage();
            props["width"] = std::to_string(image.getWidth());
            props["height"] = std::to_string(image.getHeight());
            break;
        }
        case SGNodeType::DepthImage: {
            const sc3d::DepthImage& image = node.asDepthImageNode()->getDepthImage();
            props["width"] = std::to_string(image.getWidth());
            props["height"] = std::to_string(image.getHeight());
            line << "<DepthImage>";
            break;
        }
        case SGNodeType::CoordinateFrame:
            line << "<CoordinateFrame>";
            break;
        case SGNodeType::Generic:
            line << "<Node>";
            break;
        case SGNodeType::Geometry: {
            line << "<Geometry>";
            const sc3d::Geometry& geo = node.asGeometryNode()->getGeometry();
            props["vertices"] = std::to_string(geo.vertexCount());
            props["faces"] = std::to_string(geo.faceCount());
            break;
        }
        case SGNodeType::Landmark:
            line << "<Landmark>";
            break;
        case SGNodeType::PerspectiveCamera:
            line << "<PerspectiveCamera>";
            break;
        case SGNodeType::Plane:
            line << "<Plane>";
            break;
        case SGNodeType::Point: {
            line << "<Point>";
            std::stringstream str;
            str << node.asPointNode()->getAsLandmark().getPosition();
            props["position"] = str.str();
            break;
        }
        case SGNodeType::Polyline:
            line << "<Polyline>";
            break;
        default:
            line << "<UNHANDLED NODE TYPE>";
    }
    
    std::string lineStr (line.str());

    os << lineStr;
    
    for (int i = std::max(static_cast<int>(50 - lineStr.size()), 1); i > 0; i--) {
        os << " ";
    }

    os << " (id: " << node.getId() << ", rev: " << node.getNodeRevision() << "," << node.getTreeRevision() << ")\n";
             
    std::for_each(props.begin(), props.end(), [&](std::pair<std::string, std::string> element) {
        for (int i = 0; i < scenegraphDebugIndentationLevel; i++) os << "    ";
        os << "    " << element.first << ": " << element.second << "\n";
    });
    
    scenegraphDebugIndentationLevel++;
    for (int i = 0; i < node.numChildren(); i++) {
        os << *node.getChild(i);
    }
    scenegraphDebugIndentationLevel--;
    return os;
}

// } // namespace scene_graph

// } // namespace standard_cyborg
