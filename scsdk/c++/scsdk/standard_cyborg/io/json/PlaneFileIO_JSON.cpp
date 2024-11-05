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

#include "standard_cyborg/io/json/PlaneFileIO_JSON.hpp"
#include "standard_cyborg/sc3d/Plane.hpp"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>

#include <nlohmann/json.hpp>
using JSON = nlohmann::json;

namespace standard_cyborg {
namespace io {
namespace json {

using sc3d::Plane;

bool WritePlaneToFile(std::string filename, const sc3d::Plane& plane)
{
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    
    if (!file.good()) {
        file.close();
        return false;
    }
    
    bool status = WritePlaneToJSONStream(file, plane);
    
    file.close();
    
    return status;
}

bool ReadPlaneFromFile(sc3d::Plane& destination, std::string filename)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    
    if (!file.good()) {
        file.close();
        return false;
    }
    
    bool status = ReadPlaneFromJSONStream(destination, file);
    
    file.close();
    
    return status;
}

bool ReadPlaneFromJSONStream(sc3d::Plane& plane, std::istream& input)
{
    JSON json;
    
    input >> json;
    
    // Sanity check
    if (json["version"] != "1.0.0") { // unsupported version.
        std::cerr << "ReadJSONIntoPlane: unsupported version" << std::endl;
        return false;
    }
    if (json["data"]["planePosition"].size() != 3) {
        std::cerr << "ReadJSONIntoPlane: invalid position data" << std::endl;
        return false;
    }
    if (json["data"]["planeNormal"].size() != 3) {
        std::cerr << "ReadJSONIntoPlane: invalid normal data" << std::endl;
        return false;
    }
    
    plane.position.x = json["data"]["planePosition"][0];
    plane.position.y = json["data"]["planePosition"][1];
    plane.position.z = json["data"]["planePosition"][2];
    plane.normal.x = json["data"]["planeNormal"][0];
    plane.normal.y = json["data"]["planeNormal"][1];
    plane.normal.z = json["data"]["planeNormal"][2];
    
    return true;
}

bool WritePlaneToJSONStream(std::ostream& output, const sc3d::Plane& plane)
{
    JSON json;
    
    json["data"]["planePosition"] = std::vector<float>{
        plane.position.x,
        plane.position.y,
        plane.position.z
    };
    json["data"]["planeNormal"] = std::vector<float>{
        plane.normal.x,
        plane.normal.y,
        plane.normal.z
    };
    
    json["version"] = "1.0.0";
    
    output << std::setw(4) << json;
    
    return true;
}

} // namespace json
} // namespace io
} // namespace standard_cyborg
