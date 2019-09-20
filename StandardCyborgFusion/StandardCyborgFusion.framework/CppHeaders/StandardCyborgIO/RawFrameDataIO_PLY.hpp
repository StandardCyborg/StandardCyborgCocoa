//
//  RawFrameDataIO_PLY.hpp
//  StandardCyborgIO
//
//  Created by Ricky Reusser on 8/20/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <istream>

namespace StandardCyborg {

class DepthImage;
class ColorImage;
class PerspectiveCamera;

/** Read a PLY stream containing raw frame data into an image, depth frame, and camera */
extern bool ReadRawFrameDataFromPLYStream(ColorImage& imageOut, DepthImage& depthOut, PerspectiveCamera& cameraOut, std::istream& inStream);

/** Read a PLY file containing raw frame data into an image, depth frame, and camera */
extern bool ReadRawFrameDataFromPLYFile(ColorImage& imageOut, DepthImage& depthOut, PerspectiveCamera& cameraOut, std::string filename);

} // namespace StandardCyborg
