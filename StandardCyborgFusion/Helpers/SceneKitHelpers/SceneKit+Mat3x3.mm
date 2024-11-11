//
//  SceneKit+Mat3x4.m
//  StandardCyborgData
//
//  Created by Eric Arneback on 5/21/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import "SceneKit+Mat3x3.h"
#include <vector>

#include <standard_cyborg/math/Vec3.hpp>
#include <standard_cyborg/math/Mat3x4.hpp>

#import "SceneKit+Mat3x4.h"

using namespace standard_cyborg;

@implementation SCNNode (StandardCyborgDataMat3x3)

+ (instancetype)nodeFromMat3x3:(const math::Mat3x3&)mat withScale:(float)scale
{
    math::Mat3x4 m = {
        mat.m00, mat.m01, mat.m02, 0.0f,
        mat.m10, mat.m11, mat.m12, 0.0f,
        mat.m20, mat.m21, mat.m22, 0.0f
    };
    
    SCNNode *node = [SCNNode nodeFromMat3x4:m withScale:scale];

    return node;
}

@end
