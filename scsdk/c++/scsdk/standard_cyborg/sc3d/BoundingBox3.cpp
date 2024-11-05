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


#include "standard_cyborg/sc3d/BoundingBox3.hpp"
#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/util/DebugHelpers.hpp"
#include "standard_cyborg/sc3d/Polyline.hpp"
#include "standard_cyborg/sc3d/ColorImage.hpp"
#include <iostream>

#include <cmath>

namespace standard_cyborg {
namespace sc3d {

using math::Vec3;
using math::Mat3x4;

BoundingBox3::BoundingBox3() {}

BoundingBox3::BoundingBox3(float xmin, float ymin, float zmin, float xmax, float ymax, float zmax) :
    lower({xmin, ymin, zmin}),
    upper({xmax, ymax, zmax})
{}

BoundingBox3::BoundingBox3(const Vec3& lowerBound, const Vec3& upperBound) :
    lower(lowerBound),
    upper(upperBound)
{}

BoundingBox3::BoundingBox3(const std::vector<Vec3>& positions)
{
    for (auto p : positions) {
        upper = Vec3::max(upper, p);
        lower = Vec3::min(lower, p);
    }
}

BoundingBox3::BoundingBox3(const std::vector<Vec3>& positions, const Mat3x4& transform)
{
    for (auto p : positions) {
        Vec3 mp = transform * p;
        upper = Vec3::max(upper, mp);
        lower = Vec3::min(lower, mp);
    }
}

BoundingBox3::BoundingBox3(const Geometry& geometry)
{
    const std::vector<Vec3>& positions = geometry.getPositions();
    
    for (auto p : positions) {
        upper = Vec3::max(upper, p);
        lower = Vec3::min(lower, p);
    }
}

BoundingBox3::BoundingBox3(const Geometry& geometry, const Mat3x4& transform)
{
    const std::vector<Vec3>& positions = geometry.getPositions();
    
    for (auto p : positions) {
        Vec3 mp = transform * p;
        upper = Vec3::max(upper, mp);
        lower = Vec3::min(lower, mp);
    }
}

BoundingBox3::BoundingBox3(const ColorImage& image)
{
    float aspectInv = (float)image.getHeight() / image.getWidth();
    lower = Vec3({-1.0, -aspectInv, 0.0});
    upper = Vec3({1.0, aspectInv, 0.0});
}
#include <iostream>

BoundingBox3::BoundingBox3(const ColorImage& image, const Mat3x4& transform)
{
    float aspectInv = (float)image.getHeight() / image.getWidth();
    Vec3 p = transform * Vec3({-1.0, -aspectInv, 0.0});
    lower = Vec3::min(lower, p);
    upper = Vec3::max(upper, p);
    
    p = transform * Vec3({1.0, -aspectInv, 0.0});
    lower = Vec3::min(lower, p);
    upper = Vec3::max(upper, p);
    
    p = transform * Vec3({1.0, aspectInv, 0.0});
    lower = Vec3::min(lower, p);
    upper = Vec3::max(upper, p);
    
    p = transform * Vec3({1.0, -aspectInv, 0.0});
    lower = Vec3::min(lower, p);
    upper = Vec3::max(upper, p);
}

BoundingBox3::BoundingBox3(const Polyline& polyline) {
    const std::vector<Vec3>& positions = polyline.getPositions();
    
    for (auto p : positions) {
        upper = Vec3::max(upper, p);
        lower = Vec3::min(lower, p);
    }
}

BoundingBox3::BoundingBox3(const Polyline& polyline, const Mat3x4& transform)
{
    const std::vector<Vec3>& positions = polyline.getPositions();
    
    for (auto p : positions) {
        Vec3 mp = transform * p;
        upper = Vec3::max(upper, mp);
        lower = Vec3::min(lower, mp);
    }
}

math::Vec3 BoundingBox3::getUpper()
{
    return upper;
}

math::Vec3 BoundingBox3::getLower()
{
    return lower;
}

void BoundingBox3::combine(const BoundingBox3& other)
{
    lower = Vec3::min(this->lower, other.lower);
    upper = Vec3::max(this->upper, other.upper);
}

math::Vec3 BoundingBox3::center() const
{
    return 0.5f * (upper + lower);
}

math::Vec3 BoundingBox3::shape() const
{
    return upper - lower;
}

math::Vec3 BoundingBox3::extent() const
{
    return 0.5f * (upper - lower);
}

float BoundingBox3::radius() const
{
    return 0.5f * (upper - lower).norm();
}

float BoundingBox3::squaredRadius() const
{
    return 0.25f * (upper - lower).squaredNorm();
}

BoundingBox3 BoundingBox3::combination(const BoundingBox3& a, const BoundingBox3& b)
{
    return BoundingBox3(
        Vec3::min(a.lower, b.lower),
        Vec3::max(a.upper, b.upper)
    );
}

} // namespace sc3d
} // namespace standard_cyborg
