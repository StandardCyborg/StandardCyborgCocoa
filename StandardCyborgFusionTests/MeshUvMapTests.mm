//
//  MeshUvMapTests.mm
//  StandardCyborgAlgorithmsTests
//
//  Created by eric on 2019-08-13.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <XCTest/XCTest.h>
#import <sstream>

#import <standard_cyborg/sc3d/Face3.hpp>
#import <standard_cyborg/sc3d/Geometry.hpp>
#import <standard_cyborg/math/Vec3.hpp>
#import <standard_cyborg/io/ply/GeometryFileIO_PLY.hpp>

#import <StandardCyborgFusion/MeshUvMap.hpp>

@interface MeshUvMapTests : XCTestCase

@end

@implementation MeshUvMapTests

using namespace standard_cyborg;
using namespace standard_cyborg::algorithms;
using namespace standard_cyborg::math;
using namespace standard_cyborg::sc3d;

- (void)testSeamCutMeshBySegmentation2Segment {
    
    std::vector<Vec3> positions {
        {0.0f, 1.0f, 0.0f},
        {+1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f},
        {0.0f, -1.0f, 0.0f},
    };
    
    std::vector<Vec3> normals {
        {0.0f, 0.0f, +1.0f},
        {0.0f, 0.0f, +1.0f},
        {0.0f, 0.0f, +1.0f},
        {0.0f, 0.0f, +1.0f},
    };
    
    std::vector<Vec3> colors {
        {0.0f, 1.0f, +0.0f},
        {1.0f, 0.0f, +0.0f},
        {0.0f, 0.0f, +1.0f},
        {0.0f, 1.0f, +1.0f},
    };
    
    std::vector<Face3> faces {
        { 0, 1, 2 },
        { 1, 3, 2 },
        
    };
    
    Geometry geometry0;
    geometry0.setFaces(faces);
    geometry0.setPositions(positions);
    geometry0.setNormals(normals);
    geometry0.setColors(colors);
    
    
    std::vector<std::set<int>> segmentation;
    segmentation.push_back(std::set<int>{0});
    segmentation.push_back(std::set<int>{1});
    
    auto result = seamCutMeshBySegmentation(geometry0, segmentation);
    
    XCTAssertTrue(result.size() == 2);
    
    {
        auto geo0 = result[0];
        
        std::vector<Vec3> pos = {
            {0.0f, 1.0f, 0.0f},
            {1.0f, 0.0f, 0.0f},
            {-1.0f,0.0f, 0.0f}};
        
        std::vector<Vec3> colors {
            {0.0f, 1.0f, 0.0f},
            {1.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 1.0f},
        };
        
        std::vector<Vec3> normals {
            {0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 1.0f},
        };
        std::vector<Face3> faces {
            { 0, 1, 2 },
        };
        
        XCTAssertTrue(geo0->getPositions() == pos);
        XCTAssertTrue(geo0->getNormals() == normals);
        XCTAssertTrue(geo0->getColors() == colors);
        XCTAssertTrue(geo0->getFaces() == faces);
    }
    
    
    {
        auto geo0 = result[1];
        
        std::vector<Vec3> pos = {
            {1.0f, 0.0f, 0.0f},
            {0.0f, -1.0f, 0.0f},
            {-1.0f,0.0f, 0.0f}};
        
        std::vector<Vec3> colors {
            {1.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 1.0f},
            {0.0f, 0.0f, 1.0f},
        };
        
        std::vector<Vec3> normals {
            {0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 1.0f},
        };
        std::vector<Face3> faces {
            { 0, 1, 2 },
        };
        
        XCTAssertTrue(geo0->getPositions() == pos);
        XCTAssertTrue(geo0->getNormals() == normals);
        XCTAssertTrue(geo0->getColors() == colors);
        XCTAssertTrue(geo0->getFaces() == faces);
    }
}

