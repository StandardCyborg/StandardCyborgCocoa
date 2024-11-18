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
#include <ostream>

namespace standard_cyborg {

namespace sc3d {
class PerspectiveCamera;
}

namespace io {
namespace json {

/** Read a perspective camera from a stream of JSON data */
extern bool ReadPerspectiveCameraFromJSONStream(sc3d::PerspectiveCamera& cameraOut, std::istream& input);

/** Read a perspective camera from a JSON file */
extern bool ReadPerspectiveCameraFromJSONFile(sc3d::PerspectiveCamera& cameraOut, std::string filename);

/** Serialize a perspective camera as JSON to an output stream */
extern bool WritePerspectiveCameraToJSONStream(std::ostream& output, const sc3d::PerspectiveCamera& camera);

/** Write a perspective camera to a JSON file */
extern bool WritePerspectiveCameraToJSONFile(std::string filename, const sc3d::PerspectiveCamera& camera);

} // namespace json
} // namespace io
} // namespace standard_cyborg
