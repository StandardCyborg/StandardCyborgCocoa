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

namespace standard_cyborg {

namespace sc3d {
class ColorImage;
}

namespace io {
namespace imgfile {
    
enum class ImageFormat {
    PNG,
    JPEG
};

/** Read a stream of image data into an image instance */
extern bool ReadColorImageFromStream(sc3d::ColorImage& imageOut, std::istream& inStream);

/** Read from a file into an image instance */
extern bool ReadColorImageFromFile(sc3d::ColorImage& imageOut, std::string filename);

/** Read from a buffer into an image instance */
extern bool ReadColorImageFromBuffer(sc3d::ColorImage& imageOut, const uint8_t *buf, size_t len);

/** Serialize an image to an output stream */
extern bool WriteColorImageToStream(std::ostream& outStream, const sc3d::ColorImage& image, ImageFormat format = ImageFormat::PNG, int jpegQuality = 90);

/** Write an image to a file */
extern bool WriteColorImageToFile(std::string filename, const sc3d::ColorImage& image, ImageFormat format = ImageFormat::PNG, int jpegQuality = 90);

/** Write an image to a buffer */
extern bool WriteColorImageToBuffer(std::string &buf, const sc3d::ColorImage& image, ImageFormat format = ImageFormat::PNG, int jpegQuality = 90);

} // namespace imgfile
} // namespace io
} // namespace standard_cyborg
