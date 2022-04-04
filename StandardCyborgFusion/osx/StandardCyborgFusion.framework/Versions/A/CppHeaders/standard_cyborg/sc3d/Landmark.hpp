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

#include <string>

#include "standard_cyborg/math/Vec3.hpp"

namespace standard_cyborg {
namespace sc3d {

class Landmark {
private:
public:
    std::string name;
    math::Vec3 position;
    
    std::string frame;

    std::string getFrame() const { return frame; }

    void setFrame(const std::string &f) { frame = f; }

    
    const std::string& getName() const
    {
        return name;
    }
    
    const math::Vec3& getPosition() const
    {
        return position;
    }
    
    void setName(const std::string& name_)
    {
        name = name_;
    }
    
    void setPosition(const math::Vec3& position_)
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

} // namespace sc3d
} // namespace standard_cyborg
