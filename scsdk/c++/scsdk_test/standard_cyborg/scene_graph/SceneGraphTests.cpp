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

#include "standard_cyborg/scene_graph/SceneGraph.hpp"
#include "standard_cyborg/math/Vec4.hpp"
#include "standard_cyborg/math/Vec3.hpp"

#include "standard_cyborg/sc3d/Geometry.hpp"

#include "standard_cyborg/sc3d/Polyline.hpp"
#include "standard_cyborg/sc3d/Landmark.hpp"
#include "standard_cyborg/sc3d/Plane.hpp"
#include "standard_cyborg/sc3d/PerspectiveCamera.hpp"


/*
 #include <StandardCyborgData/DebugHelpers.hpp>
 #include <StandardCyborgData/Geometry.hpp>
 #include <StandardCyborgData/Labels.hpp>
 #include <StandardCyborgData/Plane.hpp>
 #include <StandardCyborgData/Polyline.hpp>
 #include <StandardCyborgData/SceneGraph.hpp>
 #include <StandardCyborgData/PerspectiveCamera.hpp>
 */

namespace math = standard_cyborg::math;
using math::Vec4;
using math::Vec3;

using standard_cyborg::scene_graph::Node;

//using standard_cyborg::Node;

//namespace math = standard_cyborg::math;
//using math::Vec4;

/*
 #import <XCTest/XCTest.h>
 #import <iostream>
 
 #include <StandardCyborgData/ColorImage.hpp>
 #include <StandardCyborgData/DebugHelpers.hpp>
 #include <StandardCyborgData/Geometry.hpp>
 #include <StandardCyborgData/Labels.hpp>
 #include <StandardCyborgData/Landmark.hpp>
 #include <StandardCyborgData/Plane.hpp>
 #include <StandardCyborgData/Polyline.hpp>
 #include <StandardCyborgData/SceneGraph.hpp>
 #include <StandardCyborgData/PerspectiveCamera.hpp>
 
 
 using standard_cyborg::Node;
 
 namespace math = standard_cyborg::math;
 using math::Vec3;
 */

TEST(SceneGraphTests, testNodeInstantiation)
{
    Node *node0 = new Node();
    Node *node1 = new Node();
    
    EXPECT_EQ(node0->numChildren(), 0);
    EXPECT_EQ(node1->numChildren(), 0);
    
    delete node0;
    delete node1;
    
    Node *namedNode = new Node("name");
    EXPECT_TRUE(namedNode->getName() == "name");
    delete namedNode;
}

TEST(SceneGraphTests, testVisibility)
{
    Node *n = new Node();
    EXPECT_TRUE(n->isVisible());
    EXPECT_FALSE(n->toggleVisibility());
    EXPECT_FALSE(n->isVisible());
    n->setVisibility(true);
    EXPECT_TRUE(n->isVisible());
    n->setVisibility(false);
    EXPECT_FALSE(n->isVisible());
}

TEST(SceneGraphTests, testSetters)
{
    Node *n = new Node();
    n->setName("foo")
    .setTransform(math::Mat3x4{2, 0, 0, 0,  0, 3, 0, 0,  0, 0, 5, 0})
    .markDataResolved(nullptr)
    .setDataURI("file:./test.ply");
    
    EXPECT_EQ(n->getName(), "foo");
    EXPECT_EQ(math::Mat3x4::fromTransform(n->getTransform()), math::Mat3x4(2, 0, 0, 0,  0, 3, 0, 0,  0, 0, 5, 0));
    EXPECT_EQ(n->getDataURI(), "file:./test.ply");
    EXPECT_TRUE(n->dataIsResolved());
    
    n->markDataUnresolved(nullptr);
    EXPECT_FALSE(n->dataIsResolved());
}

TEST(SceneGraphTests, testGeometryNodeInstantiation)
{
    standard_cyborg::scene_graph::GeometryNode *node = new standard_cyborg::scene_graph::GeometryNode();
    
    EXPECT_EQ(node->numChildren(), 0);
    EXPECT_EQ(node->getGeometry().vertexCount(), 0);
    
    delete node;
}

TEST(SceneGraphTests, testPolylineNodeInstantiation)
{
    standard_cyborg::scene_graph::PolylineNode *node = new standard_cyborg::scene_graph::PolylineNode();
    
    EXPECT_EQ(node->numChildren(), 0);
    EXPECT_EQ(node->getPolyline().vertexCount(), 0);
    
    delete node;
}