- (void)testSeamCutMeshBySegmentation3Segment {
    
    std::vector<Vec3> positions {
        {4.0f, 1.0f, 0.0f},
        {6.0f, 1.0f, 0.0f},
        {4.0f, 3.0f, 0.0f},
        
        {6.0f, 3.0f, 0.0f},
        {6.0f, 5.0f, 0.0f},
        
        {2.0f, 5.0f, 0.0f},
        {2.0f, 3.0f, 0.0f},
        
    };
    
    std::vector<Face3> faces {
        { 0, 1, 2 },
        
        { 2, 3, 4 },
        
        { 2, 5, 6},
        
    };
    
    Geometry geometry0;
    geometry0.setFaces(faces);
    geometry0.setPositions(positions);
    
    std::vector<std::set<int>> segmentation;
    segmentation.push_back(std::set<int>{0});
    segmentation.push_back(std::set<int>{1});
    segmentation.push_back(std::set<int>{2});
    
    auto result = seamCutMeshBySegmentation(geometry0, segmentation);
    
    XCTAssertTrue(result.size() == 3);
    
    {
        auto geo0 = result[0];
        
        std::vector<Vec3> pos = {
            {4.0f, 1.0f, 0.0f},
            {6.0f, 1.0f, 0.0f},
            {4.0f, 3.0f, 0.0f}};
        
        std::vector<Face3> faces {
            { 0, 1, 2 },
        };
        
        XCTAssertTrue(geo0->getPositions() == pos);
        XCTAssertTrue(geo0->getFaces() == faces);
    }
    
    {
        auto geo0 = result[1];
        
        std::vector<Vec3> pos = {
            {4.0f, 3.0f, 0.0f},
            
            {6.0f, 3.0f, 0.0f},
            {6.0f, 5.0f, 0.0f},
            
            
            
        };
        
        std::vector<Face3> faces {
            { 0, 1, 2 },
        };
        
        XCTAssertTrue(geo0->getPositions() == pos);
        XCTAssertTrue(geo0->getFaces() == faces);
    }
    
    {
        auto geo0 = result[2];
        
        std::vector<Vec3> pos = {
            {4.0f, 3.0f, 0.0f},
            
            {2.0f, 5.0f, 0.0f},
            {2.0f, 3.0f, 0.0f},
            
        };
        
        std::vector<Face3> faces {
            { 0, 1, 2 },
        };
        
        XCTAssertTrue(geo0->getPositions() == pos);
        XCTAssertTrue(geo0->getFaces() == faces);
    }
}

// test where one segment has a vertex shared between several faces, in that segment.
- (void)testSeamCutMeshBySegmentationMoreComplex {
    
    std::vector<Vec3> positions {
        {1.0f, 1.0f, 0.0f},
        {3.0f, 3.0f, 0.0f},
        {1.0f, 3.0f, 0.0f},
        
        {3.0f, 5.0f, 0.0f},
        {1.0f, 5.0f, 0.0f},
    };
    
    std::vector<Face3> faces {
        { 0, 1, 2 },
        { 1, 3, 2 },
        { 2, 3, 4},
    };
    
    Geometry geometry0;
    geometry0.setFaces(faces);
    geometry0.setPositions(positions);
    
    std::vector<std::set<int>> segmentation;
    segmentation.push_back(std::set<int>{0 });
    segmentation.push_back(std::set<int>{1, 2});
    
    auto result = seamCutMeshBySegmentation(geometry0, segmentation);
    
    {
        auto geo0 = result[0];
        
        std::vector<Vec3> pos = {
            {1.0f, 1.0f, 0.0f},
            {3.0f, 3.0f, 0.0f},
            {1.0f, 3.0f, 0.0f}};
        
        std::vector<Face3> faces {
            { 0, 1, 2 },
        };
        
        XCTAssertTrue(geo0->getPositions() == pos);
        XCTAssertTrue(geo0->getFaces() == faces);
    }
    
    {
        auto geo0 = result[1];
        
        std::vector<Vec3> pos = {
            
            {3.0f, 3.0f, 0.0f},
            {3.0f, 5.0f, 0.0f},
            {1.0f, 3.0f, 0.0f},
            
            {1.0f, 5.0f, 0.0f},
            
        };
        
        
        std::vector<Face3> faces {
            { 0, 1, 2 },
            { 2, 1, 3 },
            
        };
        
        XCTAssertTrue(geo0->getPositions() == pos);
        XCTAssertTrue(geo0->getFaces() == faces);
    }
}


