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

#include <istream>
#include <memory>
#include <string>
#include <ostream>

namespace standard_cyborg {

namespace sc3d {
class Geometry;
}

namespace io {
namespace ply {

/** Assumes file has sRGB color space, and reads into linear color space */
extern bool ReadGeometryFromPLYFile(sc3d::Geometry& geometryOut, std::string filename);

/** Read geometry from a PLY file into an existing Geometry instance */
extern bool ReadGeometryFromPLYStream(sc3d::Geometry& geometryOut, std::istream& input);

/** Assumes vertices are in linear color space, and writes to file in sRGB color space */
extern bool WriteGeometryToPLYStream(std::ostream& output, const sc3d::Geometry& geometry);

/** Assumes vertices are in linear color space, and writes to file in sRGB color space */
extern bool WriteGeometryToPLYFile(std::string filename, const sc3d::Geometry& geometry);


void FragileWriteGeometryToPLYStream(std::ostream& output, const sc3d::Geometry& geometry);
bool FragileReadGeometryFromPLYFile(sc3d::Geometry& geometryOut, std::string filename);

} // namespace ply
} // namespace io
} // namespace standard_cyborg
