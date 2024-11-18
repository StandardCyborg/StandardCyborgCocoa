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
class Polyline;
}

namespace io {
namespace json {

/** Read a polyline from an stream of JSON data */
extern bool ReadPolylineFromJSONStream(sc3d::Polyline& polylineOut, std::istream& input);

/** Read a polyline from a JSON file */
extern bool ReadPolylineFromJSONFile(sc3d::Polyline& polylineOut, std::string filename);

/** Serialize a polyline as JSON to an output stream */
extern bool WritePolylineToJSONStream(std::ostream& output, const sc3d::Polyline& polyline);
    
/** Write a polyline to a JSON file */
extern bool WritePolylineToFile(std::string filename, const sc3d::Polyline& polyline);

} // namespace json
} // namespace io
} // namespace standard_cyborg
