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


#include "standard_cyborg/sc3d/ColorImage.hpp"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

namespace standard_cyborg {
namespace sc3d {

using math::Vec4;

ColorImage::ColorImage(int width_, int height_) :
    width(width_),
    height(height_)
{
    SCASSERT(width >= 0, "Width must be >= 0");
    SCASSERT(height >= 0, "Height must be >= 0");
    rgba.resize(width * height);
};

ColorImage::ColorImage(int width_, int height_, const std::vector<Vec4>& rgba_) :
    width(width_),
    height(height_)
{
    SCASSERT(width >= 0, "Width  must be >= 0");
    SCASSERT(height >= 0, "Height  must be >= 0");
    SCASSERT(width * height == rgba_.size(), "Size of data must match size of image");
    rgba = rgba_;
}

ColorImage::ColorImage() :
    width(0),
    height(0)
{}


int ColorImage::getWidth() const { return width; }

int ColorImage::getHeight() const { return height; }

void ColorImage::copy(const ColorImage& src)
{
    reset(src.getWidth(), src.getHeight(), src.getData());
    this->frame = src.frame;
}

void ColorImage::move(ColorImage&& src)
{
    height = src.height;
    width = src.width;
    rgba = std::move(src.rgba);
    frame = std::move(src.frame);
}

void ColorImage::resetSize(int width_, int height_)
{
    SCASSERT(width >= 0, "Width must be >= 0");
    SCASSERT(height >= 0, "Height must be >= 0");
    width = width_;
    height = height_;

    // This allocates the correct amount of storage for consistency, but makes no
    // gaurantees about what data is there.
    rgba.resize(width * height);
}

void ColorImage::reset(int width_, int height_, const std::vector<Vec4>& rgba_)
{
    SCASSERT(width_ * height_ == rgba_.size(), "Size of data must match size of image");
    width = width_;
    height = height_;
    rgba = rgba_;
}

const std::vector<math::Vec4>& ColorImage::getData() const
{
    return rgba;
}


std::vector<math::Vec4>& ColorImage::getData()
{
    return rgba;
}


void ColorImage::resizeFrom(const ColorImage& src)
{
    const float* srcData = reinterpret_cast<const float*>(src.getData().data());
    float* dstData = reinterpret_cast<float*>(const_cast<Vec4*>(rgba.data()));
    stbir_resize_float_linear(srcData, src.getWidth(), src.getHeight(), 0,
                              dstData, width, height, 0, STBIR_RGBA);
}

void ColorImage::resize(int newWidth, int newHeight)
{
    SCASSERT(newWidth >= 0, "Width must be >= 0");
    SCASSERT(newHeight >= 0, "Height must be >= 0");
    const float* srcData = reinterpret_cast<const float*>(rgba.data());

    std::vector<Vec4> newRgba(newWidth * newHeight);
    float* dstData = reinterpret_cast<float*>(newRgba.data());

    stbir_resize_float_linear(srcData, width, height, 0,
                              dstData, newWidth, newHeight, 0, STBIR_RGBA);

    width = newWidth;
    height = newHeight;
    rgba = newRgba;
}

bool operator==(const ColorImage& lhs, const ColorImage& rhs)
{
    if (lhs.getWidth() != rhs.getWidth()) return false;
    if (lhs.getHeight() != rhs.getHeight()) return false;

    const std::vector<math::Vec4>& lhsData = lhs.getData();
    const std::vector<math::Vec4>& rhsData = rhs.getData();

    int n = lhs.getWidth() * lhs.getHeight();
    for (int i = 0; i < n; i++) {
        if (lhsData[i] != rhsData[i]) return false;
    }
    return true;
}

void ColorImage::flipX()
{
    for (int row = 0; row < height; row++) {
        for (int col = width / 2 - 1; col >= 0; col--) {
            int index1 = row * width + col;
            int index2 = row * width + (width - col - 1);

            Vec4 tmp = rgba[index1];
            rgba[index1] = rgba[index2];
            rgba[index2] = tmp;
        }
    }
}

void ColorImage::flipY()
{
    for (int row = height / 2 - 1; row >= 0; row--) {
        for (int col = 0; col < width; col++) {
            int index1 = row * width + col;
            int index2 = (height - row - 1) * width + col;

            Vec4 tmp = rgba[index1];
            rgba[index1] = rgba[index2];
            rgba[index2] = tmp;
        }
    }
}

int ColorImage::getSizeInBytes() const
{
    return width * height * 4 * sizeof(float);
}

void ColorImage::premultiplyAlpha()
{
    for (int row = 0; row < getHeight(); row++) {
        for (int col = 0; col < getWidth(); col++) {
            int index = row * width + col;
            Vec4 pixel = rgba[index];
            rgba[index] = Vec4{
                pixel.xyz() * pixel.w,
                pixel.w};
        }
    }
}

void ColorImage::mutatePixelsByColRow(const std::function<Vec4(int col, int row, Vec4 rgba)>& mapFn)
{
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            int index = row * width + col;
            Vec4 pixel = rgba[index];
            rgba[index] = mapFn(col, row, pixel);
        }
    }
}


} // namespace sc3d
} // namespace standard_cyborg
