//
//  DataUtils.hpp
//  StandardCyborgGeometry
//
//  Created by Ricky Reusser on 4/1/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <vector>

#include <StandardCyborgData/Face3.hpp>
#include <StandardCyborgData/IncludeEigen.hpp>
#include <StandardCyborgData/Vec3.hpp>
#include <StandardCyborgData/VertexSelection.hpp>


namespace StandardCyborg {

/*
 * Map data to either a 3Xf or X3f Eigen array. For use in particular when interfacing with
 * libigl, which requires X3f. You may construct a no-copy matrix view as:
 *
 *   auto matrixView { Vec3ToEigenX3f(geometry.getPositions()) }
 *
 * (Of course you may spell out the full type, but that is rather burdensome.)
 */
Eigen::Ref<Eigen::Matrix<float, 3, Eigen::Dynamic, Eigen::ColMajor>> Vec3ToEigen3Xf(std::vector<Vec3>& data);
Eigen::Ref<Eigen::Matrix<float, Eigen::Dynamic, 3, Eigen::RowMajor>> Vec3ToEigenX3f(std::vector<Vec3>& data);

/* Const versions of the above */
const Eigen::Ref<const Eigen::Matrix<float, 3, Eigen::Dynamic, Eigen::ColMajor>> Vec3ToEigen3Xf(const std::vector<Vec3>& data);
const Eigen::Ref<const Eigen::Matrix<float, Eigen::Dynamic, 3, Eigen::RowMajor>> Vec3ToEigenX3f(const std::vector<Vec3>& data);

/* Non-const Face3 adaptors */
Eigen::Ref<Eigen::Matrix<int, 3, Eigen::Dynamic, Eigen::ColMajor>> Face3ToEigen3Xi(std::vector<Face3>& data);
Eigen::Ref<Eigen::Matrix<int, Eigen::Dynamic, 3, Eigen::RowMajor>> Face3ToEigenX3i(std::vector<Face3>& data);

/* Const versions of the above */
const Eigen::Ref<const Eigen::Matrix<int, 3, Eigen::Dynamic, Eigen::ColMajor>> Face3ToEigen3Xi(const std::vector<Face3>& data);
const Eigen::Ref<const Eigen::Matrix<int, Eigen::Dynamic, 3, Eigen::RowMajor>> Face3ToEigenX3i(const std::vector<Face3>& data);


Eigen::Vector3f Vec3ToEigen(Vec3 data);

Vec3 EigenToVec3(Eigen::Vector3f vector);

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
