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

#include "standard_cyborg/io/json/PolylineFileIO_JSON.hpp"
#include "standard_cyborg/sc3d/Polyline.hpp"
#include "standard_cyborg/io/json/ParsingHelpers.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <nlohmann/json.hpp>
using JSON = nlohmann::json;

namespace standard_cyborg {
namespace io {
namespace json {

bool WritePolylineToJSONFile(std::string filename, const sc3d::Polyline& polyline)
{
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    
    if (!file.good()) {
        file.close();
        return false;
    }
    
    bool status = WritePolylineToJSONStream(file, polyline);
    
    file.close();
    
    return status;
}

bool ReadPolylineFromJSONFile(sc3d::Polyline& polyline, std::string filename)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    
    if (!file.good()) {
        file.close();
        return false;
    }
    
    bool status = ReadPolylineFromJSONStream(polyline, file);
    
    file.close();
    
    return status;
}

bool ReadPolylineFromJSONStream(sc3d::Polyline& polylineOut, std::istream& input)
{
    JSON json;
    
    input >> json;
    
    // sanity check:
    if (json["version"] != "1.0.0") { // unsupported version.
        std::cerr << "ReadJSONIntoPolyline: unsupported version" << std::endl;
        return false;
    }
    
    std::vector<math::Vec3> positions;
    for (int i = 0; i < json["data"]["polylineNodePositions"].size(); i++) {
        std::vector<float> xyz = toNumericVectorWithNullAsNAN<float>(json["data"]["polylineNodePositions"][i]);
        positions.push_back({xyz[0], xyz[1], xyz[2]});
    }
    
    polylineOut.setPositions(positions);

    return true;
}

bool WritePolylineToJSONStream(std::ostream& output, const sc3d::Polyline& polyline)
{
    JSON json;
    
    const std::vector<math::Vec3>& polylinePositions = polyline.getPositions();
    
    std::vector<std::vector<float>> positions;
    
    for (int i = 0; i < polylinePositions.size(); i++) {
        math::Vec3 pos = polylinePositions[i];
        positions.push_back(std::vector<float>({pos.x, pos.y, pos.z}));
    }
    
    json["data"]["polylineNodePositions"] = positions;
    json["version"] = "1.0.0";
    
    output << std::setw(4) << json;
    
    return true;
}

} // namespace json
} // namespace io
} // namespace standard_cyborg
