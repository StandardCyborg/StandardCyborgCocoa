//
//  SceneKit+Geometry.mm
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 3/28/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <standard_cyborg/algorithms/Centroid.hpp>
#import <standard_cyborg/sc3d/Face3.hpp>
#import <standard_cyborg/sc3d/Geometry.hpp>
#import <standard_cyborg/math/Vec3.hpp>
#import <standard_cyborg/io/imgfile/ColorImageFileIO.hpp>
#import <vector>

#import "SceneKit+Geometry.h"

using namespace std;

using namespace standard_cyborg;

@implementation SCNGeometrySource (StandardCyborgDataGeometry)

+ (instancetype)vertexSourceFromGeometry:(const sc3d::Geometry&)geometry
{
    NSData *data = [NSData dataWithBytes:(const void *)geometry.getPositions().data()
                                  length:geometry.vertexCount() * sizeof(math::Vec3)];
    
    return [SCNGeometrySource geometrySourceWithData:data
                                            semantic:SCNGeometrySourceSemanticVertex
                                         vectorCount:geometry.vertexCount()
                                     floatComponents:YES
                                 componentsPerVector:3
                                   bytesPerComponent:sizeof(float)
                                          dataOffset:0
                                          dataStride:sizeof(math::Vec3)];
}

+ (instancetype)normalSourceFromGeometry:(const sc3d::Geometry&)geometry
{
    NSData *data = [NSData dataWithBytes:(const void *)geometry.getNormals().data()
                                  length:geometry.vertexCount() * sizeof(math::Vec3)];
    
    return [SCNGeometrySource geometrySourceWithData:data
                                            semantic:SCNGeometrySourceSemanticNormal
                                         vectorCount:geometry.vertexCount()
                                     floatComponents:YES
                                 componentsPerVector:3
                                   bytesPerComponent:sizeof(float)
                                          dataOffset:0
                                          dataStride:sizeof(math::Vec3)];
}

+ (instancetype)colorSourceFromGeometry:(const sc3d::Geometry&)geometry
{
    NSData *data = [NSData dataWithBytes:(const void *)geometry.getColors().data()
                                  length:geometry.vertexCount() * sizeof(math::Vec3)];
    
    return [SCNGeometrySource geometrySourceWithData:data
                                            semantic:SCNGeometrySourceSemanticColor
                                         vectorCount:geometry.vertexCount()
                                     floatComponents:YES
                                 componentsPerVector:3
                                   bytesPerComponent:sizeof(float)
                                          dataOffset:0
                                          dataStride:sizeof(math::Vec3)];
}

+ (instancetype)texCoordSourceFromGeometry:(const sc3d::Geometry&)geometry
{
    std::vector<math::Vec2> flippedTexCoords;
    flippedTexCoords.reserve(geometry.vertexCount());
    
    for (math::Vec2 texCoord : geometry.getTexCoords()) {
        flippedTexCoords.push_back(math::Vec2{texCoord.x, 1.0f - texCoord.y});
    }
    
    NSData *data = [NSData dataWithBytes:(const void *)flippedTexCoords.data()
                                  length:geometry.vertexCount() * sizeof(math::Vec2)];
    
    return [SCNGeometrySource geometrySourceWithData:data
                                            semantic:SCNGeometrySourceSemanticTexcoord
                                         vectorCount:geometry.vertexCount()
                                     floatComponents:YES
                                 componentsPerVector:2
                                   bytesPerComponent:sizeof(float)
                                          dataOffset:0
                                          dataStride:sizeof(math::Vec2)];
}

@end


@implementation SCNGeometryElement (StandardCyborgDataGeometry)

+ (instancetype)pointElementFromGeometry:(const sc3d::Geometry&)geometry
{
    SCNGeometryElement *element = [SCNGeometryElement geometryElementWithData:nil
                                                                primitiveType:SCNGeometryPrimitiveTypePoint
                                                               primitiveCount:geometry.vertexCount()
                                                                bytesPerIndex:sizeof(int)];
    element.minimumPointScreenSpaceRadius = 4;
    element.maximumPointScreenSpaceRadius = 6;
    element.pointSize = 5;
    
    return element;
}

+ (instancetype)faceElementFromGeometry:(const sc3d::Geometry&)geometry
{
    NSData *data = [NSData dataWithBytes:(const void *)geometry.getFaces().data()
                                  length:geometry.faceCount() * sizeof(sc3d::Face3)];
    
    return [SCNGeometryElement geometryElementWithData:data
                                         primitiveType:SCNGeometryPrimitiveTypeTriangles
                                        primitiveCount:geometry.faceCount()
                                         bytesPerIndex:sizeof(int)];
}

