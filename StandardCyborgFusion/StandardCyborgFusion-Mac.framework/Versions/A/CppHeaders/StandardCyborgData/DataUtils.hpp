//
//  DataUtils.hpp
//  StandardCyborgGeometry
//
//  Created by Ricky Reusser on 4/1/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <map>
#include <vector>

#include <StandardCyborgData/IncludeEigen.hpp>
#include <StandardCyborgData/VertexSelection.hpp>

namespace StandardCyborg {

struct Vec3;
struct Mat3x3;
struct Mat3x4;
struct Mat4x4;
struct Face3;

/*
 * Map data to either a 3Xf or X3f Eigen array. For use in particular when interfacing with
 * libigl, which requires X3f. You may construct a no-copy matrix view as:
 *
 *   auto matrixView { toMatrixX3f(geometry.getPositions()) }
 *
 * (Of course you may spell out the full type, but that is rather burdensome.)
 */
Eigen::Ref<Eigen::Matrix<float, 3, Eigen::Dynamic, Eigen::ColMajor>> toMatrix3Xf(std::vector<Vec3>& data);
Eigen::Ref<Eigen::Matrix<float, Eigen::Dynamic, 3, Eigen::RowMajor>> toMatrixX3f(std::vector<Vec3>& data);

/* Const versions of the above */
const Eigen::Ref<const Eigen::Matrix<float, 3, Eigen::Dynamic, Eigen::ColMajor>> toMatrix3Xf(const std::vector<Vec3>& data);
const Eigen::Ref<const Eigen::Matrix<float, Eigen::Dynamic, 3, Eigen::RowMajor>> toMatrixX3f(const std::vector<Vec3>& data);

/* Non-const Face3 adaptors */
Eigen::Ref<Eigen::Matrix<int, 3, Eigen::Dynamic, Eigen::ColMajor>> toMatrix3Xi(std::vector<Face3>& data);
Eigen::Ref<Eigen::Matrix<int, Eigen::Dynamic, 3, Eigen::RowMajor>> toMatrixX3i(std::vector<Face3>& data);

/* Const versions of the above */
const Eigen::Ref<const Eigen::Matrix<int, 3, Eigen::Dynamic, Eigen::ColMajor>> toMatrix3Xi(const std::vector<Face3>& data);
const Eigen::Ref<const Eigen::Matrix<int, Eigen::Dynamic, 3, Eigen::RowMajor>> toMatrixX3i(const std::vector<Face3>& data);

Eigen::Vector3f toVector3f(Vec3 data);
Eigen::Matrix3f toMatrix3f(Mat3x3 matrix);
Eigen::Matrix4f toMatrix4f(Mat3x4 matrix);
Eigen::Matrix4f toMatrix4f(Mat4x4 matrix);

Vec3 toVec3(Eigen::Vector3f vector);
Mat3x3 toMat3x3(Eigen::Matrix3f matrix);
Mat3x4 toMat3x4(Eigen::Matrix4f matrix);
Mat4x4 toMat4x4(Eigen::Matrix4f matrix);

Eigen::Matrix3f columnMajorToMatrix3f(const std::vector<float>& vector);
Eigen::Matrix4f columnMajorToMatrix4f(const std::vector<float>& vector);

template <class T>
void deleteEntriesFromVector(std::vector<T>& dataArray, const VertexSelection& indicesToDelete)
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
    VertexSelection::const_iterator deletionIterator = indicesToDelete.begin();
    
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

} // namespace StandardCyborg
