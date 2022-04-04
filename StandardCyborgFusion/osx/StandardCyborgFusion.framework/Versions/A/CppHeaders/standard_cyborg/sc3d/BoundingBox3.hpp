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

#include <limits>
#include <vector>

#include "standard_cyborg/math/Mat3x4.hpp"
#include "standard_cyborg/math/MathHelpers.hpp"
#include "standard_cyborg/math/Vec3.hpp"

namespace standard_cyborg {
namespace sc3d {

class Geometry;
class Polyline;
class ColorImage;

struct BoundingBox3 {
private:
public:
    // These limits are chose in the same sense that, for example, JavaScript gives:
    //     Math.min()
    //     => Infinity
    // and
    //     Math.max()
    //     => -Infinity
    // An empty bounding box isn't very meaninful, but this makes other operations
    // work as expected.
    math::Vec3 lower = math::Vec3{std::numeric_limits<float>::infinity()};
    math::Vec3 upper = math::Vec3{-std::numeric_limits<float>::infinity()};
    
    /* Construct an empty bounding box */
    BoundingBox3();
    
    /* Construct a bounding box from the six bounds */
    BoundingBox3(float xmin, float ymin, float zmin, float xmax, float ymax, float zmax);
    
    /* Construct a bounding box from two corners */
    BoundingBox3(const math::Vec3& lowerBound, const math::Vec3& upperBound);
    
    /* Compute the bounding box of a vector of Vec3's */
    BoundingBox3(const std::vector<math::Vec3>& positions);
    
    /* Compute the bounding box of a vector of transformed Vec3's */
    BoundingBox3(const std::vector<math::Vec3>& positions, const math::Mat3x4& transform);
    
    /* Compute the bounding box of a geometry's vertices */
    BoundingBox3(const Geometry& geometry);
    
    /* Compute the bounding box of a geometry's vertices */
    BoundingBox3(const Geometry& geometry, const math::Mat3x4& transform);
    
    /* Compute the bounding box of an image */
    BoundingBox3(const ColorImage& image);
    
    /* Compute the bounding box of an image */
    BoundingBox3(const ColorImage& image, const math::Mat3x4& transform);
    
    /* Compute the bounding box of a polyline's vertices */
    BoundingBox3(const Polyline& polyline);
    
    /* Compute the bounding box of a polyline's vertices */
    BoundingBox3(const Polyline& polyline, const math::Mat3x4& transform);
    
    math::Vec3 getUpper();
    math::Vec3 getLower();
    
    /* Compute the center of the bounding box */
    math::Vec3 center() const;
    
    /* combine this aabb with another aabb. Which means that this aabb is enlarged, so that it precisely contains both aabbs. */
    void combine(const BoundingBox3& other);

    /* Return the shape of the bounding box in each dimension */
    math::Vec3 shape() const;
    
    /* Return *half* the size of the bounding box in each dimension */
    math::Vec3 extent() const;

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
    return math::Vec3::almostEqual(lhs.lower, rhs.lower, relativeTolerance, absoluteTolerance) &&
           math::Vec3::almostEqual(lhs.upper, rhs.upper, relativeTolerance, absoluteTolerance);
}

} // namespace sc3d
} // namespace standard_cyborg
