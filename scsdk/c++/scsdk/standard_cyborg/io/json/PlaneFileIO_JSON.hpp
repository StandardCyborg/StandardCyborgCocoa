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
struct Plane;
}

namespace io {
namespace json {
    
/** Read a plane from a JSON file */
extern bool ReadPlaneFromJSONFile(sc3d::Plane& planeOut, std::string filename);

/** Read a plane from a stream of JSON data */
extern bool ReadPlaneFromJSONStream(sc3d::Plane& planeOut, std::istream& input);

/** Serialize a plane as JSON to an output stream */
extern bool WritePlaneToJSONStream(std::ostream& output, const sc3d::Plane& plane);

/** Write a plane to a JSON file */
extern bool WritePlaneToJSONFile(std::string filename, const sc3d::Plane& plane);

} // namespace json
} // namespace io
} // namespace standard_cyborg
