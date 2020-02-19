//
//  DepthImageFileIO.hpp
//  StandardCyborgIO
//
//  Created by Ricky Reusser on 10/14/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <istream>
#include <ostream>

#include "ColorImageFileIO.hpp"

namespace StandardCyborg {

class DepthImage;
    
/** Read a stream of image data into an image instance */
extern bool ReadDepthImageFromStream(DepthImage& imageOut, std::istream& inStream);

/** Read from a file into an image instance */
extern bool ReadDepthImageFromFile(DepthImage& imageOut, std::string filename);

/** Serialize an image to an output stream */
extern bool WriteDepthImageToStream(std::ostream& outStream, const DepthImage& image, ImageFormat format = ImageFormat::PNG, int jpegQuality = 90);

/** Write an image to a file */
extern bool WriteDepthImageToFile(std::string filename, const DepthImage& image, ImageFormat format = ImageFormat::PNG, int jpegQuality = 90);

} // namespace StandardCyborg
