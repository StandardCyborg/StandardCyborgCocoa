//
//  PerspectiveCameraFileIO_JSON.hpp
//  StandardCyborgIO
//
//  Created by Ricky Reusser on 9/11/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <istream>
#include <ostream>

namespace StandardCyborg {

class PerspectiveCamera;

/** Read a perspective camera from a stream of JSON data */
extern bool ReadPerspectiveCameraFromJSONStream(PerspectiveCamera& cameraOut, std::istream& input);

/** Read a perspective camera from a JSON file */
extern bool ReadPerspectiveCameraFromJSONFile(PerspectiveCamera& cameraOut, std::string filename);

/** Serialize a perspective camera as JSON to an output stream */
extern bool WritePerspectiveCameraToJSONStream(std::ostream& output, const PerspectiveCamera& camera);

/** Write a perspective camera to a JSON file */
extern bool WritePerspectiveCameraToJSONFile(std::string filename, const PerspectiveCamera& camera);

} // namespace StandardCyborg
