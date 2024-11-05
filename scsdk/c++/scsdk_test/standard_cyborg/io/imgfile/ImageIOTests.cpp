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

#include "standard_cyborg/sc3d/ColorImage.hpp"
#include "standard_cyborg/sc3d/DepthImage.hpp"

#include "standard_cyborg/io/imgfile/ColorImageFileIO.hpp"
#include "standard_cyborg/io/ply/DepthImageFileIO_PLY.hpp"

#include <fstream>

using namespace standard_cyborg::sc3d;
using namespace standard_cyborg::math;
using namespace standard_cyborg::io::imgfile;

TEST(ColorImageFileIOTests, testWritingPNGToFile) {
    std::string imagePath = "/tmp/test.png";
    
    ColorImage image (2, 2, std::vector<Vec4>{
        {1, 0, 0, 1},
        {0, 1, 0, 1},
        {0, 0, 1, 1},
        {1, 1, 0, 1}
    });
    
    EXPECT_TRUE(WriteColorImageToFile(imagePath, image, ImageFormat::PNG));
}

TEST(ColorImageFileIOTests, testPNGImageSerialization) {
    std::string imagePath = "/tmp/test.png";
    std::ofstream outStream (imagePath, std::ios::out | std::ios::binary);
    
    ColorImage image (2, 2, std::vector<Vec4>{
        {1, 0, 0, 1},
        {0, 1, 0, 1},
        {0, 0, 1, 1},
        {1, 1, 0, 1}
    });
    
    EXPECT_TRUE(WriteColorImageToStream(outStream, image, ImageFormat::PNG));
    
    outStream.close();
}


TEST(ColorImageFileIOTests, testReadPNGFromFile) {
    std::string imagePath = "/tmp/test.png";
    
    ColorImage inputImage (2, 2, std::vector<Vec4>{
        {0.0, 0.25, 0.5, 0.75},
        {0.2, 0.4, 0.6, 0.8},
        {0.1, 0.3, 0.5, 0.7},
        {1.0, 0.9, 0.8, 0.7}
    });
    
    EXPECT_TRUE(WriteColorImageToFile(imagePath, inputImage, ImageFormat::PNG));
    
    ColorImage outputImage;
    EXPECT_TRUE(standard_cyborg::io::imgfile::ReadColorImageFromFile(outputImage, imagePath));
    
    EXPECT_EQ(outputImage.getWidth(), 2);
    EXPECT_EQ(outputImage.getHeight(), 2);
    
    std::vector<Vec4> expectedPixels = inputImage.getData();
    std::vector<Vec4> pixels = outputImage.getData();
    EXPECT_EQ(expectedPixels.size(), pixels.size());
    
    // Due to quantization to 0-255, we don't exactly reproduce the floating point values,
    // but we get plenty close
    for (int i = 0; i < pixels.size(); i++) {
        EXPECT_TRUE(Vec4::almostEqual(pixels[i], expectedPixels[i], 4e-3));
    }
}

TEST(ColorImageFileIOTests, testReadJPEGFromFile) {
    std::string imagePath = "/tmp/test.jpeg";
    
    ColorImage inputImage (2, 2, std::vector<Vec4>{
        {0.0, 0.25, 0.5, 0.75},
        {0.2, 0.4, 0.6, 0.8},
        {0.1, 0.3, 0.5, 0.7},
        {1.0, 0.9, 0.8, 0.7}
    });
    
    EXPECT_TRUE(WriteColorImageToFile(imagePath, inputImage, ImageFormat::JPEG, 100));
    
    ColorImage outputImage;
    EXPECT_TRUE(standard_cyborg::io::imgfile::ReadColorImageFromFile(outputImage, imagePath));
    
    EXPECT_EQ(outputImage.getWidth(), 2);
    EXPECT_EQ(outputImage.getHeight(), 2);
    
    std::vector<Vec4> expectedPixels = inputImage.getData();
    std::vector<Vec4> pixels = outputImage.getData();
    EXPECT_EQ(expectedPixels.size(), pixels.size());
    
    
    // Due to quantization to 0-255, we don't exactly reproduce the floating point values,
    // but we get plenty close
    for (int i = 0; i < pixels.size(); i++) {
        // Restore the RGB data
        EXPECT_TRUE(Vec3::almostEqual(pixels[i].xyz(), expectedPixels[i].xyz(), 0.015));
        
        // Drops the alpha channel:
        EXPECT_EQ(pixels[i].w, 1.0);
    }
}

