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

#include <vector>
#include <memory>

/*
 Find all pieces of geometry,
 that are not connected to each other.
 
 for instance, if the input geometry looks like this:
 
 ------     ------
 1    1     1    1
 1    1     1    1
 1    1     1    1
 1    1     1    1
 ------     ------
 
 it will return the two squares as two separate meshes, since they are not connected to each other.
 
 */
namespace standard_cyborg {

namespace sc3d {
class Geometry;
}

namespace algorithms {

std::vector<std::shared_ptr<sc3d::Geometry>> splitMeshIntoPieces(const sc3d::Geometry& geometry);

}

} // namespace StandardCyborg
