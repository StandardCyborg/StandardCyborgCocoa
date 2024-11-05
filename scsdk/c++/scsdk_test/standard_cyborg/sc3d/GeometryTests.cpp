/*
 Copyright 2020 Standard Cyborg
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */



#include <gtest/gtest.h>

#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/sc3d/Face3.hpp"
#include "standard_cyborg/sc3d/VertexSelection.hpp"

#include "standard_cyborg/util/DebugHelpers.hpp"

using standard_cyborg::sc3d::Geometry;
using standard_cyborg::sc3d::VertexSelection;
using standard_cyborg::sc3d::Face3;

namespace math = standard_cyborg::math;
using math::Vec4;
using math::Vec3;
using math::Vec2;

template <class T>
bool checkVectorsEqual(const T& actual, const T& expected)
{
    if (actual.size() != expected.size()) {
        std::cerr << "checkVectorsEqual: Actual size (" << actual.size() << ") not equal to expected size (" << expected.size() << ")" << std::endl;
        return false;
    }
    
    int size = (int)actual.size();
    bool equal = true;
    
    for (int i = 0; i < size; i++) {
        if (actual[i] != expected[i]) {
            std::cerr << "checkVectorsEqual: actual[" << i << "] (" << actual[i] << ") not equal to expected[" << i << "] (" << expected[i] << ")" << std::endl;
            equal = false;
        }
    }
    
    return equal;
}

TEST(GeometryTests, testSetPositions)
{
    {
        Geometry geo(
                     std::vector<Vec3>({{0.0f, 1.0f, 2.0f}}),
                     std::vector<Vec3>({{3.0f, 4.0f, 5.0f}}),
                     std::vector<Vec3>({{6.0f, 7.0f, 8.0f}})
                     );
        
        bool success = geo.setPositions(std::vector<Vec3>({{-1.0f, -2.0f, -3.0f}}));
        
        EXPECT_TRUE(success);
        EXPECT_TRUE(geo.getPositions() == std::vector<Vec3>({{-1.0f, -2.0f, -3.0f}}));
    }
    
    {
        Geometry geo;
        geo.setNormals(std::vector<Vec3>({{0.0f, 0.0f, 0.0f}}));
        
        // too many positions, which is inconsistent. so should fail.
        EXPECT_FALSE(geo.setPositions(std::vector<Vec3>({{-1.0f, -2.0f, -3.0f}, {-1.0f, -2.0f, -3.0f}})));
        EXPECT_TRUE(geo.getPositions().size() == 0);
    }
    
    {
        Geometry geo;
        geo.setColors(std::vector<Vec3>({{0.0f, 0.0f, 0.0f}}));
        
        // too many positions, which is inconsistent. so should fail.
        EXPECT_FALSE(geo.setPositions(std::vector<Vec3>({{-1.0f, -2.0f, -3.0f}, {-1.0f, -2.0f, -3.0f}})));
        EXPECT_TRUE(geo.getPositions().size() == 0);
    }
    
    {
        Geometry geo;
        geo.setTexCoords(std::vector<Vec2>({{0.0f, 0.0f}}));
        
        // too many positions, which is inconsistent. so should fail.
        EXPECT_FALSE(geo.setPositions(std::vector<Vec3>({{-1.0f, -2.0f, -3.0f}, {-1.0f, -2.0f, -3.0f}})));
        EXPECT_TRUE(geo.getPositions().size() == 0);
    }
    
    {
        
        Geometry geo(
                     std::vector<Vec3>({{0.0f, 1.0f, 2.0f}}),
                     std::vector<Vec3>({{3.0f, 4.0f, 5.0f}}),
                     std::vector<Vec3>({{6.0f, 7.0f, 8.0f}})
                     );
        
        // unset positions.
        EXPECT_TRUE(geo.setPositions(std::vector<Vec3>()));
        EXPECT_TRUE(geo.getPositions().size() == 0);
    }
}

