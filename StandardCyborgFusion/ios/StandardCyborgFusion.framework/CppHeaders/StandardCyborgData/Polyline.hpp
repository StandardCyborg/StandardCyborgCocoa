//
//  Polyline.hpp
//  StandardCyborgData
//
//  Created by Ricky Reusser on 5/16/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <vector>

#include <StandardCyborgData/Vec3.hpp>

namespace StandardCyborg {

class Polyline {
public:
    Polyline(const std::vector<Vec3>& positions);
    
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
    void setPositions(const std::vector<Vec3>& positions);
    
    /* Const getter for position data */
    const std::vector<Vec3>& getPositions() const;
    
    /* Return true of the first and last points of the polyline are floating-point identical
     * to within the specified tolerance
     */
    bool isClosed(float relativeTolerance = std::numeric_limits<float>::epsilon(),
                  float absoluteTolerance = std::numeric_limits<float>::epsilon()) const;
    
    /* Compute the length of the polyline */
    float length() const;
    
private:
    std::vector<Vec3> _positions;
};

} // namespace StandardCyborg