TEST(SceneGraphTests, testGeometryNodeInstantiationFromSharedPtr)
{
    std::shared_ptr<standard_cyborg::sc3d::Geometry> geometry(new standard_cyborg::sc3d::Geometry({std::vector<Vec3>{{1, 2, 3}}}));
    std::shared_ptr<standard_cyborg::scene_graph::GeometryNode> node(new standard_cyborg::scene_graph::GeometryNode("foo", geometry));
    EXPECT_EQ(node->getGeometry().vertexCount(), 1);
}

TEST(SceneGraphTests, testPlaneNodeInstantiationFromSharedPtr)
{
    std::shared_ptr<standard_cyborg::sc3d::Plane> plane(new standard_cyborg::sc3d::Plane{{1, 2, 3}, {3, 4, 5}});
    std::shared_ptr<standard_cyborg::scene_graph::PlaneNode> node(new standard_cyborg::scene_graph::PlaneNode("foo", plane));
    EXPECT_TRUE(node->getPlane() == standard_cyborg::sc3d::Plane({{1, 2, 3}, {3, 4, 5}}));
}

TEST(SceneGraphTests, testPolylineNodeInstantiationFromSharedPtr)
{
    std::shared_ptr<standard_cyborg::sc3d::Polyline> polyline(new standard_cyborg::sc3d::Polyline({std::vector<Vec3>{{1, 2, 3}}}));
    std::shared_ptr<standard_cyborg::scene_graph::PolylineNode> node(new standard_cyborg::scene_graph::PolylineNode("foo", polyline));
    EXPECT_EQ(node->getPolyline().vertexCount(), 1);
}

TEST(SceneGraphTests, testImageNodeInstantiationFromSharedPtr)
{
    std::shared_ptr<standard_cyborg::sc3d::ColorImage> image(new standard_cyborg::sc3d::ColorImage(4, 3));
    std::shared_ptr<standard_cyborg::scene_graph::ColorImageNode> node(new standard_cyborg::scene_graph::ColorImageNode("foo", image));
    EXPECT_EQ(node->getColorImage().getWidth(), 4);
    EXPECT_EQ(node->getColorImage().getHeight(), 3);
}

TEST(SceneGraphTests, testLandmarkNodeInstantiationFromSharedPtr)
{
    std::shared_ptr<standard_cyborg::sc3d::Landmark> landmark(new standard_cyborg::sc3d::Landmark({"foo", {1, 2, 3}}));
    std::shared_ptr<standard_cyborg::scene_graph::LandmarkNode> node(new standard_cyborg::scene_graph::LandmarkNode("foo", landmark));
    EXPECT_TRUE(node->getLandmark() == standard_cyborg::sc3d::Landmark({"foo", {1, 2, 3}}));
}

TEST(SceneGraphTests, testGeometryNodeInstantiationFromValue)
{
    standard_cyborg::sc3d::Geometry geometry({std::vector<Vec3>{{1, 2, 3}}});
    std::shared_ptr<standard_cyborg::scene_graph::GeometryNode> node(new standard_cyborg::scene_graph::GeometryNode("foo", geometry));
    EXPECT_EQ(node->getGeometry().vertexCount(), 1);
}

TEST(SceneGraphTests, testPlaneNodeInstantiationFromValue)
{
    standard_cyborg::sc3d::Plane plane({{1, 2, 3}, {3, 4, 5}});
    std::shared_ptr<standard_cyborg::scene_graph::PlaneNode> node(new standard_cyborg::scene_graph::PlaneNode("foo", plane));
    EXPECT_TRUE(node->getPlane() == standard_cyborg::sc3d::Plane({{1, 2, 3}, {3, 4, 5}}));
}

TEST(SceneGraphTests, testPolylineNodeInstantiationFromValue)
{
    standard_cyborg::sc3d::Polyline polyline(std::vector<Vec3>{{1, 2, 3}});
    std::shared_ptr<standard_cyborg::scene_graph::PolylineNode> node(new standard_cyborg::scene_graph::PolylineNode("foo", polyline));
    EXPECT_EQ(node->getPolyline().vertexCount(), 1);
}

TEST(SceneGraphTests, testImageNodeInstantiationFromValue)
{
    standard_cyborg::sc3d::ColorImage image(4, 3);
    std::shared_ptr<standard_cyborg::scene_graph::ColorImageNode> node(new standard_cyborg::scene_graph::ColorImageNode("foo", image));
    EXPECT_EQ(node->getColorImage().getWidth(), 4);
    EXPECT_EQ(node->getColorImage().getHeight(), 3);
}

