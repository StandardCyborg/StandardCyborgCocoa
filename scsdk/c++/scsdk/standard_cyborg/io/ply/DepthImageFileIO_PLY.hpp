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
class DepthImage;
}

namespace io {
namespace ply {

/** Read a stream of PLY data into an image instance */
extern bool ReadDepthImageFromPLYStream(sc3d::DepthImage& imageOut, std::istream& inStream);

/** Read from a PLY file into an image instance */
extern bool ReadDepthImageFromPLYFile(sc3d::DepthImage& imageOut, std::string filename);

/** Serialize an image to an output stream of PLY data */
extern bool WriteDepthImageToPLYStream(std::ostream& outStream, const sc3d::DepthImage& image);

/** Write an image to a PLY file */
extern bool WriteDepthImageToPLYFile(std::string filename, const sc3d::DepthImage& image);

} // namespace ply
} // namespace io
} // namespace standard_cyborg

