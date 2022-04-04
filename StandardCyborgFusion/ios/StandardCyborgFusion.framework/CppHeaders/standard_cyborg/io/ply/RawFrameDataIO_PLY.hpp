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

namespace standard_cyborg {

namespace sc3d {
class ColorImage;
class DepthImage;
class PerspectiveCamera;
}

namespace io {
namespace ply {

struct RawFrameMetadata {
    double timestamp;
};

/** Read a PLY stream containing raw frame data into an image, depth frame, and camera */
extern bool ReadRawFrameDataFromPLYStream(sc3d::ColorImage& imageOut, sc3d::DepthImage& depthOut, sc3d::PerspectiveCamera& cameraOut, std::istream& inStream);

/** Read a PLY stream containing raw frame data into an image, depth frame, camera, and timestamp */
extern bool ReadRawFrameDataFromPLYStream(sc3d::ColorImage& imageOut, sc3d::DepthImage& depthOut, sc3d::PerspectiveCamera& cameraOut, RawFrameMetadata& rawFrameMetadataOut, std::istream& inStream);

/** Read a PLY file containing raw frame data into an image, depth frame, and camera */
extern bool ReadRawFrameDataFromPLYFile(sc3d::ColorImage& imageOut, sc3d::DepthImage& depthOut, sc3d::PerspectiveCamera& cameraOut, std::string filename);

/** Read a PLY file containing raw frame data into an image, depth frame, camera, and timestamp */
extern bool ReadRawFrameDataFromPLYFile(sc3d::ColorImage& imageOut, sc3d::DepthImage& depthOut, sc3d::PerspectiveCamera& cameraOut, RawFrameMetadata& rawFrameMetadataOut, std::string filename);

} // namespace ply
} // namespace io
} // namespace standard_cyborg
