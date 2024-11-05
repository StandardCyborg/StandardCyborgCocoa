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

#include "standard_cyborg/sc3d/Landmark.hpp"
#include "standard_cyborg/io/json/LandmarkFileIO_JSON.hpp"

using standard_cyborg::sc3d::Landmark;

using standard_cyborg::io::json::WriteLandmarkToJSONStream;
using standard_cyborg::io::json::ReadLandmarkFromJSONStream;
using standard_cyborg::io::json::WriteLandmarkToJSONFile;
using standard_cyborg::io::json::ReadLandmarkFromJSONFile;

TEST(LandmarkFileIOTests, testLandmarkReadWrite)
{
    Landmark originalLandmark{"foot", {1.0, 2.0, 3.0}};
    
    std::stringstream buf;
    WriteLandmarkToJSONStream(buf, originalLandmark);
    
    Landmark readLandmark;
    EXPECT_TRUE(ReadLandmarkFromJSONStream(readLandmark, buf));
    
    EXPECT_EQ(originalLandmark.position, readLandmark.position);
    EXPECT_EQ(originalLandmark.name, readLandmark.name);
}

TEST(LandmarkFileIOTests, testLandmarkFileIO)
{
    Landmark originalLandmark{"foot", {1.0, 2.0, 3.0}};
    WriteLandmarkToJSONFile("/tmp/test-landmark.json", originalLandmark);
    
    Landmark readLandmark;
    ReadLandmarkFromJSONFile(readLandmark, "/tmp/test-landmark.json");
    EXPECT_EQ(originalLandmark.position, readLandmark.position);
    EXPECT_EQ(originalLandmark.name, readLandmark.name);
}