// test where one segment has a vertex shared between several faces, in that segment.
- (void)testRemoveDegenerateTriangles {
    
    
    std::vector<Vec3> positions {
        {2.0f, 2.0f, 0.0f},
        {3.0f, 4.0f, 0.0f},
        {1.0f, 4.0f, 0.0f},
        {3.0f, 4.0f, 0.0f},
        {2.0f, 6.0f, 0.0f},
    };
    
    std::vector<Vec3> normals {
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
    };
    
    std::vector<Face3> faces {
        { 0, 1, 2 },
        { 2, 3, 4 },
        { 2, 1, 3 },
    };
    
    Geometry geometry0;
    geometry0.setFaces(faces);
    geometry0.setPositions(positions);
    geometry0.setNormals(normals);
    
    removeDegenerateTriangles(geometry0);
    
    printf("lol");
    
    {
        std::vector<Face3> newFaces {
            { 0, 1, 2 },
            { 2, 1, 3 },
        };
        
        std::vector<Vec3> newPositions {
            {2.0f, 2.0f, 0.0f},
            {3.0f, 4.0f, 0.0f},
            {1.0f, 4.0f, 0.0f},
            {2.0f, 6.0f, 0.0f},
        };
        
        std::vector<Vec3> newNormals {
            {0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 1.0f},
        };
        
        XCTAssertTrue(geometry0.getPositions() == newPositions);
        XCTAssertTrue(geometry0.getNormals() == newNormals);
        
        XCTAssertTrue(geometry0.getFaces() == newFaces);
        
    }
}

- (void)testRemoveDegenerateTrianglesMoreComplex {
    std::vector<Vec3> positions {
        {1.0f, 1.0f, 0.0f},
        {3.0f, 3.0f, 0.0f},
        {1.0f, 3.0f, 0.0f},
        
        {3.0f, 5.0f, 0.0f},
        {1.0f, 5.0f, 0.0f},
        
        {1.0f, 3.0f, 0.0f},
    };
    
    std::vector<Face3> faces {
        { 0, 1, 2 },
        
        { 1, 3, 5 },
        { 5, 3, 4},
        
        { 1, 5, 2}, // degenerate triangle.
    };
    
    
    Geometry geometry0;
    geometry0.setFaces(faces);
    geometry0.setPositions(positions);
    
    removeDegenerateTriangles(geometry0);
    
    {
        std::vector<Face3> newFaces {
            { 0, 1, 4 },
            { 1, 2, 4 },
            { 4, 2, 3 },
        };
        
        std::vector<Vec3> newPositions {
            {1.0f, 1.0f, 0.0f},
            {3.0f, 3.0f, 0.0f},
            {3.0f, 5.0f, 0.0f},
            {1.0f, 5.0f, 0.0f},
            {1.0f, 3.0f, 0.0f},
        };
        
        XCTAssertTrue(geometry0.getPositions() == newPositions);
        XCTAssertTrue(geometry0.getFaces() == newFaces);
    }
}


- (void)testRemoveDegenerateTrianglesColinearPoints {
       
    std::vector<Vec3> positions {
        {1.0f, 2.0f, 0.0f}, // 0
        {2.0f, 3.0f, 0.0f}, // 1
        {4.0f, 3.0f, 0.0f}, // 2
        {2.0f, 2.0f, 0.0f}, // 3
        {4.0f, 1.0f, 0.0f}, // 4
        {2.0f, 1.0f, 0.0f}, // 5
    };
    
    std::vector<Face3> faces {
        { 0, 5, 1 },
        { 5, 4, 3 },
        { 3, 2, 1 },

        { 5, 3, 1}, // degenerate triangle, where the points are colinear.
    };
    
    Geometry geometry0;
    geometry0.setFaces(faces);
    geometry0.setPositions(positions);
    
    removeDegenerateTriangles(geometry0);
    
    {
        std::vector<Face3> newFaces {
            { 5, 4, 3 },
            { 3, 2, 1 },
            { 0, 5, 3 },
            { 3, 1, 0 },
        };
        
        XCTAssertTrue(geometry0.getPositions() == positions);
        XCTAssertTrue(geometry0.getFaces() == newFaces);
    }
}


