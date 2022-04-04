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

#include "standard_cyborg/proto/sc3d/triangle_mesh.pb.h"
#include "standard_cyborg/proto/sc3d/point_cloud.pb.h"

#include "standard_cyborg/util/Result.hpp"

namespace standard_cyborg {

namespace sc3d {
class Geometry;
}

namespace io {
namespace pbio {

struct ConvertedGeometry {
    std::optional<standard_cyborg::proto::sc3d::TriangleMesh> triangle_mesh;
    std::optional<standard_cyborg::proto::sc3d::PointCloud> point_cloud;
};

extern Result<sc3d::Geometry> FromPB(const standard_cyborg::proto::sc3d::TriangleMesh &msg);
extern Result<sc3d::Geometry> FromPB(const standard_cyborg::proto::sc3d::PointCloud &msg);

extern Result<ConvertedGeometry> ToPB(const sc3d::Geometry &geometry);
extern Result<sc3d::Geometry> FromPB(const ConvertedGeometry &convertedGeomety);

} // namespace pbio
} // namespace io
} // namespace standard_cyborg
