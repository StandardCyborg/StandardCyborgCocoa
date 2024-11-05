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

#include "standard_cyborg/sc3d/VertexSelection.hpp"
#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/sc3d/VertexSelection.hpp"
#include "standard_cyborg/sc3d/Polyline.hpp"

using standard_cyborg::sc3d::VertexSelection;
namespace math = standard_cyborg::math;

/*
 #import <XCTest/XCTest.h>
 
 #include <algorithm>
 #include <iostream>
 #include <set>
 #include <vector>
 
 #include <StandardCyborgData/DebugHelpers.hpp>
 
 #include <StandardCyborgData/Polyline.hpp>
 #include <StandardCyborgData/VertexSelection.hpp>
 
 using standard_cyborg::sc3d::VertexSelection;
 
 namespace math = StandardCyborg::math;
 
 @interface VertexSelectionTests : XCTestCase
 
 @end
 
 @implementation VertexSelectionTests
 */

TEST(VertexSelectionTests, testInitilizeEmptySelection)
{
    VertexSelection selection;
    
    EXPECT_EQ(selection.size(), 0);
    EXPECT_TRUE(selection == VertexSelection({}));
}

TEST(VertexSelectionTests, testCountInitialization)
{
    VertexSelection selection(5);
    EXPECT_TRUE(selection == VertexSelection(5, {}));
}

TEST(VertexSelectionTests, testInitilizerList)
{
    VertexSelection selection{6, {1, 2, 3, 4, 5}};
    
    EXPECT_EQ(selection.size(), 5);
    EXPECT_TRUE(selection == VertexSelection(6, {1, 2, 3, 4, 5}));
}

TEST(VertexSelectionTests, testCopy)
{
    VertexSelection src(10, {1, 2, 3, 4, 5});
    VertexSelection dst(10, {5, 6, 7, 8, 9});
    dst.copy(src);
    EXPECT_TRUE(dst == VertexSelection(10, {1, 2, 3, 4, 5}));
}

TEST(VertexSelectionTests, testClear)
{
    VertexSelection selection(6, {1, 2, 3, 4, 5});
    selection.clear();
    EXPECT_TRUE(selection == VertexSelection({}));
}
// Uncomment to verify non-const iterators aren't implemented
/*
 TEST(VertexSelectionTests, testNonConstIterators {
 VertexSelection selection {1, 2, 3};
 for (VertexSelection::iterator it = selection.begin(); it != selection.end(); it++) {
 *it = *it + 1;
 }
 }
 */

TEST(VertexSelectionTests, testConstructionFromVector)
{
    VertexSelection selection(4, std::vector<int>{2, 1, 3});
    EXPECT_TRUE(selection == VertexSelection(4, {1, 2, 3}));
}

TEST(VertexSelectionTests, testConstIterators)
{
    VertexSelection selection(4, {1, 2, 3});
    std::vector<int> output(3);
    std::copy(selection.begin(), selection.end(), output.begin());
    EXPECT_TRUE(output == std::vector<int>({1, 2, 3}));
}

TEST(VertexSelectionTests, testInsertIndex)
{
    VertexSelection selection(3);
    selection.insertValue(2);
    EXPECT_TRUE(selection == VertexSelection(3, {2}));
}

TEST(VertexSelectionTests, testIdempotentInsertion)
{
    VertexSelection selection;
    selection.insertValue(2);
    selection.insertValue(2);
    EXPECT_TRUE(selection == VertexSelection(3, {2}));
}

TEST(VertexSelectionTests, testRemoveValue) {
    VertexSelection selection(4, {2, 3});
    selection.removeValue(2);
    EXPECT_TRUE(selection == VertexSelection(4, {3}));
}

TEST(VertexSelectionTests, testIdempotentRemoval)
{
    VertexSelection selection(4, {2, 3});
    selection.removeValue(2);
    selection.removeValue(2);
    EXPECT_TRUE(selection == VertexSelection(4, {3}));
}

TEST(VertexSelectionTests, testUnionWith)
{
    VertexSelection selection(6, {1, 2, 3});
    VertexSelection verticesToAdd(6, {3, 4, 5});
    selection.unionWith(verticesToAdd);
    EXPECT_TRUE(selection == VertexSelection(6, {1, 2, 3, 4, 5}));
}

TEST(VertexSelectionTests, testDifferenceWith)
{
    VertexSelection selection(6, {1, 2, 3, 4, 5});
    VertexSelection verticesToRemove(6, {2, 3, 4});
    selection.differenceWith(verticesToRemove);
    EXPECT_TRUE(selection == VertexSelection(6, {1, 5}));
}

