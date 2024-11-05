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

#include <sstream>

#include "standard_cyborg/sc3d/Polyline.hpp"

#include "standard_cyborg/io/json/PolylineFileIO_JSON.hpp"

using standard_cyborg::math::Vec3;

TEST(PolylineFileIOTests, testPolylineReadWrite)
{
    standard_cyborg::sc3d::Polyline originalPolyline({{1.0, 2.0, 3.0}, {2.0, 3.0, 4.0}});
    
    std::stringstream buf;
    standard_cyborg::io::json::WritePolylineToJSONStream(buf, originalPolyline);
    
    standard_cyborg::sc3d::Polyline readPolyline;
    EXPECT_TRUE(standard_cyborg::io::json::ReadPolylineFromJSONStream(readPolyline, buf));
    
    EXPECT_EQ(originalPolyline.getPositions(), readPolyline.getPositions());
}

TEST(PolylineFileIOTests, testPolylineWithNaN)
{
    standard_cyborg::sc3d::Polyline originalPolyline({{1.0, NAN, 3.0}, {2.0, 3.0, 4.0}});
    
    std::stringstream buf;
    standard_cyborg::io::json::WritePolylineToJSONStream(buf, originalPolyline);
    
    standard_cyborg::sc3d::Polyline readPolyline;
    EXPECT_TRUE(standard_cyborg::io::json::ReadPolylineFromJSONStream(readPolyline, buf));
    
    std::vector<Vec3> positions = readPolyline.getPositions();
    EXPECT_EQ(positions[0].x, 1.0);
    EXPECT_TRUE(isnan(positions[0].y));
    EXPECT_EQ(positions[0].z, 3.0);
    
    EXPECT_EQ(positions[1].x, 2.0);
    EXPECT_EQ(positions[1].y, 3.0);
    EXPECT_EQ(positions[1].z, 4.0);
}
