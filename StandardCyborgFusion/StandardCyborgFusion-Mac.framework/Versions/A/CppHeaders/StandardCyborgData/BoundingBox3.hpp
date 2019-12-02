//
//  BoundingBox3.hpp
//  StandardCyborgData
//
//  Created by Ricky Reusser on 5/17/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <limits>
#include <vector>

#include <StandardCyborgData/Mat3x4.hpp>
#include <StandardCyborgData/MathHelpers.hpp>
#include <StandardCyborgData/Vec3.hpp>

namespace StandardCyborg {

class Geometry;
class Polyline;
class ColorImage;

struct BoundingBox3 {
    // These limits are chose in the same sense that, for example, JavaScript gives:
    //     Math.min()
    //     => Infinity
    // and
    //     Math.max()
    //     => -Infinity
    // An empty bounding box isn't very meaninful, but this makes other operations
    // work as expected.
    Vec3 lower = Vec3{std::numeric_limits<float>::infinity()};
    Vec3 upper = Vec3{-std::numeric_limits<float>::infinity()};
    
    /* Construct an empty bounding box */
    BoundingBox3();
    
    /* Construct a bounding box from the six bounds */
    BoundingBox3(float xmin, float ymin, float zmin, float xmax, float ymax, float zmax);
    
    /* Construct a bounding box from two corners */
    BoundingBox3(const Vec3& lowerBound, const Vec3& upperBound);
    
    /* Compute the bounding box of a vector of Vec3's */
    BoundingBox3(const std::vector<Vec3>& positions);
    
    /* Compute the bounding box of a vector of transformed Vec3's */
    BoundingBox3(const std::vector<Vec3>& positions, const Mat3x4& transform);
    
    /* Compute the bounding box of a geometry's vertices */
    BoundingBox3(const Geometry& geometry);
    
    /* Compute the bounding box of a geometry's vertices */
    BoundingBox3(const Geometry& geometry, const Mat3x4& transform);
    
    /* Compute the bounding box of an image */
    BoundingBox3(const ColorImage& image);
    
    /* Compute the bounding box of an image */
    BoundingBox3(const ColorImage& image, const Mat3x4& transform);
    
    /* Compute the bounding box of a polyline's vertices */
    BoundingBox3(const Polyline& polyline);
    
    /* Compute the bounding box of a polyline's vertices */
    BoundingBox3(const Polyline& polyline, const Mat3x4& transform);
    
    Vec3 getUpper();
    Vec3 getLower();
    
    /* Compute the center of the bounding box */
    Vec3 center() const;
    
    /* combine this aabb with another aabb. Which means that this aabb is enlarged, so that it precisely contains both aabbs. */
    void combine(const BoundingBox3& other);

    /* Return the shape of the bounding box in each dimension */
    Vec3 shape() const;
    
    /* Return *half* the size of the bounding box in each dimension */
    Vec3 extent() const;

    /* Compute the length of the diagonal of a bounding box */
    float radius() const;
    
    /* Compute the squared length of half of the digaonal of the box */
    float squaredRadius() const;
    
    // Compute whether two bounding boxes are equal to within floating point epsilon */
    static inline bool almostEqual(
        const BoundingBox3& lhs,
        const BoundingBox3& rhs,
        float relativeTolerance = std::numeric_limits<float>::epsilon(),
        float absoluteTolerance = std::numeric_limits<float>::epsilon()
    );
    
    static BoundingBox3 combination(const BoundingBox3& a, const BoundingBox3& b);
};

/* Equality operators */
inline bool operator==(const BoundingBox3& lhs, const BoundingBox3& rhs)
{
    return lhs.lower == rhs.lower && lhs.upper == rhs.upper;
}

inline bool operator!=(const BoundingBox3& lhs, const BoundingBox3& rhs)
{
    return !(lhs == rhs);
}

inline bool BoundingBox3::almostEqual(const BoundingBox3& lhs, const BoundingBox3& rhs, float relativeTolerance, float absoluteTolerance)
{
    return Vec3::almostEqual(lhs.lower, rhs.lower, relativeTolerance, absoluteTolerance) &&
           Vec3::almostEqual(lhs.upper, rhs.upper, relativeTolerance, absoluteTolerance);
}

} // namespace StandardCyborg