TEST(VertexSelectionTests, testIntersectionWith)
{
    VertexSelection s1(6, {1, 2, 3});
    s1.intersectWith(VertexSelection(6, {2, 3, 4}));
    EXPECT_TRUE(s1 == VertexSelection(6, {2, 3}));
    
    VertexSelection s2(6, {2, 3, 4});
    s2.intersectWith(VertexSelection(6, {1, 2, 3}));
    EXPECT_TRUE(s2 == VertexSelection(6, {2, 3}));
    
    VertexSelection s3(6, {2, 3, 4, 8, 2, 2, 7});
    s3.intersectWith(VertexSelection(6, {1, 2, 3, 14, 7}));
    EXPECT_TRUE(s3 == VertexSelection(6, {2, 3, 7}));
    
    VertexSelection s4(6, {2, 4, 8, 2, 2, 7, 3, 22, 7, 3});
    s4.intersectWith(VertexSelection(6, {1, 22, 2, 3, 14, 7}));
    EXPECT_TRUE(s4 == VertexSelection(6, {2, 3, 7, 22}));
    
    VertexSelection s5(6, {1, 2, 3, 4});
    s5.intersectWith(VertexSelection(6, {2, 4}));
    EXPECT_TRUE(s5 == VertexSelection(6, {2, 4}));
    
    VertexSelection s6(6, {1, 2, 3, 4});
    s6.intersectWith(VertexSelection(6, {2, 5}));
    EXPECT_TRUE(s6 == VertexSelection(6, {2}));
    
    VertexSelection s7(6, {1, 2, 3, 4});
    s7.intersectWith(VertexSelection(6, {0, 3}));
    EXPECT_TRUE(s7 == VertexSelection(6, {3}));
    
    VertexSelection s8(6, {1, 2, 3, 4});
    s8.intersectWith(VertexSelection(6, {0, 5}));
    EXPECT_TRUE(s8 == VertexSelection(6, {}));
    
    VertexSelection s9{};
    s9.intersectWith(VertexSelection(6, {0, 5}));
    EXPECT_TRUE(s9 == VertexSelection(6, {}));
}

TEST(VertexSelectionTests, testComparison)
{
    VertexSelection selection1(6, {1, 2, 3, 4, 5});
    
    VertexSelection selection2(6, {1, 2, 3, 4, 5});
    EXPECT_TRUE(selection1 == selection2);
    
    VertexSelection selection3(6, {1, 2, 3, 4});
    EXPECT_TRUE(selection1 != selection3);
    
    VertexSelection selection4(6, {1, 2, 3, 4, 5, 6});
    EXPECT_TRUE(selection1 != selection4);
}

TEST(VertexSelectionTests, testGetTotalVertexCount)
{
    VertexSelection selection(6, {1, 2, 3, 4, 5});
    
    EXPECT_EQ(selection.getTotalVertexCount(), 6);
}

TEST(VertexSelectionTests, testToVector)
{
    VertexSelection selection(6, {1, 2, 3, 4, 5});
    std::vector<int> vec{1, 2, 3, 4, 5};
    EXPECT_EQ(selection.toVector(), vec);
}

TEST(VertexSelectionTests, testFromGeometry)
{
    std::vector<math::Vec3> positions{
        {+1.0f, +2.0f, +3.0f},
        {+6.0f, +7.0f, +8.0f},
        {-2.0f, -1.0f, -6.0f},
        {-9.0f, +4.0f, +1.0f},
    };
    
    std::vector<math::Vec3> normals{
        {+1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, -1.0f, 1.0f},
    };
    
    std::vector<math::Vec3> colors{
        {0.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
    };
    
    standard_cyborg::sc3d::Geometry geo(positions, normals, colors);
    
    /* Test filtering via position */
    std::shared_ptr<VertexSelection> viaPosition = VertexSelection::fromGeometryVertices(geo, [](int index, math::Vec3 position, math::Vec3 normal, math::Vec3 color) {
        return position.x < 0.0;
    });
    
    EXPECT_TRUE(*viaPosition == VertexSelection(6, {2, 3}));
    
    /* Test filtering via normal */
    std::shared_ptr<VertexSelection> viaNormal = VertexSelection::fromGeometryVertices(geo, [](int index, math::Vec3 position, math::Vec3 normal, math::Vec3 color) {
        return normal.y < 0.0;
    });
    
    EXPECT_TRUE(*viaNormal == VertexSelection(6, {3}));
    
    /* Test filtering via color */
    std::shared_ptr<VertexSelection> viaColor = VertexSelection::fromGeometryVertices(geo, [](int index, math::Vec3 position, math::Vec3 normal, math::Vec3 color) {
        return color.x > 0.0;
    });
    
    EXPECT_TRUE(*viaColor == VertexSelection(3, {2}));
}

TEST(VertexSelectionTests, testFromPolyline)
{
    std::vector<math::Vec3> positions{
        {+1.0f, +2.0f, +3.0f},
        {+6.0f, +7.0f, +8.0f},
        {-2.0f, -1.0f, -6.0f},
        {-9.0f, +4.0f, +1.0f},
    };
    standard_cyborg::sc3d::Polyline polyline(positions);
    VertexSelection selection(polyline, std::vector<int>{1, 2});
    EXPECT_TRUE(selection == VertexSelection(4, {1, 2}));
}

TEST(VertexSelectionTests, testInvert)
{
    VertexSelection selection(7, {1, 2, 4});
    selection.invert();
    EXPECT_TRUE(selection == VertexSelection(7, {0, 3, 5, 6}));
    
    VertexSelection selection2(8, {1, 2, 4, 7});
    selection2.invert();
    EXPECT_TRUE(selection2 == VertexSelection(8, {0, 3, 5, 6}));
}

TEST(VertexSelectionTests, testInvertWRTGeometry)
{
    standard_cyborg::sc3d::Geometry geometry(std::vector<math::Vec3>{
        math::Vec3 {0.0f, 0.0f, 0.0f},
        math::Vec3 {0.0f, 0.0f, 0.0f},
        math::Vec3 {0.0f, 0.0f, 0.0f},
    });
    
    VertexSelection selection(geometry, {0, 1});
    selection.invert();
    EXPECT_TRUE(selection == VertexSelection(geometry, {2}));
}
