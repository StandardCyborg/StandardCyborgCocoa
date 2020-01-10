//
//  Point2D.hpp
//  StandardCyborgSDK
//
//  Created by Ricky Reusser on 12/10/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

namespace StandardCyborg {

struct Point2D {
    int x = 0;
    int y = 0;
    
    inline int operator[](int i) const;
};

inline int Point2D::operator[](int i) const
{
    void* thisVoid = (void*)(this);
    return ((int*)thisVoid)[i];
}

} // namespace StandardCyborg
