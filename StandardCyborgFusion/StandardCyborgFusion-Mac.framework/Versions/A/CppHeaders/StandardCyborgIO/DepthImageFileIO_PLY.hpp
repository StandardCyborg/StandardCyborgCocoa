//
//  DepthImageFileIO_PLY.hpp
//  StandardCyborgIO
//
//  Created by Ricky Reusser on 10/16/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <istream>
#include <ostream>

namespace StandardCyborg {

class DepthImage;
    
/** Read a stream of PLY data into an image instance */
extern bool ReadDepthImageFromPLYStream(DepthImage& imageOut, std::istream& inStream);

/** Read from a PLY file into an image instance */
extern bool ReadDepthImageFromPLYFile(DepthImage& imageOut, std::string filename);

/** Serialize an image to an output stream of PLY data */
extern bool WriteDepthImageToPLYStream(std::ostream& outStream, const DepthImage& image);

/** Write an image to a PLY file */
extern bool WriteDepthImageToPLYFile(std::string filename, const DepthImage& image);

} // namespace StandardCyborg