TEST(SceneGraphTests, testLandmarkNodeInstantiationFromValue)
{
    standard_cyborg::sc3d::Landmark landmark({"foo", {1, 2, 3}});
    std::shared_ptr<standard_cyborg::scene_graph::LandmarkNode> node(new standard_cyborg::scene_graph::LandmarkNode("foo", landmark));
    EXPECT_TRUE(node->getLandmark() == standard_cyborg::sc3d::Landmark({"foo", {1, 2, 3}}));
}

TEST(SceneGraphTests, testNodeChildren)
{
    std::shared_ptr<Node> root(new Node("root"));
    std::shared_ptr<Node> child0(new Node("child0"));
    std::shared_ptr<standard_cyborg::scene_graph::LandmarkNode> child1(new standard_cyborg::scene_graph::LandmarkNode("child1"));
    std::shared_ptr<Node> child00(new Node("child0.0"));
    
    EXPECT_TRUE(root->appendChild(child0, root));
    EXPECT_TRUE(root->appendChild(child1));
    EXPECT_TRUE(child0->appendChild(child00));
    EXPECT_EQ(root->numChildren(), 2);
    
    // Test range-based for loop
    for (std::shared_ptr<Node> child : root) {
        if (child == child0) {
            EXPECT_TRUE(child->getName() == "child0");
        } else if (child == child1) {
            EXPECT_TRUE(child->getName() == "child1");
        } else {
            FAIL() << "Unexpected child name"  << child->getName();
        }
    }
    
    EXPECT_EQ(root->firstChildNamed("root"), nullptr);
    EXPECT_EQ(root->firstChildNamed("child0"), child0);
    EXPECT_EQ(root->firstChildNamed("child1"), child1);
    EXPECT_EQ(root->firstChildNamed("banana"), nullptr);
    EXPECT_EQ(root->firstChildNamed("child0.0"), child00);
    EXPECT_EQ(root->indexOfChild(child1), 1);
    EXPECT_EQ(root->indexOfChild(root), -1);
    
    // Test template-based firstChildNamed
    std::shared_ptr<standard_cyborg::scene_graph::LandmarkNode> landmarkChild = root->firstChildNamed<standard_cyborg::scene_graph::LandmarkNode>("child1");
    EXPECT_TRUE(landmarkChild != nullptr);
    
    // Filters by node type, if specified
    std::shared_ptr<standard_cyborg::scene_graph::LandmarkNode> foundChild1AsLandmark = root->firstChildNamed<standard_cyborg::scene_graph::LandmarkNode>("child1");
    EXPECT_EQ(foundChild1AsLandmark, child1);
    
    // Does not find if wrong type is specified
    std::shared_ptr<standard_cyborg::scene_graph::GeometryNode> foundChild1AsGeometry = root->firstChildNamed<standard_cyborg::scene_graph::GeometryNode>("child1");
    EXPECT_EQ(foundChild1AsGeometry, nullptr);
    
    EXPECT_TRUE(root->removeChild(child0));
    EXPECT_EQ(root->indexOfChild(child1), 0);
    EXPECT_TRUE(root->removeChild(child1));
    EXPECT_EQ(root->numChildren(), 0);
    EXPECT_FALSE(root->removeChild(root));
    
    root->appendChild(child0);
    root->appendChild(child1);
    EXPECT_EQ(root->numChildren(), 2);
    root->removeAllChildren();
    EXPECT_EQ(root->numChildren(), 0);
}


TEST(SceneGraphTests, testFindNodeWithId) {
    std::shared_ptr<Node> root(new Node());
    std::shared_ptr<Node> child0(new Node());
    std::shared_ptr<Node> child1(new Node());
    std::shared_ptr<Node> child00(new Node());
    std::shared_ptr<Node> child01(new Node());
    
    root->appendChildren({child0, child1});
    child0->appendChildren({child00, child01});
    
    //root->findNodeWithId(child0->getId());
}

TEST(SceneGraphTests, testPolylineNodeEquality)
{
    standard_cyborg::scene_graph::PolylineNode *node0 = new standard_cyborg::scene_graph::PolylineNode("node0");
    standard_cyborg::scene_graph::PolylineNode *node1 = new standard_cyborg::scene_graph::PolylineNode("node0");
    node0->getPolyline().setPositions(std::vector<Vec3>{Vec3{1.0, 0.0, 0.0}, Vec3{0.0, 1.0, 0.0}, Vec3{0.0, 0.0, 1.0}});
    node1->getPolyline().setPositions(std::vector<Vec3>{Vec3{1.0, 0.0, 0.0}, Vec3{0.0, 1.0, 0.0}, Vec3{0.0, 0.0, 1.0}});
    
    EXPECT_TRUE(node0->equals(*node1));
    
    delete node0;
    delete node1;
}


