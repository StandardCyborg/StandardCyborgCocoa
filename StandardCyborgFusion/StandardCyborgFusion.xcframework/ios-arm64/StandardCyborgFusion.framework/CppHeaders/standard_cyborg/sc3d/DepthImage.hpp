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

#pragma once

#include <functional>
#include <vector>

#include "standard_cyborg/math/Vec2.hpp"
#include "standard_cyborg/util/AssertHelper.hpp"

namespace standard_cyborg {
namespace sc3d {

/**
 * DepthImage is a 1-channel, floating-value image, where values are
 * depths in meters.
 */
class DepthImage {
public:
    /** Construct an empty image with 0x0 size */
    DepthImage() : width(0), height(0) { }
    
    /** Construct an empty image with a size */
    DepthImage(int width, int height);

    /** Construct an image with size and data */
    DepthImage(int width, int height, const std::vector<float>& depth);
    
    // // Delete evil constructors in favor of explicitly needing to copy the geometry wat????????????????????????????
    // DepthImage(DepthImage&&) = delete;
    // DepthImage& operator=(DepthImage&&) = delete;
    // DepthImage(DepthImage const& other) = delete;
    // DepthImage& operator=(DepthImage const& other) = delete;

    /** Copy another image into this instance */
    void copy(const DepthImage& src);

    /** Move another image into this instance */
    void move(DepthImage&& src);

    /** Reset the size and data of the image */
    void reset(int width, int height, const std::vector<float>& depth);
    
    /** Reset the size and clear the data of the image */
    void resetSize(int width, int height);
    
    /** Get the image width */
    int getWidth() const;
    
    /** Get the image height */
    int getHeight() const;
    
    /** Resize a source image into this image's current shape */
    void resizeFrom(const DepthImage& src);
    
    /** Resize an image in-place using nearest-neighbor sampling */
    void resize(int width, int height);
    
    /** Get a constant vector of linear-colorspace floating point RGBA data in the range [0-1] */
    const std::vector<float>& getData() const;
    
    /** Get a non-constant vector of linear-colorspace floating point RGBA data in the range [0-1] */
    std::vector<float>& getData();
    
    /** Get a pixel value by column and row */
    inline float& getPixelAtColRow(int col, int row);
    
    /** Get a pixel value by column and row */
    inline const float& getPixelAtColRow(int col, int row) const;
    
    /** Set a pixel value by column and row */
    inline void setPixelAtColRow(int col, int row, float value);
    
    /** Iterate over the pixels of a depth frame*/
    void forEachPixelAtColRow(const std::function<void(int col, int row, float depth)>& fn) const;
    
    /** Mutate the image by passing a lambda function which receives the column, row
      * and current depth value and returns a mutated value. */
    void mutatePixelsByColRow(const std::function<float(int col, int row, float value)>& mapFn);
    
    /** Flip an image horizontally, writing the new result in-place */
    void flipX();
    
    /** Flip an image vertically, writing the new result in-place */
    void flipY();

    /** Return the pixel location in [0, 1] x [0, 1] texture coordinates */
    inline math::Vec2 getTexCoordAtColRow(int col, int row) const;
    
    /** Get the approximate size of the image in bytes */
    int getSizeInBytes() const;

    std::string getFrame() const { return frame; }

    void setFrame(const std::string &f) { frame = f; }

private:
    std::string frame;

    /** Floating point depth pixel data in meters */
    std::vector<float> depth;

    /** DepthImage width */
    int width;
    
    /** DepthImage height */
    int height;
};

bool operator==(const DepthImage& lhs, const DepthImage& rhs);

inline float& DepthImage::getPixelAtColRow(int col, int row)
{
    SCASSERT(col >= 0, "Column out of bounds");
    SCASSERT(col < width, "Column out of bounds");
    SCASSERT(row >= 0, "Row out of bounds");
    SCASSERT(row < height, "Row out of bounds");
    
    return depth[row * width + col];
}

inline const float& DepthImage::getPixelAtColRow(int col, int row) const
{
    SCASSERT(col >= 0, "Column out of bounds");
    SCASSERT(col < width, "Column out of bounds");
    SCASSERT(row >= 0, "Row out of bounds");
    SCASSERT(row < height, "Row out of bounds");
    
    return depth[row * width + col];
}

inline void DepthImage::setPixelAtColRow(int col, int row, float value)
{
    SCASSERT(col >= 0, "Column out of bounds");
    SCASSERT(col < width, "Column out of bounds");
    SCASSERT(row >= 0, "Row out of bounds");
    SCASSERT(row < height, "Row out of bounds");
    
    depth[row * width + col] = value;
}

inline math::Vec2 DepthImage::getTexCoordAtColRow(int col, int row) const
{
    return math::Vec2(
        (col + 0.5f) / width,
        (row + 0.5f) / height
    );
}


} // namespace sc3d
} // namespace standard_cyborg
