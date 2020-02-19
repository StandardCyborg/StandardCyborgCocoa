//
//  PlaneFileIO_JSON.hpp
//  StandardCyborgIO
//
//  Created by Aaron Thompson on 8/26/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <istream>
#include <ostream>

namespace StandardCyborg {
    
class Plane;

/** Read a plane from a JSON file */
extern bool ReadPlaneFromJSONFile(Plane& planeOut, std::string filename);

/** Read a plane from a stream of JSON data */
extern bool ReadPlaneFromJSONStream(Plane& planeOut, std::istream& input);

/** Serialize a plane as JSON to an output stream */
extern bool WritePlaneToJSONStream(std::ostream& output, const Plane& plane);

/** Write a plane to a JSON file */
extern bool WritePlaneToJSONFile(std::string filename, const Plane& plane);

} // namespace StandardCyborg