TEST(SceneGraphTests, testPolylineNodeInequality)
{
    standard_cyborg::scene_graph::PolylineNode *node0 = new standard_cyborg::scene_graph::PolylineNode();
    standard_cyborg::scene_graph::PolylineNode *node1 = new standard_cyborg::scene_graph::PolylineNode();
    node0->getPolyline().setPositions(std::vector<Vec3>{Vec3{0.0, 0.0, 0.0}, Vec3{0.0, 1.0, 0.0}, Vec3{0.0, 0.0, 1.0}});
    node1->getPolyline().setPositions(std::vector<Vec3>{Vec3{1.0, 0.0, 0.0}, Vec3{0.0, 1.0, 0.0}, Vec3{0.0, 0.0, 1.0}});
    
    EXPECT_FALSE(node0->equals(*node1));
    
    delete node0;
    delete node1;
}


TEST(SceneGraphTests, testRemoveNode)
{
    std::shared_ptr<Node> g0;
    {
        g0.reset(new Node("g0"));
        
        std::shared_ptr<Node> n1(new Node("n1"));
        std::shared_ptr<Node> n2(new Node("n2"));
        std::shared_ptr<Node> n3(new Node("n3"));
        std::shared_ptr<Node> n4(new Node("n4"));
        std::shared_ptr<Node> n5(new Node("n5"));
        
        EXPECT_TRUE(g0->appendChild(n1, g0));
        EXPECT_TRUE(g0->appendChild(n2, g0));
        
        EXPECT_TRUE(n2->appendChild(n3, g0));
        EXPECT_TRUE(n2->appendChild(n4, g0));
        
        EXPECT_TRUE(n4->appendChild(n5, g0));
        
        EXPECT_TRUE(g0->removeChild(n2));
    }
    
    std::shared_ptr<Node> g1;
    {
        g1.reset(new Node("g0"));
        std::shared_ptr<Node> n1(new Node("n1"));
        
        EXPECT_TRUE(g1->appendChild(n1, g1));
    }
    
    EXPECT_TRUE(g0.get() != nullptr);
    EXPECT_TRUE(g1.get() != nullptr);
    EXPECT_TRUE(g1->equals(*g0));
    
    std::shared_ptr<Node> g2;
    {
        g2.reset(new Node("g0"));
        
        EXPECT_FALSE(Node::remove(g2, g2));
    }
}


TEST(SceneGraphTests, testCyclicGraph)
{
    {
        std::shared_ptr<Node> n0(new Node());
        std::shared_ptr<Node> n1(new Node());
        std::shared_ptr<Node> n2(new Node());
        std::shared_ptr<Node> n3(new Node());
        
        EXPECT_TRUE(n0->appendChild(n1, n0));
        EXPECT_TRUE(n0->appendChild(n2, n0));
        EXPECT_TRUE(n1->appendChild(n3, n0));
        
        EXPECT_FALSE(n2->appendChild(n3, n0)); // n3 already has a parent.
    }
    
    {
        std::shared_ptr<Node> n0(new Node());
        std::shared_ptr<Node> n1(new Node());
        std::shared_ptr<Node> n2(new Node());
        
        EXPECT_TRUE(n0->appendChild(n1, n0));
        EXPECT_TRUE(n1->appendChild(n2, n0));
        EXPECT_FALSE(n1->appendChild(n2, n0));
    }
}

/*
 TEST(SceneGraphTests, testReparentNode
 {
 std::unique_ptr<Node> g0;
 {
 g0.reset(new Node("g0"));
 
 Node* n1 = new Node("n1");
 Node* n2 = new Node("n2");
 Node* n3 = new Node("n3");
 Node* n4 = new Node("n4");
 Node* n5 = new Node("n5");
 
 EXPECT_TRUE(g0->pushChild(n1));
 EXPECT_TRUE(g0->pushChild(n2));
 
 EXPECT_TRUE(n2->pushChild(n3));
 EXPECT_TRUE(n2->pushChild(n4));
 
 EXPECT_TRUE(n4->pushChild(n5));
 
 EXPECT_TRUE(n2->reparent(n1));
 }
 
 std::unique_ptr<Node> g1;
 {
 g1.reset(new Node("g0"));
 
 Node* n1 = new Node("n1");
 Node* n2 = new Node("n2");
 Node* n3 = new Node("n3");
 Node* n4 = new Node("n4");
 Node* n5 = new Node("n5");
 
 EXPECT_TRUE(g1->pushChild(n1));
 
 EXPECT_TRUE(n1->pushChild(n2));
 
 EXPECT_TRUE(n2->pushChild(n3));
 EXPECT_TRUE(n2->pushChild(n4));
 
 EXPECT_TRUE(n4->pushChild(n5));
 }
 
 EXPECT_TRUE(g1->equals(g0.get()));
 }
 */

