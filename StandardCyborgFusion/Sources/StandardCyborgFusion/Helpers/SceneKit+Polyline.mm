//
//  SceneKit+Polyline.m
//  StandardCyborgData
//
//  Created by Ricky Reusser on 5/20/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import "SceneKit+Polyline.h"
#import <standard_cyborg/math/Vec3.hpp>
#import <vector>

@implementation SCNNode (StandardCyborgDataPolyline)

+ (instancetype)nodeFromPolyline:(const sc3d::Polyline&)polyline
{
    SCNGeometrySource *positionSource;
    {
        NSData *data = [NSData dataWithBytes:(const void *)polyline.getPositions().data()
                                      length:polyline.vertexCount() * sizeof(math::Vec3)];
    
        positionSource = [SCNGeometrySource geometrySourceWithData:data
                                                          semantic:SCNGeometrySourceSemanticVertex
                                                       vectorCount:polyline.vertexCount()
                                                   floatComponents:YES
                                               componentsPerVector:3
                                                 bytesPerComponent:sizeof(float)
                                                        dataOffset:0
                                                        dataStride:sizeof(math::Vec3)];
    }
    
    SCNGeometryElement *element;
    {
        std::vector<int> elementData;
        for (int i = 0; i < polyline.vertexCount() - 1; i++) {
            elementData.push_back(i);
            elementData.push_back(i + 1);
        }

        NSData *indexData = [NSData dataWithBytes:elementData.data()
                                           length:elementData.size() * sizeof(int)];
        
        element = [SCNGeometryElement geometryElementWithData:indexData
                                                primitiveType:SCNGeometryPrimitiveTypeLine
                                               primitiveCount:elementData.size() / 2
                                                bytesPerIndex:sizeof(int)];
    }
    
    SCNGeometry *scnGeometry = [SCNGeometry geometryWithSources:@[positionSource]
                                                       elements:@[element]];
    
    SCNNode *node = [SCNNode nodeWithGeometry:scnGeometry];
    
    return node;
}

@end
