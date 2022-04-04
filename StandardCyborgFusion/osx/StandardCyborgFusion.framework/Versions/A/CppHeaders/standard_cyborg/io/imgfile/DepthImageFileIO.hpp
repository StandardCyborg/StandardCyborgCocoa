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

#include "standard_cyborg/io/imgfile/ColorImageFileIO.hpp"

namespace standard_cyborg {

namespace sc3d {
class DepthImage;
}

namespace io {
namespace imgfile {

    
/** Read a stream of image data into an image instance */
[[deprecated("For debugging only; use Proto or PLY I/O for production use")]]
extern bool ReadDepthImageFromStream(sc3d::DepthImage& imageOut, std::istream& inStream);

/** Read from a file into an image instance */
[[deprecated("For debugging only; use Proto or PLY I/O for production use")]]
extern bool ReadDepthImageFromFile(sc3d::DepthImage& imageOut, std::string filename);

/** Serialize an image to an output stream */
[[deprecated("For debugging only; use Proto or PLY I/O for production use")]]
extern bool WriteDepthImageToStream(std::ostream& outStream, const sc3d::DepthImage& image, ImageFormat format = ImageFormat::PNG, int jpegQuality = 90);

/** Write an image to a file */
[[deprecated("For debugging only; use Proto or PLY I/O for production use")]]
extern bool WriteDepthImageToFile(std::string filename, const sc3d::DepthImage& image, ImageFormat format = ImageFormat::PNG, int jpegQuality = 90);

} // namespace imgfile
} // namespace io
} // namespace standard_cyborg
