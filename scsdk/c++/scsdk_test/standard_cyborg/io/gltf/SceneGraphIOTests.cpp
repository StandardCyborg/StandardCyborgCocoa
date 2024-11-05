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

#include <fstream>

#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/sc3d/Landmark.hpp"
#include "standard_cyborg/sc3d/Polyline.hpp"
#include "standard_cyborg/scene_graph/SceneGraph.hpp"
#include "standard_cyborg/io/gltf/SceneGraphFileIO_GLTF.hpp"

using namespace standard_cyborg::sc3d;
using namespace standard_cyborg::scene_graph;
namespace math = standard_cyborg::math;

std::shared_ptr<Node> getTestLandmarkNode(const std::string &name, const math::Vec3 &pos)
{
    std::shared_ptr<LandmarkNode> node0(new LandmarkNode());
    node0->setTransform(math::Mat3x4{
        1.0f, 2.0f, 0.0f, 4.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 4.0f});
    node0->setName("test0");
    node0->setLandmark({name, pos});
    
    std::shared_ptr<Node> node1(new Node());
    
    node1->setTransform(math::Mat3x4{
        1.0f, 2.0f, 0.0f, 4.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 4.0f});
    node1->setName("test1");
    
    std::shared_ptr<Node> parentNode(new Node());
    
    parentNode->appendChildren({node0, node1});
    parentNode->setTransform(math::Mat3x4{
        1.0f, 2.0f, 0.0f, 4.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 4.0f});
    parentNode->setName("node2");
    
    return parentNode;
}


std::string stringFromPath(const std::string& path)
{
    std::string str;
    std::ifstream inputStream(path);
    
    inputStream.seekg(0, std::ios::end);
    str.reserve(inputStream.tellg());
    inputStream.seekg(0, std::ios::beg);
    
    str.assign((std::istreambuf_iterator<char>(inputStream)),
               std::istreambuf_iterator<char>());
    
    return str;
}

TEST(SceneGraphIOTests, testSceneGraphsSingleNode)
{
    std::vector<math::Vec3> originalPositions = {{1.0f, 2.0f, 3.0f}};
    std::vector<math::Vec3> originalNormals = {{1.0f, 0.0f, 0.0f}};
    std::vector<math::Vec3> originalColors = {{0.0f, 1.0f, 0.0f}};
    Geometry originalGeometry(originalPositions, originalNormals, originalColors);
    
    math::Mat3x4 originalTransform{
        1.0f, 2.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f};
    std::string originalName = "node0";
    
    std::shared_ptr<GeometryNode> originalNode(new GeometryNode(originalName, originalGeometry));
    originalNode->setName(originalName);
    originalNode->setTransform(originalTransform);
    
    std::string gltfPath = "/tmp/test.gltf";
    
    EXPECT_TRUE(standard_cyborg::io::gltf::WriteSceneGraphToGltf(originalNode, gltfPath));
    
    std::string gltfSource = stringFromPath(gltfPath);
    std::shared_ptr<Node> deserializedNode = standard_cyborg::io::gltf::ReadSceneGraphFromGltf(gltfSource)[0];
    GeometryNode *deserializedGeometryNode = deserializedNode->asGeometryNode();
    const Geometry &deserializedGeometry = deserializedGeometryNode->getGeometry();
    
    EXPECT_TRUE(deserializedGeometry.getPositions() == originalPositions);
    EXPECT_EQ(deserializedGeometry.getNormals(), originalNormals);
    EXPECT_EQ(deserializedGeometry.getColors(), originalColors);
    EXPECT_EQ(deserializedNode->getName(), originalName);
    EXPECT_EQ(math::Mat3x4::fromTransform(deserializedNode->getTransform()), originalTransform);
    EXPECT_EQ(deserializedNode->dataIsResolved(), true);
    EXPECT_EQ(deserializedNode->numChildren(), 0);
}


