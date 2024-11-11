//
//  SceneKit+BoundingBox3.m
//  StandardCyborgData
//
//  Created by Eric Arneback on 5/21/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import "SceneKit+BoundingBox3.h"
#include <vector>

#include <standard_cyborg/math/Vec3.hpp>

using namespace standard_cyborg;

@implementation SCNNode (StandardCyborgDataBoundingBox3)

+ (instancetype)nodeFromBoundingBox3:(const sc3d::BoundingBox3&)boundingBox
{
    
    SCNGeometrySource *positionSource;
    {
        math::Vec3 lower = boundingBox.lower;
        math::Vec3 upper = boundingBox.upper;

        math::Vec3 positions[] = {
            math::Vec3(lower.x, lower.y, lower.z),
            math::Vec3(lower.x, upper.y, lower.z),
            math::Vec3(lower.x, lower.y, upper.z),
            math::Vec3(lower.x, upper.y, upper.z),
    
            math::Vec3(upper.x, lower.y, lower.z),
            math::Vec3(upper.x, upper.y, lower.z),
            math::Vec3(upper.x, lower.y, upper.z),
            math::Vec3(upper.x, upper.y, upper.z)
        };
        
        NSData *data = [NSData dataWithBytes:(const void *)positions
                                      length:sizeof(positions)];
        
        positionSource =  [SCNGeometrySource geometrySourceWithData:data
                                                           semantic:SCNGeometrySourceSemanticVertex
                                                        vectorCount:sizeof(positions) / sizeof(math::Vec3)
                                                    floatComponents:YES
                                                componentsPerVector:3
                                                  bytesPerComponent:sizeof(float)
                                                         dataOffset:0
                                                         dataStride:sizeof(math::Vec3)];
    }
    
    
    SCNGeometrySource *colorSource;
    {
        math::Vec3 c { 1.0f, 1.0f, 1.0f };
        
        math::Vec3 colors[] = {
            c, c,
            c, c,
            c, c,
            c, c
        };
        
        NSData *data = [NSData dataWithBytes:(const void *)colors
                                      length:sizeof(colors)];
        
        colorSource =  [SCNGeometrySource geometrySourceWithData:data
                                                        semantic:SCNGeometrySourceSemanticColor
                                                     vectorCount:sizeof(colors) / sizeof(math::Vec3)
                                                 floatComponents:YES
                                             componentsPerVector:3
                                               bytesPerComponent:sizeof(float)
                                                      dataOffset:0
                                                      dataStride:sizeof(math::Vec3)];
    }
    
    SCNGeometryElement *element;
    {
        int indices[] = {
            0, 1,
            1, 3,
            3, 2,
            2, 0,
            
            4, 5,
            5, 7,
            7, 6,
            6, 4,
            
            0, 4,
            1, 5,
            2, 6,
            3, 7
        };
        
        NSData *indexData = [NSData dataWithBytes:indices
                                           length:sizeof(indices)];
        
        element = [SCNGeometryElement geometryElementWithData:indexData
                                                primitiveType:SCNGeometryPrimitiveTypeLine
                                               primitiveCount:sizeof(indices) / sizeof(int) / 2
                                                bytesPerIndex:sizeof(int)];
    }
    
    SCNGeometry *scnGeometry = [SCNGeometry geometryWithSources:@[positionSource,colorSource]
                                                       elements:@[element]];
    
    SCNNode *node = [SCNNode nodeWithGeometry:scnGeometry];
    
    return node;
}

@end

