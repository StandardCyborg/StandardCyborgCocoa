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

#include "standard_cyborg/io/json/LandmarkFileIO_JSON.hpp"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>

#include <nlohmann/json.hpp>
using JSON = nlohmann::json;

#include "standard_cyborg/sc3d/Landmark.hpp"

namespace standard_cyborg {
namespace io {
namespace json {

bool WriteLandmarkToJSONFile(std::string filename, const sc3d::Landmark& landmark)
{
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    
    if (!file.good()) {
        file.close();
        return false;
    }
    
    bool status = WriteLandmarkToJSONStream(file, landmark);
    
    file.close();
    
    return status;
}

bool ReadLandmarkFromJSONFile(sc3d::Landmark& landmark, std::string filename)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    
    if (!file.good()) {
        file.close();
        return false;
    }
    
    bool status = ReadLandmarkFromJSONStream(landmark, file);
    
    file.close();
    
    return status;
}

bool ReadLandmarkFromJSONStream(sc3d::Landmark& destination, std::istream& input)
{
    JSON json;
    
    input >> json;
    
    // Sanity check
    if (json["version"] != "1.0.0") { // unsupported version.
        std::cerr << "ReadJSONIntoLandmark: unsupported version" << std::endl;
        return false;
    }
    
    std::vector<float> position = json["data"]["landmarkPosition"];
    if (position.size() != 3) {
        std::cerr << "ReadJSONIntoLandmark: invalid position data" << std::endl;
        return false;
    }
    
    destination.position = math::Vec3(position[0], position[1], position[2]);
    destination.name = json["data"]["landmarkName"];
    
    return true;
}

bool WriteLandmarkToJSONStream(std::ostream& output, const sc3d::Landmark& landmark)
{
    JSON json;
    
    json["data"]["landmarkPosition"] = std::vector<float>{
        landmark.position.x,
        landmark.position.y,
        landmark.position.z
    };
    
    json["data"]["landmarkName"] = landmark.name;
    json["version"] = "1.0.0";
    
    output << std::setw(4) << json;
    
    return true;
}

} // namespace json
} // namespace io
} // namespace standard_cyborg
