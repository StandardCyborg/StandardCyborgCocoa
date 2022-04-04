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

#include <optional>

#include "standard_cyborg/util/Result.hpp"

#include "standard_cyborg/sc3d/ColorImage.hpp"
#include "standard_cyborg/sc3d/DepthImage.hpp"
#include "standard_cyborg/proto/sc3d/image.pb.h"

namespace standard_cyborg {
namespace io {
namespace pbio {

struct ParsedImage {
    std::optional<sc3d::ColorImage> color_image;
    std::optional<sc3d::DepthImage> depth_image;
};

extern Result<ParsedImage> FromPB(const ::standard_cyborg::proto::sc3d::Image &msg);

enum class PBColorImageFormat {
    UNCOMPRESSED = 0, // Protobuf floats, NUMERIC_TYPE_FLOAT
    // TODO(ricky) UNCOMPRESSED_NUMERIC_TYPE_FLOAT_IEEE754_LITTLE_ENDIAN_BYTES = 1,
    PNG = 2,
    JPEG = 3,
};

extern Result<::standard_cyborg::proto::sc3d::Image> ToPB(
                                                          const sc3d::ColorImage &img,
                                                          PBColorImageFormat format=PBColorImageFormat::UNCOMPRESSED);


enum class PBDepthImageFormat {
    UNCOMPRESSED = 0, // Protobuf floats, NUMERIC_TYPE_FLOAT
    // TODO(ricky) UNCOMPRESSED_NUMERIC_TYPE_FLOAT_IEEE754_LITTLE_ENDIAN_BYTES = 1,
};

extern Result<::standard_cyborg::proto::sc3d::Image> ToPB(
                                                          const sc3d::DepthImage &img,
                                                          PBDepthImageFormat format=PBDepthImageFormat::UNCOMPRESSED);


// Create an `Image` message compatible with the encoding we use for
// `ColorImage`, except in this case the user provides the image as a
// jpeg byte buffer (which the returned `Image` will own).  Users should
// prefer this API when they have a platform-optimized JPEG compressor
// available.
extern Result<::standard_cyborg::proto::sc3d::Image> ToPBFromJpegBytes(
                                                                       const size_t image_width,
                                                                       const size_t image_height,
                                                                       const std::string &image_frame,
                                                                       std::string &&jpeg_bytes);


//
// ParsedImage support; mainly for testing
//

extern inline Result<::standard_cyborg::proto::sc3d::Image> ToPB(
                                                                 const ParsedImage &pi) {
    
    if (pi.color_image.has_value()) {
        return ToPB(pi.color_image.value());
    } else if (pi.depth_image.has_value()) {
        return ToPB(pi.depth_image.value());
    } else {
        return {.error = "Empty ParsedImage"};
    }
}

extern bool operator==(const ParsedImage &lhs, const ParsedImage &rhs);

} // namespace pbio
} // namespace io
} // namespace standard_cyborg

extern std::ostream& operator<<(std::ostream& os, const standard_cyborg::io::pbio::ParsedImage& pi);
