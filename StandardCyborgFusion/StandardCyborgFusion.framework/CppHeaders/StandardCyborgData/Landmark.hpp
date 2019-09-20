//
//  Landmark.h
//  StandardCyborgSDK
//
//  Created by eric on 2019-06-24.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <string>

#include <StandardCyborgData/Vec3.hpp>

namespace StandardCyborg {

class Landmark {
public:
    std::string name;
    Vec3 position;
    
    const std::string& getName() const
    {
        return name;
    }
    
    const Vec3& getPosition() const
    {
        return position;
    }
    
    void setName(const std::string& name_)
    {
        name = name_;
    }
    
    void setPosition(const Vec3& position_)
    {
        position = position_;
    }
    
    bool operator==(const Landmark& other)
    {
        return name == other.name && position == other.position;
    }
    
    bool operator!=(const Landmark& other)
    {
        return !(*this == other);
    }
};

}