TEST(ColorImageFileIOTests, testPNGImageDeserialization) {
    std::stringstream ioBuffer;
    
    ColorImage inputImage (2, 2, std::vector<Vec4>{
        {0.0, 0.25, 0.5, 0.75},
        {0.2, 0.4, 0.6, 0.8},
        {0.1, 0.3, 0.5, 0.7},
        {1.0, 0.9, 0.8, 0.7}
    });
    
    WriteColorImageToStream(ioBuffer, inputImage, ImageFormat::PNG);
    
    ColorImage outputImage;
    
    ReadColorImageFromStream(outputImage, ioBuffer);
    
    EXPECT_EQ(outputImage.getWidth(), 2);
    EXPECT_EQ(outputImage.getHeight(), 2);
    
    std::vector<Vec4> expectedPixels = inputImage.getData();
    std::vector<Vec4> pixels = outputImage.getData();
    EXPECT_EQ(expectedPixels.size(), pixels.size());
    
    // Due to quantization to 0-255, we don't exactly reproduce the floating point values,
    // but we get plenty close
    for (int i = 0; i < pixels.size(); i++) {
        EXPECT_TRUE(Vec4::almostEqual(pixels[i], expectedPixels[i], 4e-3));
    }
}

TEST(ColorImageFileIOTests, testJPEGImageDeserialization) {
    std::stringstream ioBuffer;
    
    ColorImage inputImage (2, 2, std::vector<Vec4>{
        {0.0, 0.25, 0.5, 0.75},
        {0.2, 0.4, 0.6, 0.8},
        {0.1, 0.3, 0.5, 0.7},
        {1.0, 0.9, 0.8, 0.7}
    });
    
    WriteColorImageToStream(ioBuffer, inputImage, ImageFormat::JPEG, 100);
    
    ColorImage outputImage;
    
    ReadColorImageFromStream(outputImage, ioBuffer);
    
    EXPECT_EQ(outputImage.getWidth(), 2);
    EXPECT_EQ(outputImage.getHeight(), 2);
    
    std::vector<Vec4> expectedPixels = inputImage.getData();
    std::vector<Vec4> pixels = outputImage.getData();
    EXPECT_EQ(expectedPixels.size(), pixels.size());
    
    // Due to quantization to 0-255, we don't exactly reproduce the floating point values,
    // but we get plenty close
    for (int i = 0; i < pixels.size(); i++) {
        // Restore the RGB data
        EXPECT_TRUE(Vec3::almostEqual(pixels[i].xyz(), expectedPixels[i].xyz(), 0.015));
        
        // Drops the alpha channel:
        EXPECT_EQ(pixels[i].w, 1.0);
    }
}

TEST(ColorImageFileIOTests, testDepthImageSerialization) {
    std::stringstream ioBuffer;
    
    DepthImage inputImage (2, 2, std::vector<float>{1.0f, 2.0f, 3.0f, 4.0f});
    standard_cyborg::io::ply::WriteDepthImageToPLYStream(ioBuffer, inputImage);
    
    DepthImage outputImage;
    
    standard_cyborg::io::ply::ReadDepthImageFromPLYStream(outputImage, ioBuffer);
    
    EXPECT_EQ(outputImage.getWidth(), 2);
    EXPECT_EQ(outputImage.getHeight(), 2);
    
    std::vector<float> expectedPixels = inputImage.getData();
    std::vector<float> pixels = outputImage.getData();
    EXPECT_EQ(expectedPixels.size(), pixels.size());
    
    // Due to quantization to 0-255, we don't exactly reproduce the floating point values,
    // but we get plenty close
    for (int i = 0; i < pixels.size(); i++) {
        EXPECT_NEAR(pixels[i], expectedPixels[i], 1e-5);
    }
}
