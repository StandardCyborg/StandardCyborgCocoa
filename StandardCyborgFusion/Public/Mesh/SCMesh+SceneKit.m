//
//  SCMesh+SceneKit.m
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 10/19/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <Metal/Metal.h>
#import <SceneKit/SceneKit.h>

#import "SCMesh.h"
#import "SCMesh_Private.h"
#import "SCMesh+FileIO.h"
#import "SCMesh+SceneKit.h"

@implementation SCMesh (SceneKit)

- (SCNGeometrySource *)buildVertexGeometrySource
{
    return [SCNGeometrySource geometrySourceWithData:self.positionData
                                            semantic:SCNGeometrySourceSemanticVertex
                                         vectorCount:self.vertexCount
                                     floatComponents:YES
                                 componentsPerVector:3
                                   bytesPerComponent:sizeof(float)
                                          dataOffset:0
                                          dataStride:4 * sizeof(float)];
}


- (SCNGeometrySource *)buildColorGeometrySource
{
    return [SCNGeometrySource geometrySourceWithData:self.colorData
                                            semantic:SCNGeometrySourceSemanticColor
                                         vectorCount:self.vertexCount
                                     floatComponents:YES
                                 componentsPerVector:3
                                   bytesPerComponent:sizeof(float)
                                          dataOffset:0
                                          dataStride:4 * sizeof(float)];
}


- (SCNGeometrySource *)buildNormalGeometrySource
{
    return [SCNGeometrySource geometrySourceWithData:self.normalData
                                            semantic:SCNGeometrySourceSemanticNormal
                                         vectorCount:self.vertexCount
                                     floatComponents:YES
                                 componentsPerVector:3
                                   bytesPerComponent:sizeof(float)
                                          dataOffset:0
                                          dataStride:4 * sizeof(float)];
}

- (SCNGeometrySource *)buildTexCoordGeometrySource
{
    // Flip about y to match what SceneKit expects
    NSMutableData *flippedTexCoordData = [self.texCoordData mutableCopy];
    float *texCoords = (float *)[flippedTexCoordData mutableBytes];
    
    for (NSInteger i = 0; i < self.vertexCount; ++i) {
        texCoords[i * 2 + 1] = 1.0f - texCoords[i * 2 + 1];
    }
    
    return [SCNGeometrySource geometrySourceWithData:flippedTexCoordData
                                            semantic:SCNGeometrySourceSemanticTexcoord
                                         vectorCount:self.vertexCount
                                     floatComponents:YES
                                 componentsPerVector:2
                                   bytesPerComponent:sizeof(float)
                                          dataOffset:0
                                          dataStride:2 * sizeof(float)];
}

- (SCNGeometryElement *)buildMeshGeometryElement
{
    return [SCNGeometryElement geometryElementWithData:self.facesData
                                         primitiveType:SCNGeometryPrimitiveTypeTriangles
                                        primitiveCount:self.faceCount
                                         bytesPerIndex:sizeof(int)];
}

- (SCNNode *)buildMeshNode
{
    // Dump the texture to a JPEG image
    // This is the asiest way to feed the texture to SceneKit
    if (self.textureJPEGPath == nil) {
        NSString *tempJPEGPath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"temp-buildMeshNode.jpeg"];
        [self writeTextureToJPEGAtPath:tempJPEGPath];
        self.textureJPEGPath = tempJPEGPath;
    }
    
    SCNGeometrySource *positionSource = [self buildVertexGeometrySource];
    SCNGeometrySource *normalSource = [self buildNormalGeometrySource];
    SCNGeometryElement *element = [self buildMeshGeometryElement];
    SCNGeometry *geometry;
    
    if (self.colorData != nil) {
        SCNGeometrySource *colorSource = [self buildColorGeometrySource];
        geometry = [SCNGeometry geometryWithSources:@[positionSource, normalSource, colorSource]
                                           elements:@[element]];

    } else {
        SCNGeometrySource *texCoordSource = [self buildTexCoordGeometrySource];
        geometry = [SCNGeometry geometryWithSources:@[positionSource, normalSource, texCoordSource]
                                           elements:@[element]];
        geometry.firstMaterial.diffuse.contents = [NSURL fileURLWithPath:self.textureJPEGPath];
    }
    
    geometry.firstMaterial.doubleSided = YES;
    
    SCNNode *node = [SCNNode nodeWithGeometry:geometry];
    
    return node;
}

@end