TEST(SceneGraphTests, testPlaneNode)
{
    standard_cyborg::sc3d::Plane plane;
    plane.normal = {0, 0.3, 0.7};
    plane.position = {1, 2, 3};
    
    standard_cyborg::scene_graph::PlaneNode node;
    node.setPlane(plane);
    
    EXPECT_TRUE(node.getPlane().normal == plane.normal);
    EXPECT_TRUE(node.getPlane().position == plane.position);
}

TEST(SceneGraphTests, testUndoRedoManager)
{
    Node::resetAllocatedResources();
    
    std::vector<std::shared_ptr<Node>> history;
    {
        std::shared_ptr<Node> g0(new Node("n0"));
        std::shared_ptr<Node> n1(new Node("n1"));
        std::shared_ptr<Node> n2(new Node("n2"));
        std::shared_ptr<Node> n3(new Node("n3"));
        std::shared_ptr<Node> na(new Node("na"));
        std::shared_ptr<Node> nb(new Node("nb"));
        std::shared_ptr<Node> nc(new Node("nc"));
        std::shared_ptr<Node> nx(new Node("nx"));
        std::shared_ptr<standard_cyborg::scene_graph::GeometryNode> ny(new standard_cyborg::scene_graph::GeometryNode("ny"));
        
        std::vector<Vec3> originalPositions = {{1.0f, 2.0f, 3.0f}};
        std::vector<Vec3> originalNormals = {{1.0f, 0.0f, 0.0f}};
        std::vector<Vec3> originalColors = {{0.0f, 1.0f, 0.0f}};
        standard_cyborg::sc3d::Geometry geo(originalPositions, originalNormals, originalColors);
        
        ny->getGeometry().copy(geo);
        
        std::shared_ptr<Node> nz(new Node("nz"));
        
        EXPECT_TRUE(g0->appendChild(n1, g0));
        EXPECT_TRUE(g0->appendChild(n2, g0));
        
        EXPECT_TRUE(n1->appendChild(n3, g0));
        
        EXPECT_TRUE(n2->appendChild(na, g0));
        EXPECT_TRUE(n2->appendChild(nb, g0));
        EXPECT_TRUE(n2->appendChild(nc, g0));
        
        EXPECT_TRUE(n3->appendChild(nx, g0));
        EXPECT_TRUE(nx->appendChild(ny, g0));
        EXPECT_TRUE(nx->appendChild(nz, g0));
        
        history.push_back(g0);
        
        EXPECT_EQ(10, Node::getAllocatedResources().size());
        
        history.push_back(Node::mutateNode(n3, history[history.size() - 1],
                                           [](std::shared_ptr<Node> targetNode, std::shared_ptr<Node> rootNode) {
            targetNode->setName("xyz", rootNode);
        }));
        
        EXPECT_EQ(13, Node::getAllocatedResources().size());
        
        history.push_back(Node::mutateNode(nx, history[history.size()-1],
                                           [&](std::shared_ptr<Node> targetNode, std::shared_ptr<Node> rootNode) {
            Node::remove(rootNode, targetNode);
        }));
        
        EXPECT_EQ(16, Node::getAllocatedResources().size());
        
        
        // modify root.
        history.push_back(Node::mutateNode(history[history.size()-1], history[history.size()-1],
                                           [](std::shared_ptr<Node> targetNode, std::shared_ptr<Node> rootNode) {
            targetNode->setName("lol", rootNode);
        }));
        
        EXPECT_EQ(17,  Node::getAllocatedResources().size());
    }
    
    
    std::shared_ptr<Node> g1;
    {
        std::shared_ptr<Node> g1(new Node("n0"));
        std::shared_ptr<Node> n1(new Node("n1"));
        std::shared_ptr<Node> n2(new Node("n2"));
        std::shared_ptr<Node> n3(new Node("n3"));
        std::shared_ptr<Node> na(new Node("na"));
        std::shared_ptr<Node> nb(new Node("nb"));
        std::shared_ptr<Node> nc(new Node("nc"));
        std::shared_ptr<Node> nx(new Node("nx"));
        std::shared_ptr<standard_cyborg::scene_graph::GeometryNode> ny(new standard_cyborg::scene_graph::GeometryNode("ny"));
        
        std::vector<Vec3> originalPositions = {{1.0f, 2.0f, 3.0f}};
        std::vector<Vec3> originalNormals = {{1.0f, 0.0f, 0.0f}};
        std::vector<Vec3> originalColors = {{0.0f, 1.0f, 0.0f}};
        standard_cyborg::sc3d::Geometry geo(originalPositions, originalNormals, originalColors);
        
        ny->getGeometry().copy(geo);
        
        std::shared_ptr<Node> nz(new Node("nz"));
        
        
        EXPECT_TRUE(g1->appendChild(n1, g1));
        EXPECT_TRUE(g1->appendChild(n2, g1));
        
        EXPECT_TRUE(n1->appendChild(n3, g1));
        
        EXPECT_TRUE(n2->appendChild(na, g1));
        EXPECT_TRUE(n2->appendChild(nb, g1));
        EXPECT_TRUE(n2->appendChild(nc, g1));
        
        EXPECT_TRUE(n3->appendChild(nx, g1));
        EXPECT_TRUE(nx->appendChild(ny, g1));
        EXPECT_TRUE(nx->appendChild(nz, g1));
        
        EXPECT_TRUE(g1.get()->equals(*history[0], true));
        
        n3->setName("xyz", g1);
        EXPECT_TRUE(g1.get()->equals(*history[1], true));
        
        Node::remove(g1, nx);
        EXPECT_TRUE(g1.get()->equals(*history[2], true));
        
        g1->setName("lol");
        EXPECT_TRUE(g1.get()->equals(*history[3], true));
    }
    
    history.clear();
    EXPECT_EQ(0, Node::getAllocatedResources().size());
}

