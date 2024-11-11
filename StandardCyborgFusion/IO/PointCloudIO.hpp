//
//  PointCloudIO.hpp
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 7/12/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#pragma once

#include <string>

#include <nlohmann/json.hpp>

#include <standard_cyborg/util/IncludeEigen.hpp>
#include <StandardCyborgFusion/RawFrame.hpp>
#include <StandardCyborgFusion/Surfel.hpp>
#include <standard_cyborg/sc3d/PerspectiveCamera.hpp>

using JSON = nlohmann::json;
using namespace standard_cyborg;

extern const char *SCFrameworkVersion();

class PointCloudIO {
public:
    /** Assumes file has sRGB color space, and reads into linear color space */
    static bool ReadVerticesFromOBJFile(Eigen::Matrix3Xf& vertices,
                                        Eigen::Matrix3Xf& normals,
                                        Eigen::Matrix3Xf& colors,
                                        std::string filename);
    
    /** Assumes file has sRGB color space, and reads into linear color space */
    static bool ReadSurfelsFromOBJFile(Surfels& surfels,
                                       std::string filename);
    
    /** Assumes file has sRGB color space, and reads into linear color space */
    static bool ReadSurfelsFromPLYFile(Surfels& surfels,
                                       std::string filename,
                                       bool normalizeNormals = false);
    
    /** Assumes vertices are in linear color space, and writes to file in sRGB color space */
    static bool WriteVerticesToOBJFile(const Eigen::Matrix3Xf& vertices,
                                       const Eigen::Matrix3Xf& normals,
                                       const Eigen::Matrix3Xf& colors,
                                       std::string filename);
    
    /** Assumes surfels are in linear color space, and writes to file in sRGB color space */
    static bool WriteSurfelsToPLYFile(const Surfel* surfels,
                                      size_t surfelCount,
                                      Eigen::Vector3f gravity,
                                      std::string filename);
    
    /** Assumes surfels are in linear color space, and writes to file in sRGB color space */
    static bool WriteSurfelsToOBJFile(const Surfel* surfels,
                                      size_t surfelCount,
                                      std::string filename);
    
    static bool WriteSurfelsToUSDAFile(const Surfel *surfels,
                                       size_t surfelCount,
                                       std::string filename,
                                       std::string colorMapFilename);
    
    static sc3d::PerspectiveCamera PerspectiveCameraFromJSON(const JSON& json);
    
    static JSON JSONFromPerspectiveCamera(const sc3d::PerspectiveCamera& camera);

    static std::unique_ptr<RawFrame> ReadRawFrameFromBPLYFile(std::string filename);
    
    static void WriteRawFrameToBPLYFile(const RawFrame& rawFrame, std::string filename);
};
