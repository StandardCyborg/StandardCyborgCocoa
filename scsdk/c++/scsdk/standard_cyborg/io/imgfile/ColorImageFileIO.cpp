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

#include "standard_cyborg/io/imgfile/ColorImageFileIO.hpp"
#include "standard_cyborg/sc3d/ColorImage.hpp"

#include <ios>
#include <iostream>
#include <fstream>
#include <sstream>

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
#include <stb_image_write.h>
#include <stb_image.h>

namespace standard_cyborg {
namespace io {
namespace imgfile {

bool WriteColorImageToFile(std::string filename, const sc3d::ColorImage& image, ImageFormat format, int jpegQuality)
{
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    
    if (!file.good()) {
        file.close();
        return false;
    }
    
    bool status = WriteColorImageToStream(file, image, format, jpegQuality);
    
    file.close();
    
    return status;
}

bool ReadColorImageFromFile(sc3d::ColorImage& destination, std::string filename)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    
    if (!file.good()) {
        file.close();
        return false;
    }
    
    bool status = ReadColorImageFromStream(destination, file);
    
    file.close();
    
    return status;
}

/** Read from a buffer into an image instance */
bool ReadColorImageFromBuffer(sc3d::ColorImage& imageOut, const uint8_t *buf, size_t len) {
    int width, height, numChannels;
    int desiredChannels = 4;

    unsigned char* data = stbi_load_from_memory(
      static_cast<const stbi_uc*>(buf), 
      len, 
      &width, 
      &height, 
      &numChannels, 
      desiredChannels);
    
    if (data == NULL) {
      return false;
    }
    
    std::vector<math::Vec4> rgba(height * width);
    for (size_t i = 0; i < rgba.size(); ++i) {
        // Jpeg and PNG have sRGB gamma, and ColorImage expects linear gamma
        rgba[i] = {
            sc3d::ApproximateSRGBGammaToLinear({
                static_cast<float>(data[4 * i    ]) / 255.0f,
                static_cast<float>(data[4 * i + 1]) / 255.0f,
                static_cast<float>(data[4 * i + 2]) / 255.0f
            }),
            static_cast<float>(data[4 * i + 3]) / 255.0f
        };
            // NB: when requesting jpeg, stbi actually retruns a `numChannels`
            // of 3, although it appears to lay out the `data` array such that
            // accessing a fourth channel is safe (and accessing r g and b
            // as coded above is correct)
    }
    
    stbi_image_free(data);

    imageOut.reset(width, height, rgba);
    
    return true;
}






static void stbiWriteCallback(void* context, void* data, int size)
{
    std::ostream& outStream = *static_cast<std::ostream*>(context);
    outStream.write((const char*)data, size);
}

bool WriteColorImageToStream(std::ostream& outStream, const sc3d::ColorImage& image, ImageFormat format, int jpegQuality)
{
    int width = image.getWidth();
    int height = image.getHeight();
    const std::vector<math::Vec4>& rgba = image.getData();

    int n = width * height;
    std::vector<unsigned char> pixels(n * 4);
    for (int i = 0; i < n; i++) {
        math::Vec3 rgb = sc3d::LinearToApproximateSRGB(rgba[i].xyz());
        float alpha = rgba[i].w;
        pixels[i * 4 + 0] = (unsigned char)std::max(0, std::min(255, (int)(rgb.x * 255.0)));
        pixels[i * 4 + 1] = (unsigned char)std::max(0, std::min(255, (int)(rgb.y * 255.0)));
        pixels[i * 4 + 2] = (unsigned char)std::max(0, std::min(255, (int)(rgb.z * 255.0)));
        pixels[i * 4 + 3] = (unsigned char)std::max(0, std::min(255, (int)(alpha * 255.0)));
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

bool ReadColorImageFromStream(sc3d::ColorImage& destination, std::istream& inStream)
{
    int width, height, numChannels;
    int desiredChannels = 4;
    unsigned char* data = stbi_load_from_callbacks(&istreamCallbacks, static_cast<void*>(&inStream), &width, &height, &numChannels, desiredChannels);
    
    if (data == NULL) return false;
    
    std::vector<math::Vec4> rgba(width * height);
    
    int n = width * height;
    for (int i = 0; i < n; i++) {
        rgba[i] = {
            sc3d::ApproximateSRGBGammaToLinear({
                static_cast<float>(data[4 * i    ]) / 255.0f,
                static_cast<float>(data[4 * i + 1]) / 255.0f,
                static_cast<float>(data[4 * i + 2]) / 255.0f
            }),
            static_cast<float>(data[4 * i + 3]) / 255.0f
        };
    }
    
    stbi_image_free(data);

    destination.reset(width, height, rgba);
    
    return true;
}


bool WriteColorImageToBuffer(
            std::string &buf,
            const sc3d::ColorImage& image,
            ImageFormat format,
            int jpegQuality) {

    const int width = image.getWidth();
    const int height = image.getHeight();
    const std::vector<math::Vec4>& rgba = image.getData();

    int n = width * height;
    std::vector<unsigned char> pixels(n * 4);
    for (int i = 0; i < n; i++) {
        // ColorImage is natively Linear gamma, and image files like jpeg and
        // png expect sRGB gamma
        const math::Vec3 rgb = sc3d::LinearToApproximateSRGB(rgba[i].xyz());
        const float alpha = rgba[i].w;
        auto ToUChar = [](float v) -> unsigned char {
            return (unsigned char)std::max(0, std::min(255, (int)(v * 255.0)));
        };

        pixels[i * 4 + 0] = ToUChar(rgb.x);
        pixels[i * 4 + 1] = ToUChar(rgb.y);
        pixels[i * 4 + 2] = ToUChar(rgb.z);
        pixels[i * 4 + 3] = ToUChar(alpha);
    }

    auto stbiWriteCallback = [](void* context, void* data, int size) {
        std::ostream& outStream = *static_cast<std::ostream*>(context);
        outStream.write((const char*)data, size);
    };
    std::stringstream ss;

    bool success = false;
    if (format == ImageFormat::JPEG) {
        success = stbi_write_jpg_to_func(stbiWriteCallback, static_cast<void*>(&ss), width, height, 4, static_cast<void*>(pixels.data()), jpegQuality);
    } else if (format == ImageFormat::PNG) {
        success = stbi_write_png_to_func(stbiWriteCallback, static_cast<void*>(&ss), width, height, 4, static_cast<void*>(pixels.data()), 4 * width);
    }

    if (success) {
        buf = ss.str();
    }

    return success;
}


} // namespace imgfile
} // namespace io
} // namespace standard_cyborg
