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


#include "standard_cyborg/io/ply/RawFrameDataIO_PLY.hpp"

#include "standard_cyborg/sc3d/DepthImage.hpp"
#include "standard_cyborg/sc3d/ColorImage.hpp"
#include "standard_cyborg/sc3d/PerspectiveCamera.hpp"
#include "standard_cyborg/io/json/PerspectiveCameraFileIO_JSON_Private.hpp"

#include <algorithm>
#include <iostream>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <happly.h>
#pragma clang diagnostic pop
#include <nlohmann/json.hpp>

namespace standard_cyborg {
namespace io {
namespace ply {

using sc3d::ColorImage;
using sc3d::DepthImage;
using sc3d::PerspectiveCamera;

bool ReadRawFrameDataFromPLYFile(ColorImage& imageOut, DepthImage& depthOut, PerspectiveCamera& cameraOut, std::string filename) {
    RawFrameMetadata metadataToDiscard;
    return ReadRawFrameDataFromPLYFile(imageOut, depthOut, cameraOut, metadataToDiscard, filename);
}

bool ReadRawFrameDataFromPLYFile(ColorImage& imageOut, DepthImage& depthOut, PerspectiveCamera& cameraOut, RawFrameMetadata& rawFrameMetadataOut, std::string filename)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    
    if (!file.good()) {
        file.close();
        return false;
    }
    
    bool status = ReadRawFrameDataFromPLYStream(imageOut, depthOut, cameraOut, rawFrameMetadataOut, file);
    
    file.close();
    
    return status;
}


bool ReadRawFrameDataFromPLYStream(ColorImage& imageOut, DepthImage& depthOut, PerspectiveCamera& cameraOut, std::istream& inStream) {
    RawFrameMetadata metadataToDiscard;
    return ReadRawFrameDataFromPLYStream(imageOut, depthOut, cameraOut, metadataToDiscard, inStream);
}

bool ReadRawFrameDataFromPLYStream(ColorImage& imageOut, DepthImage& depthOut, PerspectiveCamera& cameraOut, RawFrameMetadata& rawFrameMetadataOut, std::istream& inStream)
{
    happly::PLYData plyIn(inStream);
    
    if (!plyIn.hasElement("color")) {
        std::cerr << "ReadPLYIntoRGBDFrame: input PLY has no color data" << std::endl;
        return false;
    }
    
    if (!plyIn.hasElement("depth")) {
        std::cerr << "ReadPLYIntoRGBDFrame: input PLY has no depth data" << std::endl;
        return false;
    }
    
    if (!plyIn.hasElement("metadata")) {
        std::cerr << "ReadPLYIntoRGBDFrame: input PLY has no frame metadata" << std::endl;
        return false;
    }
    
    happly::Element& colorElement = plyIn.getElement("color");
    happly::Element& depthElement = plyIn.getElement("depth");

    const std::vector<float>& r = colorElement.getProperty<float>("r");
    const std::vector<float>& g = colorElement.getProperty<float>("g");
    const std::vector<float>& b = colorElement.getProperty<float>("b");
    const std::vector<float>& d = depthElement.getProperty<float>("d");
    
    const std::vector<uint8_t>& metadataAsChar = plyIn.getElement("metadata").getProperty<unsigned char>("char");
    nlohmann::json metadata = nlohmann::json::parse(metadataAsChar.begin(), metadataAsChar.end());
    
    int width = metadata["width"];
    int height = metadata["height"];
    
    rawFrameMetadataOut.timestamp = metadata["timestamp"];
    
    json::ReadPerspectiveCameraFromJSON(cameraOut, metadata["camera_intrinsics"]);
    
    int dataSize = width * height;
    
    if (r.size() != dataSize || g.size() != dataSize || b.size() != dataSize || d.size() != dataSize) {
        std::cerr << "ReadPLYIntoRGBDFrame: Expected data size (rgba=" << colorElement.count << ", depth=" << depthElement.count << ") to match size declared in metadata (" << dataSize << ")" << std::endl;
        return false;
    }
    
    std::vector<math::Vec4> rgba(dataSize);
    for (size_t i = 0; i < dataSize; i++) {
        rgba[i] = math::Vec4({r[i], g[i], b[i], 1.0f});
    }
    
    imageOut.reset(width, height, rgba);
    depthOut.reset(width, height, d);
    
    return true;
}

} // namespace ply
} // namespace io
} // namespace standard_cyborg
