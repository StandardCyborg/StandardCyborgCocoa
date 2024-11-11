//
//  SCMeshTests.mm
//  StandardCyborgFusionTests
//
//  Created by Aaron Thompson on 10/19/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "SCMesh+Geometry.h"
#import "SCMesh+FileIO.h"
#import "SCMesh_Private.h"

using namespace standard_cyborg;
using namespace standard_cyborg::math;
using namespace standard_cyborg::sc3d;


@interface SCMeshTests : XCTestCase

@end

@implementation SCMeshTests

- (void)_buildTestGeometry:(sc3d::Geometry &)geometry
{
    
    std::vector<Vec3> positions, normals;
    std::vector<Vec2> texCoords;
    std::vector<Face3> faces;
    
    positions.push_back(Vec3(-1, -1, 2));
    positions.push_back(Vec3(1, -1, 2));
    positions.push_back(Vec3(1, 1, 2));
    positions.push_back(Vec3(-1, 1, 2));
    
    normals.push_back(Vec3(-1, -2, -3));
    normals.push_back(Vec3(-2, -1, -3));
    normals.push_back(Vec3(-3, -1, -2));
    normals.push_back(Vec3(-3, -2, -1));
    
    texCoords.push_back(Vec2(0.2, 0.3));
    texCoords.push_back(Vec2(0.3, 0.5));
    texCoords.push_back(Vec2(0.5, 0.2));
    texCoords.push_back(Vec2(0.5, 0.3));
    
    faces.push_back(Face3(0, 1, 2));
    faces.push_back(Face3(1, 2, 3));
    
    geometry.setPositions(positions);
    geometry.setNormals(normals);
    geometry.setTexCoords(texCoords);
    geometry.setFaces(faces);
}

- (void)testFromAndToGeometry
{
    Geometry geometry;
    [self _buildTestGeometry:geometry];
    
    NSString *texturePath = [[NSBundle bundleForClass:[self class]] pathForResource:@"SquareRedImage" ofType:@"jpeg"];
    std::vector<float> textureData;
    NSInteger textureResolution = 0;
    [SCMesh readTextureFromImageAtPath:texturePath intoVector:textureData textureResolution:&textureResolution];
    
    SCMesh *mesh = [SCMesh meshFromGeometry:geometry
                                textureData:textureData
                          textureResolution:textureResolution];
    
    // Now read it back
    Geometry readGeometry;
    [mesh toGeometry:readGeometry];
    
    XCTAssertTrue(readGeometry.getPositions() == geometry.getPositions());
    // Normals are normalized upon reading in
    // XCTAssertTrue(readGeometry.getNormals() == geometry.getNormals());
    XCTAssertTrue(readGeometry.getFaces() == geometry.getFaces());
    XCTAssertTrue(readGeometry.getTexCoords() == geometry.getTexCoords());
    XCTAssertTrue(readGeometry.hasTexture());
}

- (void)testToAndFromPLY
{
    NSString *PLYPath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"testToAndFromPLY.ply"];
    NSString *texturePath = [[NSBundle bundleForClass:[self class]] pathForResource:@"SquareRedImage" ofType:@"jpeg"];
    
    Geometry geometry;
    [self _buildTestGeometry:geometry];
    
    std::vector<float> textureData;
    NSInteger textureResolution = 0;
    [SCMesh readTextureFromImageAtPath:texturePath intoVector:textureData textureResolution:&textureResolution];
    
    SCMesh *mesh = [SCMesh meshFromGeometry:geometry
                                textureData:textureData
                          textureResolution:textureResolution];
    
    [mesh writeToPLYAtPath:PLYPath];
    SCMesh *readMesh = [[SCMesh alloc] initWithPLYPath:PLYPath
                                              JPEGPath:texturePath];
    
    XCTAssertEqual([readMesh vertexCount], [mesh vertexCount]);
    XCTAssertEqualObjects([readMesh positionData], [mesh positionData]);
    // XCTAssertEqualObjects([readMesh normalData], [mesh normalData]);
    XCTAssertEqualObjects([readMesh texCoordData], [mesh texCoordData]);
    XCTAssertEqualObjects([readMesh facesData], [mesh facesData]);
}

/*
- (void)blah
{
    ReadGeometryFromPLYFile(readGeometry, [PLYPath UTF8String]);
    XCTAssertEqual(readGeometry.vertexCount(), positions.size());
    XCTAssertEqual(readGeometry.faceCount(), faces.size());
    
    Vec3 readPosition1 = readGeometry.getPositions()[1];
    Vec3 readNormal1 = readGeometry.getNormals()[1];
    Vec3 readColor1 = readGeometry.getColors()[1];
    Face3 readFace1 = readGeometry.getFaces()[1];
    XCTAssertEqual(readPosition1, positions[1]);
    
    XCTAssertEqualWithAccuracy(readNormal1.x, -2, 1e-4);
    XCTAssertEqualWithAccuracy(readNormal1.y, -1, 1e-4);
    XCTAssertEqualWithAccuracy(readNormal1.z, -3, 1e-4);
    
    // Due to reading and writing as 8-bit values, we only have an accuracy range of 1/256
    XCTAssertEqualWithAccuracy(readColor1.x, colors[1].x, 1.0 / 256.0);
    XCTAssertEqualWithAccuracy(readColor1.y, colors[1].y, 1.0 / 256.0);
    XCTAssertEqualWithAccuracy(readColor1.z, colors[1].z, 1.0 / 256.0);
    XCTAssertEqual(readFace1, faces[1]);
}
*/
@end
