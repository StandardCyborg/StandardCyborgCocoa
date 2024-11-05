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

#include "standard_cyborg/io/json/PerspectiveCameraFileIO_JSON.hpp"
#include "standard_cyborg/io/json/PerspectiveCameraFileIO_JSON_Private.hpp"
#include "standard_cyborg/sc3d/PerspectiveCamera.hpp"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>

#include "standard_cyborg/math/Mat3x3.hpp"
#include "standard_cyborg/math/Mat3x4.hpp"

namespace standard_cyborg {
namespace io {
namespace json {

using JSON = nlohmann::json;

bool WritePerspectiveCameraToJSONFile(std::string filename, const sc3d::PerspectiveCamera& camera)
{
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    
    if (!file.good()) {
        file.close();
        return false;
    }
    
    bool status = WritePerspectiveCameraToJSONStream(file, camera);
    
    file.close();
    
    return status;
}

bool ReadPerspectiveCameraFromJSONFile(sc3d::PerspectiveCamera& camera, std::string filename)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    
    if (!file.good()) {
        file.close();
        return false;
    }
    
    bool status = ReadPerspectiveCameraFromJSONStream(camera, file);
    
    file.close();
    
    return status;
}

bool ReadPerspectiveCameraFromJSON(sc3d::PerspectiveCamera& camera, const JSON& json) {
    
    using math::Mat3x3;
    using math::Mat3x4;
    using math::Mat4x4;    
    using math::Vec2;

    // Set the focal length scale factor
    if (json.contains("focalLengthScaleFactor")) {
        camera.setFocalLengthScaleFactor(json["focalLengthScaleFactor"]);
    } else if (json.contains("focal_length_scale_factor")) {
        // Handle legacy BPLY files without immediately having to rewrite everything
        camera.setFocalLengthScaleFactor(json["focal_length_scale_factor"]);
    }
    
    // Set the intrinsic matrix
    if (json.contains("nominalIntrinsicMatrix")) {
        camera.setNominalIntrinsicMatrix(Mat3x3::fromColumnMajorVector(json["nominalIntrinsicMatrix"]));
    } else if (json.contains("intrinsic_matrix")) {
        // Handle legacy BPLY files without immediately having to rewrite everything
        camera.setNominalIntrinsicMatrix(Mat3x3::fromColumnMajorVector(json["intrinsic_matrix"]));
    } else if (json.contains("uncalibratedIntrinsicMatrix")) {
        // Handle legacy GLTF files without requiring a rewrite
        camera.setNominalIntrinsicMatrix(Mat3x3::fromColumnMajorVector(json["uncalibratedIntrinsicMatrix"]));
    }
    
    // Set the orientation matrix, if present
    if (json.contains("orientationMatrix")) {
        camera.setOrientationMatrix(Mat3x4::fromColumnMajorVector(json["orientationMatrix"]));
    } else if (json.contains("orientationTransform")) {
        // Handle legacy GLTF files without requiring a rewrite
        camera.setOrientationMatrix(Mat3x4::fromColumnMajorVector(json["orientationTransform"]));
    } else {
        camera.setOrientationMatrix(Mat3x4(0, 1, 0, 0,
                                           1, 0, 0, 0,
                                           0, 0, -1, 0));
    }
    
    if (json.contains("extrinsicMatrix")) {
        camera.setExtrinsicMatrix(Mat3x4::fromColumnMajorVector(json["extrinsicMatrix"]));
    } else if (json.contains("extrinsic_matrix")) {
        // Handle legacy BPLY files without immediately having to rewrite everything
        camera.setExtrinsicMatrix(Mat3x4(Mat4x4::fromColumnMajorVector(json["extrinsic_matrix"])));
    }
    
    if (json.contains("intrinsicMatrixReferenceSize")) {
        camera.setIntrinsicMatrixReferenceSize(Vec2{
            json["intrinsicMatrixReferenceSize"][0],
            json["intrinsicMatrixReferenceSize"][1]
        });
    } else if (json.contains("intrinsic_matrix_reference_dimensions_width") && json.contains("intrinsic_matrix_reference_dimensions_width")) {
        // Handle legacy BPLY files without immediately having to rewrite everything
        camera.setIntrinsicMatrixReferenceSize(Vec2{
            json["intrinsic_matrix_reference_dimensions_width"],
            json["intrinsic_matrix_reference_dimensions_height"]
        });
    }
    
    if (json.contains("lensDistortionLookupTable")) {
        camera.setLensDistortionLookupTable(json["lensDistortionLookupTable"]);
    } else if (json.contains("lens_distortion_lookup_table")) {
        // Handle legacy BPLY files without immediately having to rewrite everything
        camera.setLensDistortionLookupTable(json["lens_distortion_lookup_table"]);
    }
    
    if (json.contains("lensDistortionLookupTable")) {
        camera.setInverseLensDistortionLookupTable(json["inverseLensDistortionLookupTable"]);
    } else if (json.contains("lens_distortion_lookup_table")) {
        // Handle legacy BPLY files without immediately having to rewrite everything
        camera.setInverseLensDistortionLookupTable(json["inverse_lens_distortion_lookup_table"]);
    }
    
    if (json.contains("width") && json.contains("height")) {
        camera.setLegacyImageSize({json["width"], json["height"]});
    }
    
    return true;
}

bool ReadPerspectiveCameraFromJSONStream(sc3d::PerspectiveCamera& camera, std::istream& input)
{
    JSON json;
    input >> json;
    
    // Sanity check
    if (json["version"] != "1.0.0") { // unsupported version.
        std::cerr << "ReadJSONIntoPerspectiveCamera: unsupported version" << std::endl;
        return false;
    }
    
    return ReadPerspectiveCameraFromJSON(camera, json["data"]);
}

bool WritePerspectiveCameraToJSON(JSON& json, const sc3d::PerspectiveCamera& camera)
{
    json["extrinsicMatrix"] = camera.getExtrinsicMatrix().toColumnMajorVector();
    json["intrinsicMatrix"] = camera.getIntrinsicMatrix().toColumnMajorVector();
    json["orientationMatrix"] = camera.getOrientationMatrix().toColumnMajorVector();
    json["nominalIntrinsicMatrix"] = camera.getNominalIntrinsicMatrix().toColumnMajorVector();
    json["focalLengthScaleFactor"] = camera.getFocalLengthScaleFactor();
    json["intrinsicMatrixReferenceSize"] = std::vector<float>{
        camera.getIntrinsicMatrixReferenceSize().x,
        camera.getIntrinsicMatrixReferenceSize().y
    };
    
    return true;
}

bool WritePerspectiveCameraToJSONStream(std::ostream& output, const sc3d::PerspectiveCamera& camera)
{
    JSON json;
    
    WritePerspectiveCameraToJSON(json["data"], camera);

    json["version"] = "1.0.0";

    output << std::setw(5) << json;
    
    return true;
}

} // namespace json
} // namespace io
} // namespace standard_cyborg