@end


@implementation SCNGeometry (StandardCyborgDataGeometry)

+ (instancetype)geometryFromGeometry:(const sc3d::Geometry&)geometry
{
    SCNGeometrySource *positionSource = [SCNGeometrySource vertexSourceFromGeometry:geometry];
    SCNGeometrySource *normalSource = [SCNGeometrySource normalSourceFromGeometry:geometry];
    SCNGeometrySource *colorSource = geometry.hasTexCoords()
                                        ? [SCNGeometrySource texCoordSourceFromGeometry:geometry]
                                        : [SCNGeometrySource colorSourceFromGeometry:geometry];
    SCNGeometryElement *element;
    if (geometry.hasFaces()) {
        element = [SCNGeometryElement faceElementFromGeometry:geometry];
    } else {
        element = [SCNGeometryElement pointElementFromGeometry:geometry];
    }
    
    SCNGeometry *scnGeometry = [SCNGeometry geometryWithSources:@[positionSource, normalSource, colorSource]
                                                       elements:@[element]];
    
    scnGeometry.firstMaterial.doubleSided = YES;
    
    if (geometry.hasTexture()) {
        // Dump the texture to a JPEG image
        // This is the asiest way to feed the texture to SceneKit
        const sc3d::ColorImage& texture = geometry.getTexture();
        NSString *textureJPEGPath = [NSTemporaryDirectory() stringByAppendingFormat:@"/SCNGeometry-texture-%@.jpeg", [[NSUUID UUID] UUIDString]];
        
        io::imgfile::WriteColorImageToFile([textureJPEGPath UTF8String], texture, io::imgfile::ImageFormat::JPEG);
        
        scnGeometry.firstMaterial.diffuse.contents = [NSURL fileURLWithPath:textureJPEGPath];
    }
    
    return scnGeometry;
}

- (void)toGeometry:(sc3d::Geometry&)geometryOut
{
    vector<math::Vec3> positions, normals, colors;
    vector<sc3d::Face3> faces;
    
    SCNGeometrySource *positionSource = [[self geometrySourcesForSemantic:SCNGeometrySourceSemanticVertex] firstObject];
    SCNGeometrySource *normalSource = [[self geometrySourcesForSemantic:SCNGeometrySourceSemanticNormal] firstObject];
    SCNGeometrySource *colorSource = [[self geometrySourcesForSemantic:SCNGeometrySourceSemanticColor] firstObject];
    SCNGeometryElement *facesElement = [[self geometryElements] firstObject];
    
    positions.resize([positionSource vectorCount]);
    normals.resize([normalSource vectorCount]);
    colors.resize([colorSource vectorCount]);
    faces.resize([facesElement primitiveCount]);
    
    [positionSource.data getBytes:positions.data() length:positionSource.data.length];
    [normalSource.data   getBytes:normals.data()   length:normalSource.data.length  ];
    [colorSource.data    getBytes:colors.data()    length:colorSource.data.length   ];
    [facesElement.data   getBytes:faces.data()     length:facesElement.data.length  ];
    
    geometryOut.setVertexData(positions, normals, colors);
    geometryOut.setFaces(faces);
}

@end


@implementation SCNNode (StandardCyborgDataGeometry)

+ (instancetype)nodeFromGeometry:(const sc3d::Geometry&)geometry withDefaultTransform:(BOOL)useDefaultTransform
{
    SCNGeometry *scnGeometry = [SCNGeometry geometryFromGeometry:geometry];
    SCNNode *node = [SCNNode nodeWithGeometry:scnGeometry];
    
    if (useDefaultTransform) {
        double boundingSphereRadius;
        [node getBoundingSphereCenter:NULL radius:&boundingSphereRadius];
        math::Vec3 center = algorithms::computeCentroid(geometry.getPositions());

         if (boundingSphereRadius > 0) {
            /* TRANSFORM NOTES!
             - We scale the node to fit based on its bounding radius, increased by a small factor to look just right.
             - The model needs to be rotated 90º about Z for it to appear correctly in SceneKit
             - The order of operations in the node's transform seems to be rotation, translation, scale.
             Therefore, it is necessary to pre-apply scale to the position.
             Also, after rotating 90º about z, (-x,-y,-z) becomes (y,-x,-z).
             */
            double scale = 0.3 / boundingSphereRadius;

            node.position = SCNVector3Make( center.y * scale,
                                           -center.x * scale,
                                           -center.z * scale);
            node.scale = SCNVector3Make(scale, scale, scale);
        }
    }

    return node;
}

@end
