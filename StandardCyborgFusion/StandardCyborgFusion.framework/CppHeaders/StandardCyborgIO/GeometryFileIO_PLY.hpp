//
//  GeometryFileIO_PLY.hpp
//  StandardCyborgGeometry
//
//  Created by Aaron Thompson on 4/2/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <istream>
#include <memory>
#include <string>
#include <ostream>

namespace StandardCyborg {

class Geometry;

/** Assumes file has sRGB color space, and reads into linear color space */
extern bool ReadGeometryFromPLYFile(Geometry& geometryOut, std::string filename);

/** Read geometry from a PLY file into an existing Geometry instance */
extern bool ReadGeometryFromPLYStream(Geometry& geometryOut, std::istream& input);

/** Assumes vertices are in linear color space, and writes to file in sRGB color space */
extern bool WriteGeometryToPLYStream(std::ostream& output, const Geometry& geometry);

/** Assumes vertices are in linear color space, and writes to file in sRGB color space */
extern bool WriteGeometryToPLYFile(std::string filename, const Geometry& geometry);


void FragileWriteGeometryToPLYStream(std::ostream& output, const Geometry& geometry);
bool FragileReadGeometryFromPLYFile(Geometry& geometryOut, std::string filename);

} // namespace StandardCyborg
