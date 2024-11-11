//
//  MeshUvMap.hpp
//  StandardCyborgAlgorithms
//
//  Created by eric on 2019-08-13.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <memory>
#include <set>
#include <vector>

namespace standard_cyborg {

namespace sc3d {
class Geometry;
}

namespace math {
struct Vec2;
}

namespace algorithms {


/*
     UV unwrap a mesh, and put the resulting UV-unwrapping into the
     texture coordintes of the mesh.
     
     Note that the algorithm will cut the mesh into pieces, into charts. Otherwise,
     it would not be possible to unwrap objects such as sphere.
     
     Returns false if UV-unwrapping failed. Call getErrorMessage() to get an error message.
     */
bool uvmapMesh(standard_cyborg::sc3d::Geometry& geo);

/*
     Returns an error message, for the last failed unwrapping.
     */
std::string getUvmapMeshErrorMessage();

// utility functions used when implementing `uvmapMesh`.
// Only exposed to make testing `uvmapMesh` easier.
// thus, these are not officially part of the API. Only the above function is part of the API.
bool seamCutMesh(standard_cyborg::sc3d::Geometry& geometry, const std::vector<int>& seamCut);
std::vector<std::shared_ptr<standard_cyborg::sc3d::Geometry>> seamCutMeshBySegmentation(standard_cyborg::sc3d::Geometry& geometry,
                                                                 std::vector<std::set<int>> segmentation);
std::vector<math::Vec2> parameterize(const standard_cyborg::sc3d::Geometry& geometry);


bool removeDegenerateTriangles(standard_cyborg::sc3d::Geometry& geometry);

void removeUnusedVertices(standard_cyborg::sc3d::Geometry& geometry);
std::vector<std::tuple<float, float, float, float, bool>> packCharts(
    const std::vector<std::pair<float, float>>& chartSizes,
    float& textureSize);
std::unique_ptr<standard_cyborg::sc3d::Geometry> combineGeometries(const std::vector<std::shared_ptr<standard_cyborg::sc3d::Geometry>>& geometries);

}

}
