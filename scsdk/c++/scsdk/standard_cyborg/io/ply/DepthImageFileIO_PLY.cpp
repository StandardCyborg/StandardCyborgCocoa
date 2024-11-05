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


#include "standard_cyborg/io/ply/DepthImageFileIO_PLY.hpp"

#include "standard_cyborg/sc3d/DepthImage.hpp"

#include <algorithm>
#include <iostream>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "happly.h"
#pragma clang diagnostic pop

namespace standard_cyborg {
namespace io {
namespace ply {

using sc3d::DepthImage;

bool ReadDepthImageFromPLYFile(DepthImage& depthOut, std::string filename)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    
    if (!file.good()) {
        file.close();
        return false;
    }
    
    bool status = ReadDepthImageFromPLYStream(depthOut, file);
    
    file.close();
    
    return status;
}

bool ReadDepthImageFromPLYStream(DepthImage& depthOut, std::istream& inStream)
{
    happly::PLYData plyIn(inStream);
    
    if (!plyIn.hasElement("depth")) {
        std::cerr << "ReadDepthImageFromPLYStream: input PLY has no depth data" << std::endl;
        return false;
    }
    
    if (!plyIn.hasElement("shape")) {
        std::cerr << "ReadDepthImageFromPLYStream: input PLY has no shape metadata" << std::endl;
        return false;
    }
    
    happly::Element& depthElement = plyIn.getElement("depth");
    happly::Element& shapeElement = plyIn.getElement("shape");
    
    const std::vector<float>& depth = depthElement.getProperty<float>("value");
    int width = shapeElement.getProperty<int>("width")[0];
    int height = shapeElement.getProperty<int>("height")[0];
    if (depth.size() != width * height) {
        std::cerr << "ReadDepthImageFromPLYStream: Depth data does not match size of image" << std::endl;
        return false;
    }

    depthOut.reset(width, height, depth);
    
    return true;
}


bool WriteDepthImageToPLYFile(std::string filename, const DepthImage& depthImage)
{
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    
    if (!file.good()) {
        file.close();
        return false;
    }
    
    bool status = WriteDepthImageToPLYStream(file, depthImage);
    
    file.close();
    
    return status;
}

bool WriteDepthImageToPLYStream(std::ostream& output, const DepthImage& depthImage)
{
    happly::PLYData plyOut;
    
    plyOut.addElement("shape", 1);
    plyOut.addElement("depth", depthImage.getData().size());

    plyOut.getElement("shape").addProperty<int>("width", {depthImage.getWidth()});
    plyOut.getElement("shape").addProperty<int>("height", {depthImage.getHeight()});
    plyOut.getElement("depth").addProperty<float>("value", depthImage.getData());

    plyOut.write(output, happly::DataFormat::Binary);
    
    return true;
}

} // namespace ply
} // namespace io
} // namespace standard_cyborg
