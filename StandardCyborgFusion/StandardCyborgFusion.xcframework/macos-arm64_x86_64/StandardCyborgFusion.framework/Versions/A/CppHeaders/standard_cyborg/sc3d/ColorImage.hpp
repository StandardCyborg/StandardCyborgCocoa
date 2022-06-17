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

#include "standard_cyborg/util/AssertHelper.hpp"
#include "standard_cyborg/math/Vec2.hpp"
#include "standard_cyborg/math/Vec4.hpp"

namespace standard_cyborg {
namespace sc3d {

// Convert from linear gamma (ColorImage native) to sRGB gamma (native for
// jpeg, png, others).  `rgb` is an RGB color with each color value in 
// the unit interval [0, 1].  Here for consitency with other code and 
// simplicity, we use a fast approximation.
inline math::Vec3 LinearToApproximateSRGB(const math::Vec3 &rgb) {
    return math::Vec3::pow(rgb, 1.0 / 2.2);
}

// Convert from sRGB gamma (native for jpeg, png, others) to linear gamma
// (native for ColorImage).  `rgb` is an RGB color with each color value in 
// the unit interval [0, 1].  Here for consitency with other code and 
// simplicity, we use a fast approximation.
inline math::Vec3 ApproximateSRGBGammaToLinear(const math::Vec3 &rgb) {
    return math::Vec3::pow(rgb, 2.2);
}

/**
 * ColorImage is a 4-channel, RGBA image with floating-point color
 * values in the unit interval ([0, 1]) and linear gamma.
 */
class ColorImage {
public:
    /** Construct an empty image with 0x0 size */
    ColorImage();
    
    /** Construct an empty image with a size */
    ColorImage(int width, int height);

    /** Construct an image with size and data */
    ColorImage(int width, int height, const std::vector<math::Vec4>& rgba);
    
    // // Delete evil constructors in favor of explicitly needing to copy the geometry  wat????????????????????????????
    // ColorImage(ColorImage&&) = delete;
    // ColorImage& operator=(ColorImage&&) = delete;
    // ColorImage(ColorImage const& other) = delete;
    // ColorImage& operator=(ColorImage const& other) = delete;

    /** Copy another image into this instance */
    void copy(const ColorImage& src);

    /** Move another image into this instance */
    void move(ColorImage&& src);

    /** Reset the size and data of the image */
    void reset(int width, int height, const std::vector<math::Vec4>& rgba);
    
    /** Reset the size and clear the data of the image */
    void resetSize(int width, int height);
    
    /** Get the image width */
    int getWidth() const;
    
    /** Get the image height */
    int getHeight() const;
    
    /** Get a constant vector of linear-colorspace floating point RGBA data in the range [0-1] */
    const std::vector<math::Vec4>& getData() const;
    
    /** Get a non-constant vector of linear-colorspace floating point RGBA data in the range [0-1] */
    std::vector<math::Vec4>& getData();
    
    /** Get a pixel value by column and row */
    inline math::Vec4 getPixelAtColRow(int col, int row);
    inline math::Vec4 getPixelAtColRow(int col, int row) const;
    
    /** Set a pixel value by column and row */
    inline void setPixelAtColRow(int col, int row, math::Vec4 value);
    
    /** Resize a source image into this image's current shape */
    void resizeFrom(const ColorImage& src);
    
    /** Resize an image in-place */
    void resize(int width, int height);
    
    /** Flip an image horizontally in-place */
    void flipX();
    
    /** Flip an image vertically in-place */
    void flipY();
    
    /** Return the pixel location in [0, 1] x [0, 1] texture coordinates */
    inline math::Vec2 getTexCoordAtColRow(int col, int row) const;
    
    /** Get the approximate size of the image in bytes */
    int getSizeInBytes() const;
    
    /** Get the perceptual ligthness at pixel (row, col) */
    inline float getLightness(int col, int row) const;
    
    /** Mutate the image by passing a lambda function which receives the column, row
      * and current RGBA value and returns a mutated value. */
    void mutatePixelsByColRow(const std::function<math::Vec4(int col, int row, math::Vec4 rgba)>& mapFn);
    
    void premultiplyAlpha();
    
    std::string getFrame() const { return frame; }
    void setFrame(const std::string &f) { frame = f; }

private:
    std::string frame;

    /** Floating point rgba pixel data in a linear colorspace */
    std::vector<math::Vec4> rgba;

    /** ColorImage width */
    int width;
    
    /** ColorImage height */
    int height;
};

bool operator==(const ColorImage& lhs, const ColorImage& rhs);

inline math::Vec4 ColorImage::getPixelAtColRow(int col, int row)
{
    SCASSERT(col >= 0 &&
             col < width &&
             row >= 0 &&
             row < height,
             "Row or column out of bounds");
    
    return rgba[row * width + col];
}

inline math::Vec4 ColorImage::getPixelAtColRow(int col, int row) const
{
    SCASSERT(col >= 0 &&
             col < width &&
             row >= 0 &&
             row < height,
             "Row or column out of bounds");
    
    return rgba[row * width + col];
}

inline void ColorImage::setPixelAtColRow(int col, int row, math::Vec4 value)
{
    SCASSERT(col >= 0 &&
             col < width &&
             row >= 0 &&
             row < height,
             "Row or column out of bounds");
    
    rgba[row * width + col] = value;
}

inline math::Vec2 ColorImage::getTexCoordAtColRow(int col, int row) const
{
    return math::Vec2(
        (col + 0.5f) / width,
        (row + 0.5f) / height
    );
}

float ColorImage::getLightness(int col, int row) const
{
    return math::Vec3::dot(
        rgba[row * width + col].xyz(),
        { 0.2126, 0.7152, 0.0722 }
    );
}


} // namespace sc3d
} // namespace standard_cyborg
