//
//  PointCloudIO_JSON.cpp
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 3/11/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#include <nlohmann/json.hpp>
#include <StandardCyborgFusion/PointCloudIO.hpp>
#include <standard_cyborg/sc3d/PerspectiveCamera.hpp>
#include <standard_cyborg/io/json/PerspectiveCameraFileIO_JSON_Private.hpp>

using JSON = nlohmann::json;
using namespace standard_cyborg;

sc3d::PerspectiveCamera PointCloudIO::PerspectiveCameraFromJSON(const JSON& json)
{
    sc3d::PerspectiveCamera camera;
    io::json::ReadPerspectiveCameraFromJSON(camera, json);
    return camera;
}

JSON PointCloudIO::JSONFromPerspectiveCamera(const sc3d::PerspectiveCamera& camera)
{
    JSON json;
    io::json::WritePerspectiveCameraToJSON(json, camera);
    return json;
}