- (void)testPackCharts {
    {
        std::vector<std::pair<float, float> > chartSizes;
        
        chartSizes.push_back({0.25, 0.25});
        chartSizes.push_back({0.25, 0.25});
        chartSizes.push_back({0.25, 0.25});
        chartSizes.push_back({0.25, 0.25});
        
        chartSizes.push_back({0.25, 0.25});
        chartSizes.push_back({0.25, 0.25});
        chartSizes.push_back({0.25, 0.25});
        chartSizes.push_back({0.25, 0.25});
        
        chartSizes.push_back({0.25, 0.25});
        chartSizes.push_back({0.25, 0.25});
        chartSizes.push_back({0.25, 0.25});
        chartSizes.push_back({0.25, 0.25});
        
        chartSizes.push_back({0.25, 0.25});
        chartSizes.push_back({0.25, 0.25});
        chartSizes.push_back({0.25, 0.25});
        chartSizes.push_back({0.25, 0.25});
        
        float textureSize;
        std::vector<std::tuple<float, float, float, float, bool>> result =
        packCharts(chartSizes, textureSize);
        
        XCTAssertTrue(1.0f == textureSize);
        XCTAssertTrue(chartSizes.size() == result.size());
    }
}

// test where one segment has a vertex shared between several faces, in that segment.
- (void)testSeamCutMesh {
    std::vector<Vec3> positions {
        {-1, 0, 1}, // 0
        {1, 0, 1}, // 1
        {-1, 0, -1}, // 2
        {1, 0, -1}, // 3
        {-1, 0, 0}, // 4
        {0, 0, 1}, // 5 cut
        {1, 0, 0}, // 6
        {0, 0, -1}, // 7 cut
        {0, 0, 0}, // 8 cut
    };
    
    std::vector<Face3> faces {
        {6, 7, 8},
        {8, 2, 4},
        {5, 4, 0},
        {1, 8, 5},
        {6, 3, 7},
        {8, 7, 2},
        {5, 8, 4},
        {1, 6, 8},
    };
    
    std::vector<int> seamCut{7, 8, 5};
    
    Geometry geometry0;
    geometry0.setFaces(faces);
    geometry0.setPositions(positions);
    
    XCTAssertTrue(seamCutMesh(geometry0, seamCut));
    
    std::vector<Vec3> newPos = {
        {-1, 0, 1},
        {1, 0, 1},
        {-1, 0, -1},
        {1, 0, -1},
        {-1, 0, 0},
        {0, 0, 1},
        {1, 0, 0},
        {0, 0, -1},
        {0, 0, 0},
        {0, 0, -1},
        {0, 0, 0},
        {0, 0, 1},
    };
    
    std::vector<Face3> newFaces {
        {6, 7, 8},
        {10, 2, 4},
        {11, 4, 0},
        {1, 8, 5},
        {6, 3, 7},
        {10, 9, 2},
        {11, 10, 4},
        {1, 6, 8},
    };
    
    XCTAssertTrue(geometry0.getPositions() == newPos);
    XCTAssertTrue(geometry0.getFaces() == newFaces);
}

/*
- (void)testUvmapMeshStressTest {
    using namespace StandardCyborg;
    
    std::vector<NSString*> testCases {
        @"stress-test-colinear-tri-points.ply",
        @"stress-test-crash-mesh.ply",
        @"stress-test-fails-dijsktra.ply",
        @"stress-test-kitchen.ply"
    };
    
    for(NSString* testCase: testCases)
    {
        NSString *PLYPath = [[PathHelpers testCasesPath] stringByAppendingPathComponent:testCase];
        Geometry geometry;
        std::string cppPath([PLYPath UTF8String]);
        ReadGeometryFromPLYFile(geometry, cppPath);
        XCTAssertTrue(geometry.faceCount() > 0);
        if (!StandardCyborg::uvmapMesh(geometry)) {
            printf("UV unwrapping failed %s\n %s", cppPath.c_str(), StandardCyborg::getUvmapMeshErrorMessage().c_str());
            XCTAssertTrue(false);
        }
    }
    
}
*/

@end