TEST(GeometryTests, testSetNormals)
{
    {
        Geometry geo(
                     std::vector<Vec3>({{0.0f, 1.0f, 2.0f}}),
                     std::vector<Vec3>({{3.0f, 4.0f, 5.0f}}),
                     std::vector<Vec3>({{6.0f, 7.0f, 8.0f}})
                     );
        
        bool success = geo.setNormals(std::vector<Vec3>({{-1.0f, -2.0f, -3.0f}}));
        
        EXPECT_TRUE(success);
        EXPECT_TRUE(geo.getNormals() == std::vector<Vec3>({{-1.0f, -2.0f, -3.0f}}));
    }
    
    {
        Geometry geo;
        geo.setPositions(std::vector<Vec3>({{0.0f, 0.0f, 0.0f}}));
        
        // too many normals, which is inconsistent. so should fail.
        EXPECT_FALSE(geo.setNormals(std::vector<Vec3>({{-1.0f, -2.0f, -3.0f}, {-1.0f, -2.0f, -3.0f}})));
        EXPECT_TRUE(geo.getNormals().size() == 0);
    }
    
    {
        Geometry geo;
        geo.setColors(std::vector<Vec3>({{0.0f, 0.0f, 0.0f}}));
        
        // too many normals, which is inconsistent. so should fail.
        EXPECT_FALSE(geo.setNormals(std::vector<Vec3>({{-1.0f, -2.0f, -3.0f}, {-1.0f, -2.0f, -3.0f}})));
        EXPECT_TRUE(geo.getNormals().size() == 0);
    }
    
    {
        Geometry geo;
        geo.setTexCoords(std::vector<Vec2>({{0.0f, 0.0f}}));
        
        // too many normals, which is inconsistent. so should fail.
        EXPECT_FALSE(geo.setNormals(std::vector<Vec3>({{-1.0f, -2.0f, -3.0f}, {-1.0f, -2.0f, -3.0f}})));
        EXPECT_TRUE(geo.getNormals().size() == 0);
    }
    
    {
        
        Geometry geo(
                     std::vector<Vec3>({{0.0f, 1.0f, 2.0f}}),
                     std::vector<Vec3>({{3.0f, 4.0f, 5.0f}}),
                     std::vector<Vec3>({{6.0f, 7.0f, 8.0f}})
                     );
        
        // unset normals.
        EXPECT_TRUE(geo.setNormals(std::vector<Vec3>()));
        EXPECT_TRUE(geo.getNormals().size() == 0);
    }
}


TEST(GeometryTests, testSetTexCoords)
{
    {
        Geometry geo(
                     std::vector<Vec3>({{0.0f, 1.0f, 2.0f}}),
                     std::vector<Vec3>({{3.0f, 4.0f, 5.0f}}),
                     std::vector<Vec3>({{6.0f, 7.0f, 8.0f}})
                     );
        
        bool success = geo.setTexCoords(std::vector<Vec2>({{0.0f, 1.0f}}));
        
        EXPECT_TRUE(success);
        EXPECT_TRUE(geo.getTexCoords() == std::vector<Vec2>({{0.0f, 1.0f}}));
    }
    
    {
        Geometry geo;
        geo.setPositions(std::vector<Vec3>({{0.0f, 0.0f, 0.0f}}));
        
        // too many texcoords, which is inconsistent. so should fail.
        EXPECT_FALSE(geo.setTexCoords(std::vector<Vec2>({{0.0f, 1.0f}, {1.0f, 0.0f}})));
        EXPECT_TRUE(geo.getTexCoords().size() == 0);
    }
    
    {
        Geometry geo;
        geo.setColors(std::vector<Vec3>({{0.0f, 0.0f, 0.0f}}));
        
        // too many texcoords, which is inconsistent. so should fail.
        EXPECT_FALSE(geo.setTexCoords(std::vector<Vec2>({{0.0f, 1.0f}, {1.0f, 0.0f}})));
        EXPECT_TRUE(geo.getTexCoords().size() == 0);
    }
    
    {
        Geometry geo;
        geo.setNormals(std::vector<Vec3>({{0.0f, 0.0f, 0.0f}}));
        
        // too many texcoords, which is inconsistent. so should fail.
        EXPECT_FALSE(geo.setTexCoords(std::vector<Vec2>({{0.0f, 1.0f}, {1.0f, 0.0f}})));
        EXPECT_TRUE(geo.getTexCoords().size() == 0);
    }
    
    {
        
        Geometry geo(
                     std::vector<Vec3>({{0.0f, 1.0f, 2.0f}}),
                     std::vector<Vec3>({{3.0f, 4.0f, 5.0f}}),
                     std::vector<Vec3>({{6.0f, 7.0f, 8.0f}})
                     );
        
        EXPECT_TRUE(geo.setTexCoords(std::vector<Vec2>({{0.0f, 1.0f}})));
        
        // unset texcoords.
        EXPECT_TRUE(geo.setTexCoords(std::vector<Vec2>({})));
        EXPECT_TRUE(geo.getTexCoords().size() == 0);
    }
}

