//
//  SceneGraphFileIO_GLTF.hpp
//  StandardCyborgIO
//
//  Created by eric on 2019-06-25.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace StandardCyborg {

class Node;

/** Read from a string formatted as gltf-file. */
std::vector<std::shared_ptr<Node>> ReadSceneGraphFromGltf(const std::string& gltfSource);

/** Write a Scene graph to a path. */
bool WriteSceneGraphToGltf(std::vector<std::shared_ptr<Node>> sceneGraph, const std::string& path);
bool WriteSceneGraphToGltf(std::shared_ptr<Node> sceneGraph, const std::string& path);
bool WriteSceneGraphToGltf(Node* sceneGraph, const std::string& path);

} // namespace StandardCyborg
