//
//  Surfel.hpp
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 7/31/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#ifndef Surfel_hpp
#define Surfel_hpp

#ifdef __METAL_VERSION__

// Alows us to use the same Surfel struct in our Metal shaders as in our C++ code
#include <metal_stdlib>
using namespace metal;

#define Vector3f  packed_float3
#define uint32_t  uint
#define uint64_t  uint2

#else /* ! __METAL_VERSION__ */

#include <standard_cyborg/util/IncludeEigen.hpp>
using Vector3f = Eigen::Vector3f;

#endif // __METAL_VERSION__

struct Surfel {
    Vector3f position;
    
    Vector3f normal;
    
    Vector3f color;
    
    // A running count of how many surfels have been averaged, then we do a (frankly fairly numerical
    // questionably-stable but okay for how much data we have here) online weighted average
    float weight;
    
    // A finite lifetime. Every time a surfel is touched, its lifetime is reset. If its lifetime
    // expires and it has low confidence, it's culled
    uint32_t lifetime;
    
    float surfelSize;
};

#ifndef __METAL_VERSION__

#define METAL_REQUIRED_ALIGNMENT 4096

typedef std::vector<Surfel> Surfels __attribute__((aligned(METAL_REQUIRED_ALIGNMENT)));

#ifdef DEBUG
// To prevent std::vector from agressively inlining, which prevents inspection with lldb
// https://stackoverflow.com/questions/39680320/printing-debugging-libc-stl-with-xcode-lldb
#include <vector>
template struct std::vector<Surfel>;
#endif /* DEBUG */

#endif /* __METAL_VERSION__ */

#endif /* Surfel_hpp */