TEST(SceneGraphTests, testDeepCopyRecursive)
{
    std::shared_ptr<Node> a(new Node("a"));
    std::shared_ptr<Node> b(new Node("b"));
    std::shared_ptr<Node> c(new Node("c"));
    
    /*
     std::shared_ptr<GeometryNode> ny(new GeometryNode());
     ny->setName("ny");
     */
    
    std::shared_ptr<standard_cyborg::scene_graph::GeometryNode> d(new standard_cyborg::scene_graph::GeometryNode("d"));
    
    std::vector<Vec3> originalPositions = {{1.0f, 2.0f, 3.0f}};
    standard_cyborg::sc3d::Geometry geo (originalPositions);
    d->getGeometry().copy(geo);
    
    EXPECT_TRUE(a->appendChild(b, a));
    EXPECT_TRUE(a->appendChild(c, a));
    EXPECT_TRUE(b->appendChild(d, a));
    
    std::shared_ptr<Node> aCopy = a->deepCopyRecursive();
    
    EXPECT_EQ(a->getName(), aCopy->getName());
    EXPECT_NE(a->getId(), aCopy->getId());
    EXPECT_NE(a.get(), aCopy.get());
    
    EXPECT_EQ(a->getChild(0)->getName(), aCopy->getChild(0)->getName());
    EXPECT_NE(a->getChild(0)->getId(), aCopy->getChild(0)->getId());
    EXPECT_NE(a->getChild(0), aCopy->getChild(0));
    
    EXPECT_EQ(a->getChild(1)->getName(), aCopy->getChild(1)->getName());
    EXPECT_NE(a->getChild(1)->getId(), aCopy->getChild(1)->getId());
    EXPECT_NE(a->getChild(1), aCopy->getChild(1));
    
    EXPECT_EQ(a->getChild(0)->getChild(0)->getName(),aCopy->getChild(0)->getChild(0)->getName());
    EXPECT_NE(a->getChild(0)->getChild(0)->getId(), aCopy->getChild(0)->getChild(0)->getId());
    EXPECT_NE(a->getChild(0)->getChild(0), aCopy->getChild(0)->getChild(0));
    
    {
        const standard_cyborg::sc3d::Geometry& geo = a->getChild(0)->getChild(0)->asGeometryNode()->getGeometry();
        const standard_cyborg::sc3d::Geometry& copyGeo = aCopy->getChild(0)->getChild(0)->asGeometryNode()->getGeometry();
        
        EXPECT_FALSE(&copyGeo == &geo);
        
        Vec3 v{1.0f, 2.0f, 3.0f};
        EXPECT_TRUE(copyGeo.getPositions()[0] == v);
    }
}