TEST(SceneGraphIOTests, testSceneGraphRootWithChildren)
{
    std::shared_ptr<Node> root(new Node());
    
    root->appendChildren({std::shared_ptr<Node>(new GeometryNode()),
        std::shared_ptr<Node>(new GeometryNode())});
    root->setName("parent");
    
    std::string gltfPath = "/tmp/test.gltf";
    standard_cyborg::io::gltf::WriteSceneGraphToGltf({root}, gltfPath);
    
    std::string gltfSource = stringFromPath(gltfPath);
    std::shared_ptr<Node> deserializedRoot = standard_cyborg::io::gltf::ReadSceneGraphFromGltf(gltfSource)[0];
    
    EXPECT_EQ(deserializedRoot->numChildren(), 2);
    EXPECT_EQ(math::Mat3x4::fromTransform(deserializedRoot->getTransform()), math::Mat3x4{});
    EXPECT_EQ(deserializedRoot->getName(), std::string("parent"));
}

TEST(SceneGraphIOTests, testGeometryNodeBase64)
{
    Geometry geometry;
    geometry.setPositions({{1, 2, 3}, {2, 3, 4}});
    geometry.setNormals({{10, 20, 30}, {20, 30, 40}});
    geometry.setColors({{100, 200, 300}, {200, 300, 400}});
    geometry.setFaces({{1, 2, 3}, {2, 3, 4}});
    geometry.setTexCoords({{5, 6}, {7, 8}});
    
    std::shared_ptr<GeometryNode> geometryNode = std::make_shared<GeometryNode>("test0", geometry);
    
    std::string gltfPath = "/tmp/test.gltf";
    EXPECT_TRUE(standard_cyborg::io::gltf::WriteSceneGraphToGltf(geometryNode, gltfPath));
    
    std::string gltfSource = stringFromPath(gltfPath);
    std::shared_ptr<Node> deserializedNode = standard_cyborg::io::gltf::ReadSceneGraphFromGltf(gltfSource)[0];
    
    EXPECT_TRUE(deserializedNode->equals(*geometryNode));
}


TEST(SceneGraphIOTests, testGeometryNodeTexture)
{
    Geometry geometry;
    
    ColorImage texture2{1, 1, {math::Vec4{0.0f, 1.0f, 0.0f, 1.0f}}};
    geometry.setTexture(texture2);
    
    std::shared_ptr<GeometryNode> geometryNode = std::make_shared<GeometryNode>("test0", geometry);
    
    std::string gltfPath = "/tmp/test.gltf";
    EXPECT_TRUE(standard_cyborg::io::gltf::WriteSceneGraphToGltf(geometryNode, gltfPath));
    
    std::string gltfSource = stringFromPath(gltfPath);
    std::shared_ptr<Node> deserializedNode = standard_cyborg::io::gltf::ReadSceneGraphFromGltf(gltfSource)[0];
    
    EXPECT_TRUE(deserializedNode->asGeometryNode()->getGeometry().hasTexture());
    EXPECT_TRUE(texture2 == deserializedNode->asGeometryNode()->getGeometry().getTexture());
}

TEST(SceneGraphIOTests, testNodeMaterial)
{
    std::shared_ptr<Node> node = std::make_shared<Node>("test0");
    
    math::Vec3 objectColor(1, 0, 1);
    MaterialModel materialModel = MaterialModel::Phong;
    
    node->getMaterial().objectColor = objectColor;
    node->getMaterial().materialModel = materialModel;
    
    
    std::string gltfPath = "/tmp/test.gltf";
    EXPECT_TRUE(standard_cyborg::io::gltf::WriteSceneGraphToGltf(node, gltfPath));
    
    std::string gltfSource = stringFromPath(gltfPath);
    std::shared_ptr<Node> deserializedNode = standard_cyborg::io::gltf::ReadSceneGraphFromGltf(gltfSource)[0];
    
    EXPECT_TRUE(objectColor == deserializedNode->getMaterial().objectColor);
    EXPECT_TRUE(materialModel == deserializedNode->getMaterial().materialModel);
}

