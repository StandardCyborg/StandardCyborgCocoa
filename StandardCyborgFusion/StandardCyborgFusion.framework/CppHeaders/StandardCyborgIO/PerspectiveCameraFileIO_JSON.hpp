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

#include "json.hpp"
using JSON = nlohmann::json;

namespace StandardCyborg {

class PerspectiveCamera;

/** Read a perspective camera from a stream of JSON data */
extern bool ReadPerspectiveCameraFromJSONStream(PerspectiveCamera& cameraOut, std::istream& input);

/** Read a perspective camera from a JSON file */
extern bool ReadPerspectiveCameraFromJSONFile(PerspectiveCamera& cameraOut, std::string filename);

/** Read a perspective camera from a parsed JSON object */
extern bool ReadPerspectiveCameraFromJSON(PerspectiveCamera& cameraOut, const JSON& json);

/** Serialize a perspective camera as JSON to an output stream */
extern bool WritePerspectiveCameraToJSONStream(std::ostream& output, const PerspectiveCamera& camera);

/** Write a perspective camera to a JSON file */
extern bool WritePerspectiveCameraToJSONFile(std::string filename, const PerspectiveCamera& camera);

/** Write a perspective camera to a JSON object */
extern bool WritePerspectiveCameraToJSON(JSON& json, const PerspectiveCamera& camera);

} // namespace StandardCyborg