TEST(SceneGraphTests, testGetSize)
{   
    {
        std::shared_ptr<standard_cyborg::scene_graph::PolylineNode> a(new standard_cyborg::scene_graph::PolylineNode("a"));
        a->getPolyline().setPositions(std::vector<Vec3>{Vec3{1.0, 0.0, 0.0}, Vec3{0.0, 1.0, 0.0}, Vec3{0.0, 0.0, 1.0} });
        
        EXPECT_EQ(a->approximateSizeInBytes(), sizeof(Vec3) * 3);
    }
    
    std::vector< std::shared_ptr<Node>> history;
    std::shared_ptr<standard_cyborg::scene_graph::GeometryNode> a(new standard_cyborg::scene_graph::GeometryNode("a"));
    
    {
        std::vector<Vec3> positions = {
            {1.0f, 2.0f, 3.0f},
            {4.0f, 10.0f, -2.0f},
            {-14.0f, 45.0f, 13.0f},
        };
        
        std::vector<Vec3> normals = {
            {1.0f, 0.0f, 0.0f},
            {1.0f, 0.0f, 0.0f},
            {1.0f, 0.0f, 0.0f},
            
            
        };
        std::vector<Vec3> colors = {
            {0.0f, 1.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
        };
        
        std::vector<standard_cyborg::sc3d::Face3> faces{
            {0, 1, 2}
        };
        
        standard_cyborg::sc3d::Geometry geo(positions, normals, colors, faces);
        
        a->getGeometry().copy(geo);
        
        EXPECT_TRUE(a->approximateSizeInBytes() >
                    sizeof(Vec3) * 3 * 3 + // size positions, normals, colors
                    sizeof(standard_cyborg::sc3d::Face3)
                    );
    }
    
    std::shared_ptr<standard_cyborg::scene_graph::GeometryNode> d(new standard_cyborg::scene_graph::GeometryNode());
    a->setName("d");
    {
        std::vector<Vec3> positions = {
            {1.0f, 2.0f, 3.0f},
            {4.0f, 10.0f, -2.0f},
            {-14.0f, 45.0f, 13.0f},
        };
        
        std::vector<Vec3> normals = {
            {1.0f, 0.0f, 0.0f},
            {1.0f, 0.0f, 0.0f},
            {1.0f, 0.0f, 0.0f},
            
            
        };
        std::vector<Vec3> colors = {
            {0.0f, 1.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
        };
        
        standard_cyborg::sc3d::Geometry geo (positions, normals, colors);
        
        d->getGeometry().copy(geo);
        
        EXPECT_TRUE(d->approximateSizeInBytes() >
                    sizeof(Vec3) * 3 * 3 // size positions, normals, colors
                    );
    }
    
    std::shared_ptr<standard_cyborg::scene_graph::LandmarkNode> b(new standard_cyborg::scene_graph::LandmarkNode("b"));
    b->setLandmark({"ab", Vec3(1.0, 2.0f, 3.0f)});
    
    EXPECT_EQ(b->approximateSizeInBytes(), sizeof(Vec3) + 2*sizeof(char));
    
    EXPECT_TRUE(a->appendChild(b, a));
    
    history.push_back(a);
    
    EXPECT_EQ(Node::calculateHistorySizeInBytes(history), b->approximateSizeInBytes() + a->approximateSizeInBytes());
    
    history.push_back(Node::mutateNode(b, history[history.size()-1],
                                       [](std::shared_ptr<Node> targetNode, std::shared_ptr<Node> rootNode) {
        targetNode->setName("xyz", rootNode);
    }));
    
    EXPECT_EQ(Node::calculateHistorySizeInBytes(history), b->approximateSizeInBytes() + a->approximateSizeInBytes());
    
    history.push_back(Node::mutateNode(a, history[history.size()-1],
                                       [&](std::shared_ptr<Node> targetNode, std::shared_ptr<Node> rootNode) {
        targetNode->appendChild(d, rootNode);
    }));
    
    EXPECT_EQ(Node::calculateHistorySizeInBytes(history),
              b->approximateSizeInBytes() + a->approximateSizeInBytes() + d->approximateSizeInBytes());
}


TEST(SceneGraphTests, testGeometryAllocatedIds)
{
    Node::resetAllocatedResources();
    Node::resetAllocatedResources();
    
    std::vector< std::shared_ptr<Node>> history;
    std::shared_ptr<standard_cyborg::scene_graph::GeometryNode> a(new standard_cyborg::scene_graph::GeometryNode("a"));
    std::shared_ptr<standard_cyborg::scene_graph::GeometryNode> b(new standard_cyborg::scene_graph::GeometryNode("b"));
    std::shared_ptr<standard_cyborg::scene_graph::GeometryNode> c(new standard_cyborg::scene_graph::GeometryNode("c"));
    
    {
        std::vector<Vec3> positions = {{1.0f, 2.0f, 3.0f}};
        a->getGeometry().copy(standard_cyborg::sc3d::Geometry(positions));
    }
    
    {
        std::vector<Vec3> positions = {{1.0f, 2.0f, 3.0f}};
        b->getGeometry().copy(standard_cyborg::sc3d::Geometry(positions));
    }
    
    {
        std::vector<Vec3> positions = {{1.0f, 2.0f, 3.0f}};
        c->getGeometry().copy(standard_cyborg::sc3d::Geometry(positions));
    }
    
    EXPECT_TRUE(a->appendChild(std::move(b), a));
    EXPECT_TRUE(a->appendChild(std::move(c), a));
    
    history.push_back(std::move(a));
    history.push_back(Node::mutateNode(
                                       history[history.size()-1]->getChildSharedPtr(0),
                                       history[history.size()-1],
                                       [](std::shared_ptr<Node> targetNode, std::shared_ptr<Node> rootNode) {
        targetNode->setName("xyz", rootNode); } ));
    
    
    EXPECT_EQ(standard_cyborg::sc3d::Geometry::getAllocatedIds().size(), 3);
    
    history.push_back(Node::mutateNode(
                                       history[history.size()-1]->getChildSharedPtr(0),
                                       history[history.size()-1],
                                       [&](std::shared_ptr<Node> targetNode, std::shared_ptr<Node> rootNode)
                                       {
        rootNode->removeChild(targetNode);
    }));
    
    history.push_back(Node::mutateNode(
                                       history[history.size()-1]->getChildSharedPtr(0), // b.
                                       history[history.size()-1],
                                       [&](std::shared_ptr<Node> targetNode, std::shared_ptr<Node> rootNode)
                                       {
        rootNode->removeChild(targetNode);
    }));
    
    EXPECT_EQ(standard_cyborg::sc3d::Geometry::getAllocatedIds().size(), 3);
    
    history.erase(history.begin());
    EXPECT_EQ(standard_cyborg::sc3d::Geometry::getAllocatedIds().size(), 3);
    
    history.erase(history.begin());
    EXPECT_EQ(standard_cyborg::sc3d::Geometry::getAllocatedIds().size(), 2);
    
    history.erase(history.begin());
    EXPECT_EQ(standard_cyborg::sc3d::Geometry::getAllocatedIds().size(), 1);
    
    history.erase(history.begin());
    EXPECT_EQ(standard_cyborg::sc3d::Geometry::getAllocatedIds().size(), 0);
}

TEST(SceneGraphTests, testPerspectiveCameraNodeRepresentationGeometry) {
    std::unique_ptr<standard_cyborg::scene_graph::PerspectiveCameraNode> node = std::make_unique<standard_cyborg::scene_graph::PerspectiveCameraNode> ();
    standard_cyborg::sc3d::PerspectiveCamera& camera = node->getPerspectiveCamera();
    camera.setNominalIntrinsicMatrix(math::Mat3x3({1, 0, 1, 0, 1, 1, 0, 0, 1}));
    camera.setIntrinsicMatrixReferenceSize({1, 1});
    camera.setExtrinsicMatrix({
        0.07188516, -0.99225633, -0.10129113, 1.0,
        0.53543876, -0.04728975,  0.84324908, 2.0,
        -0.84150927, -0.11485229,  0.52789307, 3.0
    });
    camera.setOrientationMatrix({
        0.27986145, -0.75411556,  0.59412734, 0.0,
        -0.265547  ,  0.53390646,  0.80276316, 0.0,
        -0.92258461, -0.38243119, -0.05083329, 0.0
    });
    
    std::shared_ptr<standard_cyborg::sc3d::Geometry> frustum = node->getRepresentationGeometry();
    std::vector<Vec3> positions = frustum->getPositions();
    
    std::vector<Vec3> expectedPositions = std::vector<Vec3>({
        Vec3{2.33728f, 0.0991766f, 2.37377f},
        Vec3{2.31843f, 0.126439f, 2.4112f},
        Vec3{2.36806f, 0.136752f, 2.36191f},
        Vec3{2.34921f, 0.164015f, 2.39934f},
        Vec3{2.40531f, -0.151047f, 2.38991f},
        Vec3{2.3299f, -0.041998f, 2.53965f},
        Vec3{2.52842f, -0.000743511f, 2.34245f},
        Vec3{2.45301f, 0.108306f, 2.49219f}
    });
    
    EXPECT_EQ(positions.size(), expectedPositions.size());
    for (int i = 0; i < expectedPositions.size(); i++) {
        EXPECT_TRUE(Vec3::almostEqual(positions[i], expectedPositions[i], 1.0e-5));
    }
}
