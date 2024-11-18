//
//  FastRand.hpp
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 5/1/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

class FastRand {
public:
    FastRand(unsigned int seedIn = 0) :
        _seed(seedIn)
    {}

    void seed(unsigned int seed)
    {
        _seed = seed;
    }
    
    inline int sample(int range)
    {
        _seed = 214013 * _seed + 2531011;
        return ((_seed >> 16) & 0x7FFF) % range;
    }
    
    inline int sample()
    {
        _seed = 214013 * _seed + 2531011;
        return (_seed >> 16) & 0x7FFF;
    }
    
private:
    unsigned int _seed;
};
