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

#include <nlohmann/json.hpp>

namespace standard_cyborg {

namespace sc3d {
class PerspectiveCamera;
}

namespace io {
namespace json {

/** Read a perspective camera from a parsed JSON object */
extern bool ReadPerspectiveCameraFromJSON(sc3d::PerspectiveCamera& cameraOut, const nlohmann::json& json);

/** Write a perspective camera to a JSON object */
extern bool WritePerspectiveCameraToJSON(nlohmann::json& json, const sc3d::PerspectiveCamera& camera);

} // namespace json
} // namespace io
} // namespace standard_cyborg
