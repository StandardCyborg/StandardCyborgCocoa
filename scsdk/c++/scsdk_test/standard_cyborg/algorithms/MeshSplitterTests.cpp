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

#include "standard_cyborg/algorithms/MeshSplitter.hpp"

#include "standard_cyborg/sc3d/Geometry.hpp"

using standard_cyborg::math::Vec3;
using standard_cyborg::sc3d::Geometry;
using standard_cyborg::sc3d::Face3;

TEST(MeshSplitterTests, testMeshSplitter) {
    
    std::vector<Vec3> positions {
        {1.0f, 1.0f, 0.0f},
        {3.0f, 1.0f, 0.0f},
        {1.0f, 3.0f, 0.0f},
        
        {1.0f, 1.0f, 3.0f},
        {3.0f, 1.0f, 3.0f},
        {1.0f, 3.0f, 3.0f},
        
        {1.0f, 1.0f, 10.0f},
        {3.0f, 1.0f, 10.0f},
        {1.0f, 3.0f, 10.0f},
        {3.0f, 3.0f, 10.0f},
    };
    
    std::vector<Vec3> normals {
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
    };
    
    std::vector<Vec3> colors {
        {0.0f, 0.2f, 0.0f},
        {0.0f, 0.2f, 0.0f},
        {0.0f, 0.2f, 0.0f},
        
        {0.0f, 0.2f, 0.0f},
        {0.0f, 0.2f, 0.0f},
        {0.0f, 0.2f, 0.0f},
        
        {0.0f, 0.2f, 0.0f},
        {0.0f, 0.2f, 0.0f},
        {0.0f, 0.2f, 0.0f},
        {0.0f, 0.2f, 0.0f},
    };
    
    std::vector<Face3> faces {
        { 0, 1, 2 },
        { 3, 4, 5 },
        
        { 6, 7, 8 },
        { 7, 9, 8 },
    };
    
    Geometry geometry0(positions, normals, colors, faces);
    
    auto result = standard_cyborg::algorithms::splitMeshIntoPieces(geometry0);
    
    {
        auto geo0 = result[0];
        
        std::vector<Vec3> pos = {
            {1.0f, 1.0f, 0.0f},
            {3.0f, 1.0f, 0.0f},
            {1.0f, 3.0f, 0.0f}};
        
        std::vector<Vec3> colors {
            {0.0f, 0.2f, 0.0f},
            {0.0f, 0.2f, 0.0f},
            {0.0f, 0.2f, 0.0f},
        };
        
        std::vector<Vec3> normals {
            {0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 1.0f},
        };
        std::vector<Face3> faces {
            { 0, 1, 2 },
        };
        
        EXPECT_TRUE(geo0->getPositions() == pos);
        EXPECT_TRUE(geo0->getNormals() == normals);
        EXPECT_TRUE(geo0->getColors() == colors);
        EXPECT_TRUE(geo0->getFaces() == faces);
    }
    
    
    {
        auto geo0 = result[1];
        
        std::vector<Vec3> pos = {
            {1.0f, 1.0f, 3.0f},
            {3.0f, 1.0f, 3.0f},
            {1.0f, 3.0f, 3.0f}};
        
        std::vector<Vec3> colors {
            {0.0f, 0.2f, 0.0f},
            {0.0f, 0.2f, 0.0f},
            {0.0f, 0.2f, 0.0f},
        };
        
        std::vector<Vec3> normals {
            {0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 1.0f},
        };
        
        std::vector<Face3> faces {
            { 0, 1, 2 },
        };
        
        EXPECT_TRUE(geo0->getPositions() == pos);
        EXPECT_TRUE(geo0->getNormals() == normals);
        EXPECT_TRUE(geo0->getColors() == colors);
        EXPECT_TRUE(geo0->getFaces() == faces);
    }
    
    {
        auto geo0 = result[2];
        
        std::vector<Vec3> pos = {
            {1.0f, 1.0f, 10.0f},
            {3.0f, 1.0f, 10.0f},
            {1.0f, 3.0f, 10.0f},
            {3.0f, 3.0f, 10.0f}
        };
        
        std::vector<Vec3> colors {
            {0.0f, 0.2f, 0.0f},
            {0.0f, 0.2f, 0.0f},
            {0.0f, 0.2f, 0.0f},
            {0.0f, 0.2f, 0.0f},
        };
        
        std::vector<Vec3> normals {
            {0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 1.0f},
        };
        
        std::vector<Face3> faces {
            { 0, 1, 2 },
            { 1, 3, 2 },
        };
        
        EXPECT_TRUE(geo0->getPositions() == pos);
        EXPECT_TRUE(geo0->getNormals() == normals);
        EXPECT_TRUE(geo0->getColors() == colors);
        EXPECT_TRUE(geo0->getFaces() == faces);
    }
}


TEST(MeshSplitterTests, testMeshSplitterEmptyMesh) {
    
    {
        Geometry geometry0(std::vector<Vec3>({}));
        EXPECT_EQ(standard_cyborg::algorithms::splitMeshIntoPieces(geometry0).size(), 0);
    }
    
    {
        Geometry geometry0(std::vector<Vec3>({1.0f, 2.0f, 3.0f}), std::vector<Face3>({}));
        EXPECT_EQ(standard_cyborg::algorithms::splitMeshIntoPieces(geometry0).size(), 0);
    }
    
    {
        Geometry geometry0(std::vector<Vec3>({}), std::vector<Face3>({Face3{1, 2, 3}}));
        EXPECT_EQ(standard_cyborg::algorithms::splitMeshIntoPieces(geometry0).size(), 0);
    }
}
