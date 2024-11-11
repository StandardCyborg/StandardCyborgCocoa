//
//  SCMesh+Geometry.mm
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 10/17/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <vector>
#import <standard_cyborg/io/imgfile/ColorImageFileIO.hpp>

#import "SCMesh_Private.h"
#import "SCMesh+Geometry.h"

using namespace standard_cyborg;
using sc3d::Face3;

namespace math = standard_cyborg::math;
using math::Vec4;
using math::Vec3;
using math::Vec2;

@implementation SCMesh (StandardCyborgGeometry)

+ (NSData *)_positionDataFromGeometry:(const sc3d::Geometry &)geo
{
    return [NSData dataWithBytes:geo.getPositions().data()
                          length:geo.vertexCount() * sizeof(Vec3)];
}

+ (NSData *)_colorDataFromGeometry:(const sc3d::Geometry &)geo
{
    return [NSData dataWithBytes:geo.getColors().data()
                          length:geo.vertexCount() * sizeof(Vec3)];
}


+ (NSData *)_normalDataFromGeometry:(const sc3d::Geometry &)geo
{
    std::vector<Vec3> normalizedNormals;
    for (Vec3 normal : geo.getNormals()) {
        if (normal.squaredNorm() < 0.00000000001f) {
            normalizedNormals.push_back(Vec3{1, 0, 0});
        } else {
            normalizedNormals.push_back(normal.normalize());
        }
    }
    
    return [NSData dataWithBytes:(const void *)normalizedNormals.data()
                          length:geo.vertexCount() * sizeof(Vec3)];
}

+ (NSData *)_texCoordDataFromGeometry:(const sc3d::Geometry &)geo
{
    return [NSData dataWithBytes:(const void *)geo.getTexCoords().data()
                          length:geo.vertexCount() * sizeof(Vec2)];
}

+ (NSData *)_facesDataFromGeometry:(const sc3d::Geometry &)geo
{
    return [NSData dataWithBytes:(const void *)geo.getFaces().data()
                          length:geo.faceCount() * sizeof(int) * 3];
}

+ (SCMesh *)meshWithVertexColorsFromGeometry:(const sc3d::Geometry &)geo
{    
    if (geo.vertexCount() == 0 || geo.faceCount() == 0) {
        return nil;
    }
    
    NSData *positionData = [self _positionDataFromGeometry:geo];
    NSData *normalData = [self _normalDataFromGeometry:geo];
    NSData *colorData = [self _colorDataFromGeometry:geo];
    NSData *facesData = [self _facesDataFromGeometry:geo];

    return [[SCMesh alloc] initWithPositionData:positionData
                                     normalData:normalData
                                      colorData:colorData
                                      facesData:facesData];
}

+ (SCMesh *)meshFromGeometry:(const sc3d::Geometry &)geo
                 textureData:(const std::vector<float> &)textureDataVec
           textureResolution:(NSInteger)textureResolution
{
    NSParameterAssert(textureDataVec.size() == textureResolution * textureResolution * 4);
    
    if (geo.vertexCount() == 0 || geo.faceCount() == 0 || textureResolution <= 0) {
        return nil;
    }
    
    NSData *positionData = [self _positionDataFromGeometry:geo];
    NSData *normalData = [self _normalDataFromGeometry:geo];
    NSData *texCoordData = [self _texCoordDataFromGeometry:geo];
    NSData *facesData = [self _facesDataFromGeometry:geo];
    NSData *textureData = [NSData dataWithBytes:textureDataVec.data()
                                         length:textureDataVec.size() * sizeof(float)];
    
    return [[SCMesh alloc] initWithPositionData:positionData
                                     normalData:normalData
                                   texCoordData:texCoordData
                                      facesData:facesData
                                    textureData:textureData
                                   textureWidth:textureResolution
                                  textureHeight:textureResolution];
}

+ (SCMesh *)meshFromGeometry:(const sc3d::Geometry &)geo
             textureJPEGPath:(NSString *)JPEGPath
{
    if (geo.vertexCount() == 0 || geo.faceCount() == 0) {
        return nil;
    }
    
    NSData *positionData = [self _positionDataFromGeometry:geo];
    NSData *normalData = [self _normalDataFromGeometry:geo];
    NSData *texCoordData = [self _texCoordDataFromGeometry:geo];
    NSData *facesData = [self _facesDataFromGeometry:geo];
    
    return [[SCMesh alloc] initWithPositionData:positionData
                                     normalData:normalData
                                   texCoordData:texCoordData
                                      facesData:facesData
                                textureJPEGPath:JPEGPath];
}

- (void)toGeometry:(sc3d::Geometry &)geo
{
    Vec3 *positionsStart = (Vec3 *)[self.positionData bytes];
    Vec3 *positionsEnd = positionsStart + [self.positionData length] / sizeof(Vec3);
    
    Vec3 *normalsStart = (Vec3 *)[self.normalData bytes];
    Vec3 *normalsEnd = normalsStart + [self.normalData length] / sizeof(Vec3);
    
    Vec3 *colorsStart = (Vec3 *)[self.colorData bytes];
    Vec3 *colorsEnd = colorsStart + [self.colorData length] / sizeof(Vec3);
    
    Vec2 *texCoordsStart = (Vec2 *)[self.texCoordData bytes];
    Vec2 *texCoordsEnd = texCoordsStart + [self.texCoordData length] / sizeof(Vec2);
    
    Face3 *facesStart = (Face3 *)[self.facesData bytes];
    Face3 *facesEnd = facesStart + [self.facesData length] / sizeof(Face3);
    
    std::vector<Vec3> positions(positionsStart, positionsEnd);
    std::vector<Vec3> normals(normalsStart, normalsEnd);
    std::vector<Vec2> texCoords(texCoordsStart, texCoordsEnd);
    std::vector<Face3> faces(facesStart, facesEnd);
    
    geo.setPositions(positions);
    geo.setNormals(normals);
    if ([self.colorData length] > 0) {
        std::vector<Vec3> colors(colorsStart, colorsEnd);
        geo.setColors(colors);
    }
    geo.setTexCoords(texCoords);
    geo.setFaces(faces);
    
    if (self.textureData != nil) {
        std::vector<Vec4> rgba(self.textureWidth * self.textureHeight, Vec4{0, 0, 0, 0});
        memcpy(rgba.data(), [self.textureData bytes], [self.textureData length]);
        
        sc3d::ColorImage texture((int)self.textureWidth,
                           (int)self.textureHeight,
                           rgba);
        
        geo.setTexture(texture);
    } else if (self.textureJPEGPath != nil) {
        sc3d::ColorImage texture;
        
        if (io::imgfile::ReadColorImageFromFile(texture, [self.textureJPEGPath UTF8String])) {
            geo.setTexture(texture);
        } else {
            NSLog(@"Error reading color image from %@", self.textureJPEGPath);
        }
    }
}

@end
