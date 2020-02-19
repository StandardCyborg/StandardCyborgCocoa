//
//  ImageFileIO.hpp
//  StandardCyborgIO
//
//  Created by Ricky Reusser on 8/26/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <istream>
#include <ostream>

namespace StandardCyborg {

class ColorImage;
    
enum class ImageFormat {
    PNG,
    JPEG
};

/** Read a stream of image data into an image instance */
extern bool ReadColorImageFromStream(ColorImage& imageOut, std::istream& inStream);

/** Read from a file into an image instance */
extern bool ReadColorImageFromFile(ColorImage& imageOut, std::string filename);

/** Serialize an image to an output stream */
extern bool WriteColorImageToStream(std::ostream& outStream, const ColorImage& image, ImageFormat format = ImageFormat::PNG, int jpegQuality = 90);

/** Write an image to a file */
extern bool WriteColorImageToFile(std::string filename, const ColorImage& image, ImageFormat format = ImageFormat::PNG, int jpegQuality = 90);

} // namespace StandardCyborg