TEST(GeometryTests, testSetTexture)
{
    Geometry geo(std::vector<Vec3>({{0.0f, 1.0f, 2.0f}}));
    
    EXPECT_FALSE(geo.hasTexture());
    
    standard_cyborg::sc3d::ColorImage texture{1, 1, {Vec4{1.0f, 0.0f, 0.0f, 1.0f}}};
    
    bool success = geo.setTexture(texture);
    
    standard_cyborg::sc3d::ColorImage texture2{1, 1, {Vec4{1.0f, 0.0f, 0.0f, 1.0f}}};
    EXPECT_TRUE(success);
    EXPECT_TRUE(geo.getTexture() == texture2);
    EXPECT_TRUE(geo.hasTexture());
}

TEST(GeometryTests, testSetInconsistentNormals)
{
    Geometry geo(
                 std::vector<Vec3>({{0.0f, 1.0f, 2.0f}}),
                 std::vector<Vec3>({{3.0f, 4.0f, 5.0f}}),
                 std::vector<Vec3>({{6.0f, 7.0f, 8.0f}})
                 );
    
    bool success = geo.setNormals(std::vector<Vec3>({{-1.0f, -2.0f, -3.0f}, {1.0f, 2.0f, 3.0f}}));
    
    EXPECT_FALSE(success);
    EXPECT_TRUE(geo.getNormals() == std::vector<Vec3>({{3.0f, 4.0f, 5.0f}}));
}

TEST(GeometryTests, testSetColors)
{
    {
        Geometry geo(
                     std::vector<Vec3>({{0.0f, 1.0f, 2.0f}}),
                     std::vector<Vec3>({{3.0f, 4.0f, 5.0f}}),
                     std::vector<Vec3>({{6.0f, 7.0f, 8.0f}})
                     );
        
        bool success = geo.setColors(std::vector<Vec3>({{1.0f, 0.0f, 0.0f}}));
        
        EXPECT_TRUE(success);
        EXPECT_TRUE(geo.getColors() == std::vector<Vec3>({{1.0f, 0.0f, 0.0f}}));
    }
    
    {
        Geometry geo;
        geo.setPositions(std::vector<Vec3>({{0.0f, 0.0f, 0.0f}}));
        
        // too many colors, which is inconsistent. so should fail.
        EXPECT_FALSE(geo.setColors(std::vector<Vec3>({{-1.0f, -2.0f, -3.0f}, {-1.0f, -2.0f, -3.0f}})));
        EXPECT_TRUE(geo.getColors().size() == 0);
    }
    
    {
        Geometry geo;
        geo.setNormals(std::vector<Vec3>({{0.0f, 0.0f, 0.0f}}));
        
        // too many colos, which is inconsistent. so should fail.
        EXPECT_FALSE(geo.setColors(std::vector<Vec3>({{-1.0f, -2.0f, -3.0f}, {-1.0f, -2.0f, -3.0f}})));
        EXPECT_TRUE(geo.getColors().size() == 0);
    }
    
    {
        Geometry geo;
        geo.setTexCoords(std::vector<Vec2>({{0.0f, 0.0f}}));
        
        // too many colors, which is inconsistent. so should fail.
        EXPECT_FALSE(geo.setColors(std::vector<Vec3>({{-1.0f, -2.0f, -3.0f}, {-1.0f, -2.0f, -3.0f}})));
        EXPECT_TRUE(geo.getColors().size() == 0);
    }
    
    {
        
        Geometry geo(
                     std::vector<Vec3>({{0.0f, 1.0f, 2.0f}}),
                     std::vector<Vec3>({{3.0f, 4.0f, 5.0f}}),
                     std::vector<Vec3>({{6.0f, 7.0f, 8.0f}})
                     );
        
        // unset normals.
        EXPECT_TRUE(geo.setColors(std::vector<Vec3>()));
        EXPECT_TRUE(geo.getColors().size() == 0);
    }
}

