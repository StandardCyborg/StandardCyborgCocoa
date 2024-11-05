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

#include "standard_cyborg/algorithms/PrincipalAxes.hpp"
#include "standard_cyborg/algorithms/Centroid.hpp"

#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/math/Mat3x4.hpp"

#include "standard_cyborg/util/DataUtils.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <Eigen/Eigenvalues>
#pragma clang diagnostic pop

using standard_cyborg::sc3d::Geometry;
using standard_cyborg::math::Vec3;
using standard_cyborg::math::Mat3x4;

using standard_cyborg::sc3d::Face3;

namespace standard_cyborg {

namespace algorithms {

Mat3x4 computeNormalwisePrincipalAxes(const Geometry& geometry)
{
    using namespace math;

    Eigen::Matrix3f Moment;
    Moment.setZero();
    
    Vec3 areaVector{0.0f, 0.0f, 0.0f};
    Vec3 centroid{0.0f, 0.0f, 0.0f};
    float areaSum = 0.0f;
    
    int numFaces = geometry.faceCount();
    const std::vector<Face3>& faces = geometry.getFaces();
    const std::vector<Vec3>& positions = geometry.getPositions();
    
    for (int i = 0; i < numFaces; i++) {
        const Face3& face = faces[i];
        const Vec3& pA = positions[face[0]];
        const Vec3& pB = positions[face[1]];
        const Vec3& pC = positions[face[2]];
        
        Vec3 faceCentroid = (pA + pB + pC) * (1.0f / 3.0f);
        
        // Vectors along two of the edges
        Vec3 vBA = pB - pA;
        Vec3 vCA = pC - pA;
        
        // Face area vector (strictly speaking, twice the area, but since we use it as
        // a weight and also divide by twice the area, it's strictly equivalent.
        Vec3 dArea = Vec3::cross(vBA, vCA);
        
        // Integrate the differential surface element area vectors into a summed area
        // vectors. For a perfectly closed volume, this will sum to zero and won't work
        // very well, but if part of the model is open, it will provide stable orientation
        // where eigenvalues+eigenvectors don't.
        areaVector += dArea;
        
        // Add this face's contributions to accumulators
        float dAreaMagnitude = dArea.norm();
        centroid += faceCentroid * dAreaMagnitude;
        areaSum += dAreaMagnitude;
        
        Moment(0, 0) += dArea.x * dArea.x;
        Moment(1, 0) += dArea.x * dArea.y;
        Moment(2, 0) += dArea.x * dArea.z;
        
        //Moment(0, 1) += dArea.y * dArea.x;
        Moment(1, 1) += dArea.y * dArea.y;
        Moment(2, 1) += dArea.y * dArea.z;
        
        //Moment(0, 2) += dArea.z * dArea.x;
        //Moment(1, 2) += dArea.z * dArea.y;
        Moment(2, 2) += dArea.z * dArea.z;
    }
    
    centroid /= areaSum;
    
    // The eigen docs state these entries are not used at all. We could copy them, but
    // it has no effect.
    // Moment(0, 1) = Moment(1, 0);
    // Moment(0, 2) = Moment(2, 0);
    // Moment(1, 2) = Moment(2, 1);
    
    // Recall that if a self-adjoint matrix is real, then it's also symmetric, and the
    // eigenvalues of a real, symmetric matrix are real and also orthogonal.
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> eigensolver(Moment);
    
    auto eigenvectors = eigensolver.eigenvectors();
    auto eigenvalues = eigensolver.eigenvalues();
    
    Vec3 axis0 = toVec3(eigenvectors.col(0));
    Vec3 axis1 = toVec3(eigenvectors.col(1));
    
    // Eigenvectors are unique up to a sign, so we use the integrated area vector to
    // select an orientation stable with respect to small peturtbations. The first two
    // eigenvectors correspond to the strongest eigenvalues, so we flip their parity.
    // The third eigenvalue encodes the least amount of information, so we overwrite it
    // to enforce the correct parity.
    if (Vec3::dot(areaVector, axis0) < 0.0f) axis0 = -axis0;
    if (Vec3::dot(areaVector, axis1) < 0.0f) axis1 = -axis1;
    Vec3 axis2{Vec3::cross(axis0, axis1)};
    
    // Return the transform which moves a model at the origin *into* alignment with this
    // model. To align this model upright, you must invert this transform.
    return Mat3x4{
        axis0.x, axis1.x, axis2.x, centroid.x,
        axis0.y, axis1.y, axis2.y, centroid.y,
        axis0.z, axis1.z, axis2.z, centroid.z
    };
}

Mat3x4 computePointwisePrincipalAxes(const Geometry& geometry)
{
    using namespace math;

    Eigen::Matrix3f Moment;
    Moment.setZero();
    
    Vec3 areaVector{0.0f, 0.0f, 0.0f};
    Vec3 centroid(computeCentroid(geometry));

    int numFaces = geometry.faceCount();
    const std::vector<Face3>& faces = geometry.getFaces();
    const std::vector<Vec3>& positions = geometry.getPositions();
    
    
    for (int i = 0; i < numFaces; i++) {
        const Face3 face = faces[i];
        const Vec3 pA = positions[face[0]];
        const Vec3 pB = positions[face[1]];
        const Vec3 pC = positions[face[2]];

        Vec3 offset = (pA + pB + pC) * (1.0f / 3.0f) - centroid;

        // Face area vector (strictly speaking, twice the area, but since we use it as
        // a weight and also divide by twice the area, it's strictly equivalent.
        Vec3 dArea = 0.5 * Vec3::cross(pB - pA, pC - pA);

        // Integrate the differential surface element area vectors into a summed area
        // vectors. For a perfectly closed volume, this will sum to zero and won't work
        // very well, but if part of the model is open, it will provide stable orientation
        // where eigenvalues+eigenvectors don't.
        areaVector += dArea;

        // Weight each face by its area
        float weight = dArea.norm();
        
        Moment(0, 0) += offset.x * offset.x * weight;
        Moment(1, 0) += offset.x * offset.y * weight;
        Moment(2, 0) += offset.x * offset.z * weight;
        
        //Moment(0, 1) += offset.y * offset.x * weight;
        Moment(1, 1) += offset.y * offset.y * weight;
        Moment(2, 1) += offset.y * offset.z * weight;
        
        //Moment(0, 2) += offset.z * offset.x * weight;
        //Moment(1, 2) += offset.z * offset.y * weight;
        Moment(2, 2) += offset.z * offset.z * weight;
    }
    
    // Recall that if a self-adjoint matrix is real, then it's also symmetric, and the
    // eigenvalues of a real, symmetric matrix are real and also orthogonal.
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> eigensolver(Moment);
    
    auto eigenvectors = eigensolver.eigenvectors();
    auto eigenvalues = eigensolver.eigenvalues();
    
    // Eigenvalues are sorted in *increasing* order, so the strongest axis is the last
    Vec3 axis0 = toVec3(eigenvectors.col(2));
    Vec3 axis1 = toVec3(eigenvectors.col(1));
    
    // Eigenvectors are unique up to a sign, so we use the integrated area vector to
    // select an orientation stable with respect to small peturtbations. The first two
    // eigenvectors correspond to the strongest eigenvalues, so we flip their parity.
    // The third eigenvalue encodes the least amount of information, so we overwrite it
    // to enforce the correct parity.
    if (Vec3::dot(areaVector, axis0) < 0.0f) axis0 = -axis0;
    if (Vec3::dot(areaVector, axis1) < 0.0f) axis1 = -axis1;
    Vec3 axis2{Vec3::cross(axis0, axis1)};
    
    // Return the transform which moves a model at the origin *into* alignment with this
    // model. To align this model upright, you must invert this transform.
    return Mat3x4{
        axis0.x, axis1.x, axis2.x, centroid.x,
        axis0.y, axis1.y, axis2.y, centroid.y,
        axis0.z, axis1.z, axis2.z, centroid.z
    };
}

Mat3x4 computePointwisePrincipalAxes(const std::vector<math::Vec3>& positions)
{
    using namespace math;

    Eigen::Matrix3f Moment;
    Moment.setZero();

    Vec3 areaVector{0.0f, 0.0f, 0.0f};
    Vec3 centroid(computeCentroid(positions));

    for (int i = 0; i < positions.size(); i++) {
        const Vec3 pA = positions[i];

        Vec3 offset = pA - centroid;
        areaVector += offset;

        Moment(0, 0) += offset.x * offset.x;
        Moment(1, 0) += offset.x * offset.y;
        Moment(2, 0) += offset.x * offset.z;

        //Moment(0, 1) += offset.y * offset.x * weight;
        Moment(1, 1) += offset.y * offset.y;
        Moment(2, 1) += offset.y * offset.z;

        //Moment(0, 2) += offset.z * offset.x * weight;
        //Moment(1, 2) += offset.z * offset.y * weight;
        Moment(2, 2) += offset.z * offset.z;
    }
    
    // Recall that if a self-adjoint matrix is real, then it's also symmetric, and the
    // eigenvalues of a real, symmetric matrix are real and also orthogonal.
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> eigensolver(Moment);
    
    auto eigenvectors = eigensolver.eigenvectors();
    auto eigenvalues = eigensolver.eigenvalues();
    
    // Eigenvalues are sorted in *increasing* order, so the strongest axis is the last
    Vec3 axis0 = toVec3(eigenvectors.col(2));
    Vec3 axis1 = toVec3(eigenvectors.col(1));
    
    // Eigenvectors are unique up to a sign, so we use the integrated area vector to
    // select an orientation stable with respect to small peturtbations. The first two
    // eigenvectors correspond to the strongest eigenvalues, so we flip their parity.
    // The third eigenvalue encodes the least amount of information, so we overwrite it
    // to enforce the correct parity.
    if (Vec3::dot(areaVector, axis0) < 0.0f) axis0 = -axis0;
    if (Vec3::dot(areaVector, axis1) < 0.0f) axis1 = -axis1;
    Vec3 axis2{Vec3::cross(axis0, axis1)};
    
    // Return the transform which moves a model at the origin *into* alignment with this
    // model. To align this model upright, you must invert this transform.
    return Mat3x4{
        axis0.x, axis1.x, axis2.x, centroid.x,
        axis0.y, axis1.y, axis2.y, centroid.y,
        axis0.z, axis1.z, axis2.z, centroid.z
    };
}

}

} // namespace StandardCyborg {
