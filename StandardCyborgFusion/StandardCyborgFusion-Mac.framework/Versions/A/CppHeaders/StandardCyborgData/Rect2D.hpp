//
//  Shape.hpp
//  StandardCyborgSDK
//
//  Created by Ricky Reusser on 12/10/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <StandardCyborgData/Point2D.hpp>
#include <StandardCyborgData/Size2D.hpp>

namespace StandardCyborg {

struct Rect2D {
    Size2D size;
    Point2D origin;
};

} // namespace StandardCyborg

