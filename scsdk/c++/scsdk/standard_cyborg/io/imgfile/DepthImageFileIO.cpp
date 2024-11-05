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

#include "standard_cyborg/io/imgfile/DepthImageFileIO.hpp"
#include "standard_cyborg/sc3d/DepthImage.hpp"

#include <ios>
#include <iostream>
#include <fstream>

#define STBI_WRITE_NO_STDIO
//#define STBI_NO_JPEG
//#define STBI_NO_PNG
#define STBI_NO_BMP
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#include "stb_image_write.h"
#include "stb_image.h"

namespace standard_cyborg {
namespace io {
namespace imgfile {

bool WriteDepthImageToFile(std::string filename, const sc3d::DepthImage& image, ImageFormat format, int jpegQuality)
{
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    
    if (!file.good()) {
        file.close();
        return false;
    }
    
    bool status = WriteDepthImageToStream(file, image, format, jpegQuality);
    
    file.close();
    
    return status;
}

bool ReadDepthImageFromFile(sc3d::DepthImage& destination, std::string filename)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    
    if (!file.good()) {
        file.close();
        return false;
    }
    
    bool status = ReadDepthImageFromStream(destination, file);
    
    file.close();
    
    return status;
}



static void stbiWriteCallback(void* context, void* data, int size)
{
    std::ostream& outStream = *static_cast<std::ostream*>(context);
    outStream.write((const char*)data, size);
}

bool WriteDepthImageToStream(std::ostream& outStream, const sc3d::DepthImage& image, ImageFormat format, int jpegQuality)
{
    int width = image.getWidth();
    int height = image.getHeight();
    const std::vector<float>& depth = image.getData();

    int n = width * height;
    std::vector<unsigned char> pixels(n * 4);
    for (int i = 0; i < n; i++) {
        float value = std::pow(depth[i], 1.0 / 2.2);
        int iValue = (unsigned char)std::max(0, std::min(255, (int)(value * 255)));
        pixels[i * 4 + 0] = iValue;
        pixels[i * 4 + 1] = iValue;
        pixels[i * 4 + 2] = iValue;
        pixels[i * 4 + 3] = 255;
    }
    
    switch (format) {
        case ImageFormat::JPEG:
            return stbi_write_jpg_to_func(stbiWriteCallback, static_cast<void*>(&outStream), width, height, 4, static_cast<void*>(pixels.data()), jpegQuality);
        default:
        case ImageFormat::PNG:
            return stbi_write_png_to_func(stbiWriteCallback, static_cast<void*>(&outStream), width, height, 4, static_cast<void*>(pixels.data()), 4 * width);
    }
}

static int stbiReadCallback(void* user, char* data, int size)
{
    std::istream& inStream = *static_cast<std::istream*>(user);
    inStream.read(data, size);
    return int(inStream.gcount());
}

static void stbiSkipCallback(void* user, int n)
{
    std::istream& inStream = *static_cast<std::istream*>(user);
    inStream.seekg(n, std::ios_base::cur);
}

static int stbiEofCallback(void* user)
{
    std::istream& in = *static_cast<std::istream*>(user);
    return in.eof();
}

static const stbi_io_callbacks istreamCallbacks = stbi_io_callbacks{
    &stbiReadCallback,
    &stbiSkipCallback,
    &stbiEofCallback
};

bool ReadDepthImageFromStream(sc3d::DepthImage& destination, std::istream& inStream)
{
    int width, height, numChannels;
    int desiredChannels = 4;
    unsigned char* data = stbi_load_from_callbacks(&istreamCallbacks, static_cast<void*>(&inStream), &width, &height, &numChannels, desiredChannels);
    
    if (data == NULL) return false;
    
    std::vector<float> depth(width * height);
    
    int n = width * height;
    for (int i = 0; i < n; i++) {
        depth[i] = std::pow(static_cast<float>(data[4 * i]) / 255.0f, 2.2f);
    }
    
    stbi_image_free(data);
    
    destination.reset(width, height, depth);
    
    return true;
}

} // namespace imgfile
} // namespace io
} // namespace standard_cyborg
