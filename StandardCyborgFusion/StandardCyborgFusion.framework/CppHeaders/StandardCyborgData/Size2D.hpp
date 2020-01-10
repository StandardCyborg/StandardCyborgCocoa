//
//  Size2D.hpp
//  StandardCyborgData
//
//  Created by Ricky Reusser on 12/10/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//


#pragma once

namespace StandardCyborg {

struct Size2D {
    int width = 0;
    int height = 0;
    
    inline int operator[](int i) const;
};

inline int Size2D::operator[](int i) const
{
    void* thisVoid = (void*)(this);
    return ((int*)thisVoid)[i];
}

} // namespace StandardCyborg
