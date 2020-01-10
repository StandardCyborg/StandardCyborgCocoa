//
//  Transform.hpp
//  StandardCyborgData
//
//  Created by Ricky Reusser on 12/3/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <cmath>
#include <limits>

#include "Quaternion.hpp"
#include "Vec3.hpp"

namespace StandardCyborg {

struct Mat3x4;

struct Transform {
    Quaternion rotation;
    Vec3 translation;
    Vec3 scale = Vec3{1.0f, 1.0f, 1.0f};
    Vec3 shear;
    
    static Transform fromMat3x4(const Mat3x4& matrix);
};

} // namespace StandardCyborg
