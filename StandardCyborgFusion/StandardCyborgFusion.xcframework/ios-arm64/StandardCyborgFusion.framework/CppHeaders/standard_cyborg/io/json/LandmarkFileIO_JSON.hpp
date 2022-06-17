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
class Landmark;
}

namespace io {
namespace json {

/** Read a landmark from a stream of JSON data */
extern bool ReadLandmarkFromJSONStream(sc3d::Landmark& landmarkOut, std::istream& input);

/** Read a landmark from a JSON file */
extern bool ReadLandmarkFromJSONFile(sc3d::Landmark& landmarkOut, std::string filename);

/** Serialize a landmark as JSON to an output stream */
extern bool WriteLandmarkToJSONStream(std::ostream& output, const sc3d::Landmark& landmark);

/** Write a landmark to a JSON file */
extern bool WriteLandmarkToJSONFile(std::string filename, const sc3d::Landmark& landmark);

} // namespace json
} // namespace io
} // namespace standard_cyborg

