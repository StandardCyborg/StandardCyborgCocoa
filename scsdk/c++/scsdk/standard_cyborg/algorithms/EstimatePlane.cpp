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

#include "standard_cyborg/algorithms/EstimatePlane.hpp"

#include <cmath>
#include <cstdlib>

#include <map>
#include <queue>

#include "standard_cyborg/util/DataUtils.hpp"
#include "standard_cyborg/math/Vec3.hpp"
#include "standard_cyborg/sc3d/VertexSelection.hpp"

using standard_cyborg::sc3d::Face3;
using standard_cyborg::sc3d::Plane;
using standard_cyborg::math::Vec3;
using standard_cyborg::sc3d::VertexSelection;

namespace standard_cyborg {

namespace algorithms {

EstimatePlaneResult estimatePlane(const std::vector<Vec3>& positions,
                                  const VertexSelection& initialVertexSet,
                                  int maxIterations,
                                  float outlierStandardDeviationThreshold,
                                  float relativeConvergenceTolerance,
                                  float absoluteConvergenceTolerance)
{
    int vertexCount = static_cast<int>(positions.size());
    
    EstimatePlaneResult result;

    std::unique_ptr<VertexSelection> currentVertexSet = std::make_unique<VertexSelection>();
    currentVertexSet->copy(initialVertexSet);

    // If an initial set is provided this is unnecessary, but we'll elect to make seeding the RNG
    // not conditional.
    std::srand(0);
    
    if (currentVertexSet->size() == 0) {
        // If no initial set of vertices is provided, seed it with 1/3 of the vertices, randomly selected.
        // In general this does not work very well, so it's highly advised to use gravity or some sort of
        // heuristic to make an educated initial guess
        for (int i = 0; i < vertexCount; i++) {
            if (std::rand() % 3 == 1) {
                currentVertexSet->insertValue(i);
            }
        }
    }
    
    Plane bestFit;
    
    float previousRMS = 1e-10;
    float rmsProjectedDistance = 1e10;


    auto positionsMatrix(toMatrix3Xf(positions));
    Eigen::Matrix3Xf debiasedPositions = Eigen::Matrix3Xf(3, vertexCount);

    Eigen::Vector3f normal;
    Eigen::Vector3f centroid;
    
    // This algorithm works by computing the singular vector corresponding to the smallest
    // singular value of a set of vertices. The first two singular vectors lie in the desired
    // plane, so the third singular vector is by definition then normal to the estimated plane.
    //
    // After each plane estimation, the RMS projected distance for all inlier points is computed.
    // All points of the original point cloud are compared against this value, new inlier/outliers
    // are selected, and we return to fitting.

    for (int iteration = 0; iteration < maxIterations; iteration++) {
        // Extract the current inlier subset into a Matrix3Xf.
        int subsetSize = (int)currentVertexSet->size();
        Eigen::Matrix3Xf subsetPositions(3, subsetSize);
        int j = 0;
        
        for (auto index : *currentVertexSet) {
            subsetPositions.col(j++) = positionsMatrix.col(index);
        }
        
        // Compute the centroid of the inlier subset
        centroid = subsetPositions.rowwise().mean();

        // Subtract off the centroid from the inlier subset
        subsetPositions.colwise() -= centroid;
        
        // Compute the plane
        Eigen::BDCSVD<Eigen::Matrix3Xf> svd = subsetPositions.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV);
        
        // The normal is the smallest left singular vector
        normal = svd.matrixU().col(2);
        
        // To determine convergence, the remaining section below computes the out-of-plane projected distance
        // for each vertex and from that computes the aggregate RMS out-of-plane distance (roughly speaking,
        // the standard deviation). It examines the values for convergence, otherwise it iterates back over all
        // points of the original point cloud and tags anything within, say five standard deviations as being an
        // inlier. That becomes the new set of vertices to which it fits a plane in the next iteration.
        
        // Subtract the current centroid from the subset positions and compute the RMS projected distance.
        // This could be optimized by looping and adding without storing the intermediate result.
        rmsProjectedDistance = (normal.transpose() * subsetPositions).norm() / std::sqrt(subsetPositions.size());

        Eigen::VectorXf projectedDistanceFactor = (normal.transpose() * (positionsMatrix.colwise() - centroid) / rmsProjectedDistance).cwiseAbs();

        bestFit.position = standard_cyborg::toVec3(centroid);
        bestFit.normal = standard_cyborg::toVec3(normal);

        // Exit early if the RMS distance is very low, though this is unlikely to happen on any realistic data
        if (rmsProjectedDistance < absoluteConvergenceTolerance) break;
        
        // Examine the relative RMS distance from frame to frame as our overall convergence criteria
        float relativeRMSDistance = std::abs(rmsProjectedDistance - previousRMS) / std::max(rmsProjectedDistance, previousRMS);
        if (relativeRMSDistance < relativeConvergenceTolerance) break;

        // Recompute the subset indices
        currentVertexSet->clear();
        for (int j = 0; j < vertexCount; j++) {
            if (projectedDistanceFactor[j] < outlierStandardDeviationThreshold) currentVertexSet->insertValue(j);
        }
        
        previousRMS = rmsProjectedDistance;
    }
    
    result.rmsProjectedDistance = rmsProjectedDistance;
    result.planeVertices->copy(*currentVertexSet);
    result.plane.position = standard_cyborg::toVec3(centroid);
    result.plane.normal = standard_cyborg::toVec3(normal);
    result.converged = true;
    
    return result;
}

}

} // namespace StandardCyborg
