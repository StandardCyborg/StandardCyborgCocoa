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

#include "standard_cyborg/math/Vec3.hpp"

#include <string>

namespace standard_cyborg {
namespace sc3d {

class Polyline {
public:
    Polyline(const std::vector<math::Vec3>& positions);
    
    Polyline();
    
    ~Polyline();
    
    /* Makes this Polyline into a deep copy of 'that'*/
    void copy(const Polyline& that);
    
    // Disallow evil constructors
    // Edit: too damned annoying. what cost are we actually saving here???
    //Polyline(Polyline&&) = delete;
    //Polyline& operator=(Polyline&&) = delete;
    //Polyline(Polyline const& other) = delete;
    //Polyline& operator=(Polyline const& other) = delete;
    
    /* Return the size of the position vector */
    int vertexCount() const;
    
    /* Set positions */
    void setPositions(const std::vector<math::Vec3>& positions);
    
    /* Const getter for position data */
    const std::vector<math::Vec3>& getPositions() const;
    
    /* Return true of the first and last points of the polyline are floating-point identical
     * to within the specified tolerance
     */
    bool isClosed(float relativeTolerance = std::numeric_limits<float>::epsilon(),
                  float absoluteTolerance = std::numeric_limits<float>::epsilon()) const;
    
    /* Compute the length of the polyline */
    float length() const;
    
    std::string getFrame() const { return frame; }
    void setFrame(const std::string &f) { frame = f; }

private:
    std::string frame;
    std::vector<math::Vec3> _positions;
};

bool operator==(const Polyline& lhs, const Polyline& rhs);

} // namespace sc3d
} // namespace standard_cyborg