TEST(SceneGraphIOTests, testPolylineGLTFIO)
{
    Polyline polyline;
    polyline.setPositions(std::vector<math::Vec3>{math::Vec3{1.0, 0.0, 0.0}, math::Vec3{0.0, 1.0, 0.0}, math::Vec3{0.0, 0.0, 1.0}});
    
    std::shared_ptr<PolylineNode> polylineNode = std::make_shared<PolylineNode>("Polyline", polyline);
    
    std::string gltfPath = "/tmp/test.gltf";
    EXPECT_TRUE(standard_cyborg::io::gltf::WriteSceneGraphToGltf(polylineNode, gltfPath));
    
    std::string gltfSource = stringFromPath(gltfPath);
    std::shared_ptr<Node> deserializedNode = standard_cyborg::io::gltf::ReadSceneGraphFromGltf(gltfSource)[0];
    
    EXPECT_TRUE(deserializedNode->isPolylineNode());
    EXPECT_EQ(deserializedNode->numChildren(), 0);
    EXPECT_EQ(deserializedNode->asPolylineNode()->getPolyline().getPositions(), polyline.getPositions());
    EXPECT_TRUE(deserializedNode->equals(*polylineNode));
}

TEST(SceneGraphIOTests, testSceneGraphsEqualsLandmarkNode)
{
    {
        std::shared_ptr<Node> graph0(getTestLandmarkNode("nose", math::Vec3{1.0f, 2.0f, 3.0f}));
        std::shared_ptr<Node> graph1(getTestLandmarkNode("nose", math::Vec3{1.0f, 2.0f, 3.0f}));
        EXPECT_TRUE(graph0->equals(*graph1));
    }
    
    {
        std::shared_ptr<Node> graph0(getTestLandmarkNode("nose", math::Vec3{1.0f, 2.0f, 3.0f}));
        std::shared_ptr<Node> graph1(getTestLandmarkNode("nosee", math::Vec3{1.0f, 2.0f, 3.0f}));
        EXPECT_FALSE(graph0->equals(*graph1));
    }
    
    {
        std::shared_ptr<Node> graph0(getTestLandmarkNode("nose", math::Vec3{1.0f, 2.0f, 3.0f}));
        std::shared_ptr<Node> graph1(getTestLandmarkNode("nose", math::Vec3{1.0f, 8.0f, 3.0f}));
        EXPECT_FALSE(graph0->equals(*graph1));
    }
    
    {
        std::string gltfPath = "/tmp/test.gltf";
        
        Landmark landmark{"nose", {1, 2, 3}};
        std::shared_ptr<LandmarkNode> landmarkNode = std::make_shared<LandmarkNode>("landmark", landmark);
        EXPECT_TRUE(standard_cyborg::io::gltf::WriteSceneGraphToGltf(landmarkNode, gltfPath));
        
        std::string gltfSource = stringFromPath(gltfPath);
        std::shared_ptr<Node> deserializedNode = standard_cyborg::io::gltf::ReadSceneGraphFromGltf(gltfSource)[0];
        
        EXPECT_EQ(deserializedNode->getName(), landmarkNode->getName());
        EXPECT_EQ(deserializedNode->getTransform(), landmarkNode->getTransform());
        EXPECT_EQ(deserializedNode->numChildren(), landmarkNode->numChildren());
        EXPECT_EQ(deserializedNode->asLandmarkNode()->getLandmark().getPosition(), landmarkNode->asLandmarkNode()->getLandmark().getPosition());
        EXPECT_EQ(deserializedNode->asLandmarkNode()->getLandmark().getName(), landmarkNode->asLandmarkNode()->getLandmark().getName());
    }
}

TEST(SceneGraphIOTests, testCyclicGraph)
{
    std::shared_ptr<Node> n0(new Node());
    std::shared_ptr<Node> n1(new Node());
    std::shared_ptr<Node> n2(new Node());
    
    EXPECT_TRUE(n0->appendChild(n1, n0));
    EXPECT_TRUE(n1->appendChild(n2, n0));
    EXPECT_FALSE(n2->appendChild(n0, n0));
}
