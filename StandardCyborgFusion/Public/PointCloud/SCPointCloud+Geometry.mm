//
//  SCPointCloud+Geometry.mm
//  StandardCyborgData
//
//  Created by Ricky Reusser on 8/8/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import "SCPointCloud+Geometry.h"
#import <standard_cyborg/math/Vec3.hpp>

using namespace standard_cyborg;

@implementation SCPointCloud (Geometry)

- (void)toGeometry:(sc3d::Geometry&)geometry
{
    int vertexCount = (int)[self pointCount];
    
    std::vector<math::Vec3> positions(vertexCount);
    std::vector<math::Vec3> normals(vertexCount);
    std::vector<math::Vec3> colors(vertexCount);
    
    size_t stride = [SCPointCloud pointStride] / sizeof(float);
    size_t positionOffset = [SCPointCloud positionOffset] / sizeof(float);
    size_t normalOffset = [SCPointCloud normalOffset] / sizeof(float);
    size_t colorOffset = [SCPointCloud colorOffset] / sizeof(float);
    size_t pointSizeOffset = [SCPointCloud pointSizeOffset] / sizeof(float);
    
    const float* floatData = (float*)[[self pointsData] bytes];
    
    for (int i = 0; i < vertexCount; i++) {
        positions[i].x = floatData[i * stride + positionOffset + 0];
        positions[i].y = floatData[i * stride + positionOffset + 1];
        positions[i].z = floatData[i * stride + positionOffset + 2];
    }
    
    for (int i = 0; i < vertexCount; i++) {
        const float surfelSize = floatData[i * stride + pointSizeOffset];
        
        normals[i].x = surfelSize * floatData[i * stride + normalOffset + 0];
        normals[i].y = surfelSize * floatData[i * stride + normalOffset + 1];
        normals[i].z = surfelSize * floatData[i * stride + normalOffset + 2];
    }
    
    for (int i = 0; i < vertexCount; i++) {
        colors[i].x = floatData[i * stride + colorOffset];
        colors[i].y = floatData[i * stride + colorOffset + 1];
        colors[i].z = floatData[i * stride + colorOffset + 2];
    }
    
    geometry.setPositions(positions);
    geometry.setNormals(normals);
    geometry.setColors(colors);
}

@end
