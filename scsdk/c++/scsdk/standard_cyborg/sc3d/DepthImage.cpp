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

#include "standard_cyborg/sc3d/DepthImage.hpp"

#include <algorithm>

namespace standard_cyborg {
namespace sc3d {

DepthImage::DepthImage(int width_, int height_) :
    width(width_),
    height(height_)
{
    SCASSERT(width >= 0, "Width must be >= 0");
    SCASSERT(height >= 0, "Height must be >= 0");
    depth.resize(width * height);
};

DepthImage::DepthImage(int width_, int height_, const std::vector<float>& depth_) :
    width(width_),
    height(height_)
{
    SCASSERT(width >= 0, "Width must be >= 0");
    SCASSERT(height >= 0, "Height must be >= 0");
    SCASSERT(width * height == depth_.size(), "Size of image must match size of data");
    depth = depth_;
}

int DepthImage::getWidth() const { return width; }

int DepthImage::getHeight() const { return height; }

void DepthImage::copy(const DepthImage& src)
{
    reset(src.getWidth(), src.getHeight(), src.getData());
	this->frame = src.getFrame();

}

void DepthImage::move(DepthImage&& src)
{
    height = src.height;
    width = src.width;
    depth = std::move(src.depth);
    frame = std::move(src.frame);
}

void DepthImage::resetSize(int width_, int height_)
{
    SCASSERT(width >= 0, "Width must be >= 0");
    SCASSERT(height >= 0, "Height must be >= 0");
    width = width_;
    height = height_;
    
    // This allocates the correct amount of storage for consistency, but makes no
    // gaurantees about what data is there.
    depth.resize(width * height);
}

void DepthImage::reset(int width_, int height_, const std::vector<float>& depth_)
{
    SCASSERT(width_ * height_ == depth_.size(), "Size of image must match size of data");
    width = width_;
    height = height_;
    depth = depth_;
}
    
const std::vector<float>& DepthImage::getData() const
{
    return depth;
}

std::vector<float>& DepthImage::getData()
{
    return depth;
}

bool operator==(const DepthImage& lhs, const DepthImage& rhs)
{
    if (lhs.getWidth() != rhs.getWidth()) return false;
    if (lhs.getHeight() != rhs.getHeight()) return false;
    
    const std::vector<float>& lhsData = lhs.getData();
    const std::vector<float>& rhsData = rhs.getData();
    
    int n = lhs.getWidth() * lhs.getHeight();
    for (int i = 0; i < n; i++) {
        if (lhsData[i] != rhsData[i]) return false;
    }
    return true;
}

void DepthImage::flipY()
{
    for (int row = height / 2 - 1; row >= 0; row--) {
        for (int col = 0; col < width; col++) {
            int index1 = row * width + col;
            int index2 = (height - row - 1) * width + col;
            
            float tmp = depth[index1];
            depth[index1] = depth[index2];
            depth[index2] = tmp;
        }
    }
}

void DepthImage::flipX()
{
    for (int row = 0; row < height; row++) {
        for (int col = width / 2 - 1; col >= 0; col--) {
            int index1 = row * width + col;
            int index2 = row * width + (width - col - 1);
            
            float tmp = depth[index1];
            depth[index1] = depth[index2];
            depth[index2] = tmp;
        }
    }
}

void DepthImage::forEachPixelAtColRow(const std::function<void(int col, int row, float depth)>& fn) const {
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            int index = row * width + col;
            fn(col, row, depth[index]);
        }
    }
}

void DepthImage::mutatePixelsByColRow(const std::function<float(int col, int row, float value)>& mapFn) {
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            int index = row * width + col;
            depth[index] = mapFn(col, row, depth[index]);
        }
    }
}

int DepthImage::getSizeInBytes() const
{
    return width * height * sizeof(float);
}


void DepthImage::resize(int newWidth, int newHeight)
{
    SCASSERT(newWidth >= 0, "Width must be >= 0");
    SCASSERT(newHeight >= 0, "Height must be >= 0");

    std::vector<float> newDepth (newWidth * newHeight);
    
    for (int newRow = 0; newRow < newHeight; newRow++) {
        int oldRow = std::clamp(static_cast<int>(static_cast<float>(newRow + 0.5f) / newHeight * height - 0.5f), 0, height - 1);
        for (int newCol = 0; newCol < newWidth; newCol++) {
            int oldCol = std::clamp(static_cast<int>(static_cast<float>(newCol + 0.5f) / newWidth * width - 0.5f), 0, width - 1);
            
            int newIndex = newRow * newWidth + newCol;
            int oldIndex = oldRow * width + oldCol;
            
            newDepth[newIndex] = depth[oldIndex];
        }
    }

    width = newWidth;
    height = newHeight;
    depth = newDepth;
}

} // namespace sc3d
} // namespace standard_cyborg