TEST(GeometryTests, testConstructor)
{
    std::vector<Vec3> positions({{0.0f, 1.0f, 2.0f}});
    std::vector<Face3> faces({{1, 2, 3}});
    
    Geometry geo(positions, faces);
    
    EXPECT_TRUE(geo.getPositions() == positions);
    EXPECT_TRUE(geo.getFaces() == faces);
}

TEST(GeometryTests, testSetVertexData)
{
    Geometry geo;
    
    EXPECT_FALSE(geo.setVertexData(
                                   std::vector<Vec3>({{0.0f, 1.0f, 2.0f}}),
                                   std::vector<Vec3>({{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}),
                                   std::vector<Vec3>({{0.0f, 0.0f, 1.0f}})));
    
    EXPECT_FALSE(geo.setVertexData(
                                   std::vector<Vec3>({{0.0f, 1.0f, 2.0f}}),
                                   std::vector<Vec3>({{0.0f, 1.0f, 0.0f} }),
                                   std::vector<Vec3>({{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f} })));
    
    EXPECT_TRUE(geo.setVertexData(
                                  std::vector<Vec3>({{0.0f, 1.0f, 2.0f}}),
                                  std::vector<Vec3>({{0.0f, 1.0f, 0.0f} }),
                                  std::vector<Vec3>({{0.0f, 0.0f, 1.0f}})));
    
    EXPECT_TRUE(geo.getPositions() == std::vector<Vec3>({{0.0f, 1.0f, 2.0f}}));
    EXPECT_TRUE(geo.getNormals() == std::vector<Vec3>({{0.0f, 1.0f, 0.0f}}));
    EXPECT_TRUE(geo.getColors() == std::vector<Vec3>({{0.0f, 0.0f, 1.0f}}));
}

TEST(GeometryTests, testSetColorWithSelection)
{
    Geometry geo(
                 std::vector<Vec3>({{0.0f, 1.0f, 2.0f}, {1.0f, 2.0f, 3.0f}}),
                 std::vector<Vec3>({{3.0f, 4.0f, 5.0f}, {1.0f, 2.0f, 3.0f}}),
                 std::vector<Vec3>({{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}})
                 );
    
    geo.setColor(Vec3({0.5f, 0.5f, 0.5f}), 0.5f, VertexSelection(geo, {0}));
    
    EXPECT_TRUE(geo.getColors() == std::vector<Vec3>({{0.25f, 0.75f, 0.25f}, {0.0f, 1.0f, 0.0f}}));
}

TEST(GeometryTests, testSetColorWithoutSelection)
{
    Geometry geo(
                 std::vector<Vec3>({{0.0f, 1.0f, 2.0f}, {1.0f, 2.0f, 3.0f}}),
                 std::vector<Vec3>({{3.0f, 4.0f, 5.0f}, {1.0f, 2.0f, 3.0f}}),
                 std::vector<Vec3>({{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}})
                 );
    
    geo.setColor(Vec3({0.5f, 0.5f, 0.5f}), 0.5f);
    
    EXPECT_TRUE(geo.getColors() == std::vector<Vec3>({{0.25f, 0.75f, 0.25f}, {0.25f, 0.75f, 0.25f}}));
}

TEST(GeometryTests, testSetInconsistentColors)
{
    Geometry geo(
                 std::vector<Vec3>({{0.0f, 1.0f, 2.0f}}),
                 std::vector<Vec3>({{3.0f, 4.0f, 5.0f}}),
                 std::vector<Vec3>({{6.0f, 7.0f, 8.0f}})
                 );
    
    bool success = geo.setColors(std::vector<Vec3>({{-1.0f, -2.0f, -3.0f}, {1.0f, 2.0f, 3.0f}}));
    
    EXPECT_FALSE(success);
    EXPECT_TRUE(geo.getColors() == std::vector<Vec3>({{6.0f, 7.0f, 8.0f}}));
}

TEST(GeometryTests, testUnsetNormals)
{
    Geometry geo(
                 std::vector<Vec3>({{0.0f, 1.0f, 2.0f}}),
                 std::vector<Vec3>({{3.0f, 4.0f, 5.0f}}),
                 std::vector<Vec3>({{6.0f, 7.0f, 8.0f}})
                 );
    
    EXPECT_TRUE(geo.hasNormals());
    bool success = geo.setNormals(std::vector<Vec3>({}));
    EXPECT_TRUE(success);
    EXPECT_FALSE(geo.hasNormals());
}

TEST(GeometryTests, testGetClosestPoint)
{
    std::vector<Vec3> positions{
        {+1.0f, +2.0f, +3.0f},
        {+6.0f, +7.0f, +8.0f},
        {-2.0f, -1.0f, -6.0f},
        {-9.0f, +4.0f, +1.0f},
    };
    
    Geometry tri(positions);
    
    EXPECT_EQ(tri.getClosestVertexIndex(Vec3{-2.1f, -1.0f, -6.0f}), 2);
    EXPECT_EQ(tri.getClosestVertexIndex(Vec3{-9.1f, +4.0f, +1.0f}), 3);
    EXPECT_TRUE(tri.getClosestVertexPosition(Vec3{-2.1f, -1.0f, -6.0f}) == positions[2]);
    EXPECT_TRUE(tri.getClosestVertexPosition(Vec3{-9.1f, +4.0f, +1.0f}) == positions[3]);
}

TEST(GeometryTests, testPointsInRadius)
{
    std::vector<Vec3> positions{
        {+1.0f, +2.0f, +3.0f},
        {+6.0f, +7.0f, +8.0f},
        {-2.0f, -1.0f, -6.0f},
        {-9.0f, +4.0f, +1.0f},
    };
    
    Geometry geometry(positions);
    
    auto closest = geometry.getVertexIndicesInRadius({+1.0, +1.0, +1.0}, 4.0);
    
    EXPECT_EQ(closest.size(), 1);
}

TEST(GeometryTests, testDeleteVertices)
{
    std::vector<Vec3> positions{
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {2.0f, 0.0f, 0.0f},
        {3.0f, 0.0f, 0.0f},
        {4.0f, 0.0f, 0.0f},
        {5.0f, 0.0f, 0.0f},
        {6.0f, 0.0f, 0.0f},
        {7.0f, 0.0f, 0.0f},
        {8.0f, 0.0f, 0.0f},
    };
    std::vector<Face3> faces{
        {1, 2, 3},
        {5, 6, 8},
        {8, 7, 0}
    };
    
    std::vector<Vec3> expectedState1 = positions; // The same
    std::vector<Face3> expectedFaces1 = faces; // The same
    
    Geometry geometry(positions, {}, {}, faces);
    VertexSelection vertexSelection1(geometry, {});
    
    geometry.deleteVertices(vertexSelection1);
    EXPECT_TRUE(geometry.getPositions() == expectedState1);
    EXPECT_TRUE(geometry.getFaces() == expectedFaces1);
    
    
    VertexSelection vertexSelection2(geometry, {1, 2, 4, 5});
    std::vector<Vec3> expectedState2{
        {0.0f, 0.0f, 0.0f}, // 0
        {3.0f, 0.0f, 0.0f}, // 1
        {6.0f, 0.0f, 0.0f}, // 2
        {7.0f, 0.0f, 0.0f}, // 3
        {8.0f, 0.0f, 0.0f}, // 4
    };
    std::vector<Face3> expectedFaces2{
        {4, 3, 0}
    };
    
    geometry.deleteVertices(vertexSelection2);
    EXPECT_TRUE(geometry.getPositions() == expectedState2);
    EXPECT_TRUE(geometry.getFaces() == expectedFaces2);
    
    
    VertexSelection vertexSelection3(4, {3});
    std::vector<Vec3> expectedState3{
        {0.0f, 0.0f, 0.0f},
        {3.0f, 0.0f, 0.0f},
        {6.0f, 0.0f, 0.0f},
        {8.0f, 0.0f, 0.0f},
    };
    std::vector<Face3> expectedFaces3{};
    
    geometry.deleteVertices(vertexSelection3);
    EXPECT_TRUE(geometry.getPositions() == expectedState3);
    EXPECT_TRUE(geometry.getFaces() == expectedFaces3);
}

TEST(GeometryTests, testTransform)
{
    std::vector<Vec3> positions{
        {+1.0f, +1.0f, +0.0f},
        {-1.0f, +1.0f, +0.0f},
        {-1.0f, -1.0f, +0.0f},
        {+1.0f, -1.0f, +0.0f}
    };
    std::vector<Vec3> normals{
        {+0.0f, +0.0f, +1.0f},
        {+0.0f, +0.0f, +1.0f},
        {+0.0f, +0.0f, +1.0f},
        {+0.0f, +0.0f, +1.0f}
    };
    
    // matrix that first does counter-clockwise rotation about the x-axis, and then translates by (7, 8, 9)
    math::Mat3x4 m = {
        +1.0f, +0.0f, +0.0f, +7.0f,
        +0.0f, +0.0f, -1.0f, +8.0f,
        +0.0f, +1.0f, +0.0f, +9.0f
    };
    
    Geometry tri(positions, normals);
    
    tri.transform(m);
    
    positions = tri.getPositions();
    normals = tri.getNormals();
    
    EXPECT_NEAR(positions[0].x, +1.0f + 7.0f, FLT_EPSILON);
    EXPECT_NEAR(positions[0].y, +0.0f + 8.0f, FLT_EPSILON);
    EXPECT_NEAR(positions[0].z, +1.0f + 9.0f, FLT_EPSILON);
    
    EXPECT_NEAR(positions[1].x, -1.0f + 7.0f, FLT_EPSILON);
    EXPECT_NEAR(positions[1].y, +0.0f + 8.0f, FLT_EPSILON);
    EXPECT_NEAR(positions[1].z, +1.0f + 9.0f, FLT_EPSILON);
    
    EXPECT_NEAR(positions[2].x, -1.0f + 7.0f, FLT_EPSILON);
    EXPECT_NEAR(positions[2].y, +0.0f + 8.0f, FLT_EPSILON);
    EXPECT_NEAR(positions[2].z, -1.0f + 9.0f, FLT_EPSILON);
    
    EXPECT_NEAR(positions[3].x, +1.0f + 7.0f, FLT_EPSILON);
    EXPECT_NEAR(positions[3].y, +0.0f + 8.0f, FLT_EPSILON);
    EXPECT_NEAR(positions[3].z, -1.0f + 9.0f, FLT_EPSILON);
    
    EXPECT_NEAR(normals[0].x, +0.0f, FLT_EPSILON);
    EXPECT_NEAR(normals[0].y, -1.0f, FLT_EPSILON);
    EXPECT_NEAR(normals[0].z, +0.0f, FLT_EPSILON);
    
    EXPECT_NEAR(normals[1].x, +0.0f, FLT_EPSILON);
    EXPECT_NEAR(normals[1].y, -1.0f, FLT_EPSILON);
    EXPECT_NEAR(normals[1].z, +0.0f, FLT_EPSILON);
    
    EXPECT_NEAR(normals[2].x, +0.0f, FLT_EPSILON);
    EXPECT_NEAR(normals[2].y, -1.0f, FLT_EPSILON);
    EXPECT_NEAR(normals[2].z, +0.0f, FLT_EPSILON);
    
    EXPECT_NEAR(normals[3].x, +0.0f, FLT_EPSILON);
    EXPECT_NEAR(normals[3].y, -1.0f, FLT_EPSILON);
    EXPECT_NEAR(normals[3].z, +0.0f, FLT_EPSILON);
}

TEST(GeometryTests, testCopy)
{
    std::vector<Vec3> positions{
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {0.0f, 2.0f, 0.0f},
    };
    
    std::vector<Vec3> normals{
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
    };
    
    std::vector<Vec3> colors{
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
    };
    
    
    std::vector<Face3> faces{
        {0, 1, 2}
    };
    
    Geometry tri(positions, normals, colors, faces);
    
    //Geometry copy(std::vector<Vec3>{0,0,0});
    Geometry copy;
    copy.copy(tri);
    
    EXPECT_TRUE(checkVectorsEqual(copy.getPositions(), std::vector<Vec3>({
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {0.0f, 2.0f, 0.0f}
    })));
    
    EXPECT_TRUE(checkVectorsEqual(copy.getNormals(), std::vector<Vec3>({
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
    })));
    
    EXPECT_TRUE(checkVectorsEqual(copy.getColors(), std::vector<Vec3>({
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
    })));
    
    EXPECT_TRUE(checkVectorsEqual(copy.getFaces(), std::vector<Face3>({{
        0, 1, 2
    }})));
}

TEST(GeometryTests, testPositionsMutation)
{
    Geometry geo(
                 std::vector<Vec3>({{1.0f, 2.0f, 3.0f}, {4.0f, 5.0f, 6.0f}}),
                 std::vector<Vec3>({{0.3f, 0.4f, 0.5f}, {0.4f, 0.5f, 0.3f}}),
                 std::vector<Vec3>({{0.0f, 0.5f, 1.0f}, {1.0f, 0.0f, 0.5f}})
                 );
    
    geo.mutatePositionsWithFunction([](int index, Vec3 position, Vec3 normal, Vec3 color) {
        return Vec3 {position.x + 1.0f, position.y + 2.0f, position.z + 3.0f};
    });
    
    EXPECT_TRUE(geo.getPositions() == std::vector<Vec3>({{2.0f, 4.0f, 6.0f}, {5.0f, 7.0f, 9.0f}}));
}

TEST(GeometryTests, testPositionsMutationWithSelection)
{
    Geometry geo(
                 std::vector<Vec3>({{1.0f, 2.0f, 3.0f}, {4.0f, 5.0f, 6.0f}}),
                 std::vector<Vec3>({{0.3f, 0.4f, 0.5f}, {0.4f, 0.5f, 0.3f}}),
                 std::vector<Vec3>({{0.0f, 0.5f, 1.0f}, {1.0f, 0.0f, 0.5f}})
                 );
    
    geo.mutatePositionsWithFunction([](int index, Vec3 position, Vec3 normal, Vec3 color) {
        return Vec3 {position.x + 1.0f, position.y + 2.0f, position.z + 3.0f};
    }, VertexSelection(2, {1}));
    
    EXPECT_TRUE(geo.getPositions() == std::vector<Vec3>({{1.0f, 2.0f, 3.0f}, {5.0f, 7.0f, 9.0f}}));
}

TEST(GeometryTests, testNormalsMutation)
{
    Geometry geo(
                 std::vector<Vec3>({{1.0f, 2.0f, 3.0f}, {4.0f, 5.0f, 6.0f}}),
                 std::vector<Vec3>({{0.3f, 0.4f, 0.5f}, {0.4f, 0.5f, 0.3f}}),
                 std::vector<Vec3>({{0.0f, 0.5f, 1.0f}, {1.0f, 0.0f, 0.5f}})
                 );
    
    geo.mutateNormalsWithFunction([](int index, Vec3 position, Vec3 normal, Vec3 color) {
        return Vec3 {normal.z, normal.x, normal.y};
    });
    
    EXPECT_TRUE(geo.getNormals() == std::vector<Vec3>({{0.5f, 0.3f, 0.4f}, {0.3f, 0.4f, 0.5f}}));
}

TEST(GeometryTests, testNormalsMutationWithSelection)
{
    Geometry geo(
                 std::vector<Vec3>({{1.0f, 2.0f, 3.0f}, {4.0f, 5.0f, 6.0f}}),
                 std::vector<Vec3>({{0.3f, 0.4f, 0.5f}, {0.4f, 0.5f, 0.3f}}),
                 std::vector<Vec3>({{0.0f, 0.5f, 1.0f}, {1.0f, 0.0f, 0.5f}})
                 );
    
    geo.mutateNormalsWithFunction([](int index, Vec3 position, Vec3 normal, Vec3 color) {
        return Vec3 {normal.z, normal.x, normal.y};
    }, VertexSelection(2, {1}));
    
    EXPECT_TRUE(geo.getNormals() == std::vector<Vec3>({{0.3f, 0.4f, 0.5f}, {0.3f, 0.4f, 0.5f}}));
}

TEST(GeometryTests, testColorsMutation)
{
    Geometry geo(
                 std::vector<Vec3>({{1.0f, 2.0f, 3.0f}, {4.0f, 5.0f, 6.0f}}),
                 std::vector<Vec3>({{0.3f, 0.4f, 0.5f}, {0.4f, 0.5f, 0.3f}}),
                 std::vector<Vec3>({{0.0f, 0.5f, 1.0f}, {1.0f, 0.0f, 0.5f}})
                 );
    
    geo.mutateColorsWithFunction([](int index, Vec3 position, Vec3 normal, Vec3 color) {
        return normal;
    });
    
    EXPECT_TRUE(geo.getColors() == std::vector<Vec3>({{0.3f, 0.4f, 0.5f}, {0.4f, 0.5f, 0.3f}}));
}

TEST(GeometryTests, testColorsMutationWithSelection)
{
    Geometry geo(
                 std::vector<Vec3>({{1.0f, 2.0f, 3.0f}, {4.0f, 5.0f, 6.0f}}),
                 std::vector<Vec3>({{0.3f, 0.4f, 0.5f}, {0.4f, 0.5f, 0.3f}}),
                 std::vector<Vec3>({{0.0f, 0.5f, 1.0f}, {1.0f, 0.0f, 0.5f}})
                 );
    
    geo.mutateColorsWithFunction([](int index, Vec3 position, Vec3 normal, Vec3 color) {
        return normal;
    }, VertexSelection(2, {1}));
    
    EXPECT_TRUE(geo.getColors() == std::vector<Vec3>({{0.0f, 0.5f, 1.0f}, {0.4f, 0.5f, 0.3f}}));
}

TEST(GeometryTests, testRayTrace)
{
    std::vector<Vec3> positions{
        {1.0f, 1.0f, +0.0f},
        {4.0f, 1.0f, +0.0f},
        {1.0f, 8.0f, +0.0f},
        
        {1.0f, 4.0f, -1.0f},
        {5.0f, 4.0f, -1.0f},
        {1.0f, 7.0f, -1.0f},
    };
    
    std::vector<Face3> faces{
        {0, 1, 2},
        {3, 4, 5},
    };
    
    Geometry tri(positions, std::vector<Vec3>(), std::vector<Vec3>(), faces);
    
    {
        Vec3 rayOrigin{+2, +5, +2};
        Vec3 rayDirection{+0,+0,-1};
        
        standard_cyborg::sc3d::RayTraceResult result = tri.rayTrace(rayOrigin, rayDirection);
        
        EXPECT_NEAR(result.t, 2.0f, FLT_EPSILON);
        EXPECT_EQ(result.index, 0);
    }
    
    {
        Vec3 rayOrigin{+3.001, +5, +2};
        Vec3 rayDirection{+0,+0,-1};
        
        standard_cyborg::sc3d::RayTraceResult result = tri.rayTrace(rayOrigin, rayDirection);
        
        EXPECT_NEAR(result.t, 3.0f, FLT_EPSILON);
        EXPECT_EQ(result.index, 1);
    }
    
    
    tri.transform({
        +1.0f, +0.0f, +0.0f, +7.0f,
        +0.0f, +1.0f, +0.0f, +0.0f,
        +0.0f, +0.0f, +1.0f, +0.0f
    });
    
    {
        Vec3 rayOrigin{+3.001 + 7.0, +5, +2};
        Vec3 rayDirection{+0,+0,-1};
        
        standard_cyborg::sc3d::RayTraceResult result = tri.rayTrace(rayOrigin, rayDirection);
        
        EXPECT_NEAR(result.t, 3.0f, FLT_EPSILON);
        EXPECT_EQ(result.index, 1);
    }
}
