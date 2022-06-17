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

#include <map>
#include <vector>

#include "standard_cyborg/util/IncludeEigen.hpp"
#include "standard_cyborg/sc3d/VertexSelection.hpp"

namespace standard_cyborg {

namespace math {
struct Vec3;
struct Mat3x3;
struct Mat3x4;
struct Mat4x4;
}

namespace sc3d {
struct Face3;
}

/*
 * Map data to either a 3Xf or X3f Eigen array. For use in particular when interfacing with
 * libigl, which requires X3f. You may construct a no-copy matrix view as:
 *
 *   auto matrixView { toMatrixX3f(geometry.getPositions()) }
 *
 * (Of course you may spell out the full type, but that is rather burdensome.)
 */
Eigen::Ref<Eigen::Matrix<float, 3, Eigen::Dynamic, Eigen::ColMajor>> toMatrix3Xf(std::vector<math::Vec3>& data);
Eigen::Ref<Eigen::Matrix<float, Eigen::Dynamic, 3, Eigen::RowMajor>> toMatrixX3f(std::vector<math::Vec3>& data);

/* Const versions of the above */
const Eigen::Ref<const Eigen::Matrix<float, 3, Eigen::Dynamic, Eigen::ColMajor>> toMatrix3Xf(const std::vector<math::Vec3>& data);
const Eigen::Ref<const Eigen::Matrix<float, Eigen::Dynamic, 3, Eigen::RowMajor>> toMatrixX3f(const std::vector<math::Vec3>& data);

/* Non-const Face3 adaptors */
Eigen::Ref<Eigen::Matrix<int, 3, Eigen::Dynamic, Eigen::ColMajor>> toMatrix3Xi(std::vector<sc3d::Face3>& data);
Eigen::Ref<Eigen::Matrix<int, Eigen::Dynamic, 3, Eigen::RowMajor>> toMatrixX3i(std::vector<sc3d::Face3>& data);

/* Const versions of the above */
const Eigen::Ref<const Eigen::Matrix<int, 3, Eigen::Dynamic, Eigen::ColMajor>> toMatrix3Xi(const std::vector<sc3d::Face3>& data);
const Eigen::Ref<const Eigen::Matrix<int, Eigen::Dynamic, 3, Eigen::RowMajor>> toMatrixX3i(const std::vector<sc3d::Face3>& data);

Eigen::Vector3f toVector3f(math::Vec3 data);
Eigen::Matrix3f toMatrix3f(math::Mat3x3 matrix);
Eigen::Matrix4f toMatrix4f(math::Mat3x4 matrix);
Eigen::Matrix4f toMatrix4f(math::Mat4x4 matrix);

math::Vec3 toVec3(Eigen::Vector3f vector);
math::Mat3x3 toMat3x3(Eigen::Matrix3f matrix);
math::Mat3x4 toMat3x4(Eigen::Matrix4f matrix);
math::Mat4x4 toMat4x4(Eigen::Matrix4f matrix);

Eigen::Matrix3f columnMajorToMatrix3f(const std::vector<float>& vector);
Eigen::Matrix4f columnMajorToMatrix4f(const std::vector<float>& vector);

template <class T>
void deleteEntriesFromVector(std::vector<T>& dataArray, const sc3d::VertexSelection& indicesToDelete)
{
    // We iterate through the (ordered) set of *vertices to delete* and renumber
    // everything between since it's faster than iterating through *geometry vertices*
    // and using `count()` to check whether every single vertex ought to be deleted
    // or not.
    
    // Initialize the state
    int previousDeleted = -1;
    int renumberedPosition = 0;
    int dataSize = (int)dataArray.size();
    
    // Iterate through deleted vertices
    sc3d::VertexSelection::const_iterator deletionIterator = indicesToDelete.begin();
    
    while (deletionIterator != indicesToDelete.end()) {
        if (*deletionIterator > dataSize) break;
        
        // Loop *between* deleted vertices, from one past the previousDeleted up to the next
        for (int i = previousDeleted + 1; i < *deletionIterator; i++) {
            dataArray[renumberedPosition] = dataArray[i];
            renumberedPosition++;
            
            if (renumberedPosition >= dataSize) break;
        }
        
        previousDeleted = *deletionIterator;
        deletionIterator++;
    }
    
    // Renumber the final entries after the last deleted index
    for (int i = previousDeleted + 1; i < dataArray.size(); i++) {
        dataArray[renumberedPosition++] = dataArray[i];
    }
    
    dataArray.resize(renumberedPosition);
}

} // namespace standard_cyborg
