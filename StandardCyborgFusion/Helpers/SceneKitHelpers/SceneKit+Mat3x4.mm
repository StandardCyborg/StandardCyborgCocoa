//
//  SceneKit+Mat3x4.m
//  StandardCyborgData
//
//  Created by Eric Arneback on 5/21/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import "SceneKit+Mat3x4.h"
#include <vector>
#include <iostream>

#include <standard_cyborg/math/Vec3.hpp>

using namespace standard_cyborg;
using math::Vec3;
using math::Mat3x4;

@implementation SCNNode (StandardCyborgDataMat3x4)

+ (instancetype)nodeFromMat3x4:(const Mat3x4&)mat withScale:(float)scale
{
    SCNGeometrySource *positionSource;
    {
        Vec3 o { mat.m03, mat.m13, mat.m23 };

        Vec3 positions[] = {
            o, o + Vec3::normalize({mat.m00, mat.m10, mat.m20}) * scale,
            o, o + Vec3::normalize({mat.m01, mat.m11, mat.m21}) * scale,
            o, o + Vec3::normalize({mat.m02, mat.m12, mat.m22}) * scale,
        };

        NSData *positionData = [NSData dataWithBytes:(const void *)positions
                                              length:sizeof(positions)];
        
        positionSource = [SCNGeometrySource geometrySourceWithData:positionData
                                                          semantic:SCNGeometrySourceSemanticVertex
                                                       vectorCount:sizeof(positions) / sizeof(Vec3)
                                                   floatComponents:YES
                                               componentsPerVector:3
                                                 bytesPerComponent:sizeof(float)
                                                        dataOffset:0
                                                        dataStride:sizeof(Vec3)];
    
    }
    
    SCNGeometrySource *colorSource;
    {
        Vec3 colors[] = {
            Vec3(1.0f, 0.1f, 0.1f),
            Vec3(1.0f, 0.1f, 0.1f),
            Vec3(0.1f, 1.0f, 0.1f),
            Vec3(0.1f, 1.0f, 0.1f),
            Vec3(0.1f, 0.1f, 1.0f),
            Vec3(0.1f, 0.1f, 1.0f)
        };
        
        NSData *colorData = [NSData dataWithBytes:(const void *)colors
                                           length:sizeof(colors)];
        
        colorSource = [SCNGeometrySource geometrySourceWithData:colorData
                                                       semantic:SCNGeometrySourceSemanticColor
                                                    vectorCount:sizeof(colors) / sizeof(Vec3)
                                                floatComponents:YES
                                            componentsPerVector:3
                                              bytesPerComponent:sizeof(float)
                                                     dataOffset:0
                                                     dataStride:sizeof(Vec3)];
    }
    
    SCNGeometryElement *elements;
    elements = [SCNGeometryElement geometryElementWithData:nil
                                             primitiveType:SCNGeometryPrimitiveTypeLine
                                            primitiveCount:3
                                             bytesPerIndex:sizeof(int)];

    SCNGeometry *scnGeometry = [SCNGeometry geometryWithSources:@[positionSource, colorSource]
                                                       elements:@[elements]];
    
    SCNNode *node = [SCNNode nodeWithGeometry:scnGeometry];

    return node;
}

@end
