//
//  ColorImage.hpp
//  StandardCyborgData
//
//  Created by Ricky Reusser on 8/20/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <functional>
#include <vector>

#include <StandardCyborgData/AssertHelper.hpp>
#include <StandardCyborgData/Vec2.hpp>
#include <StandardCyborgData/Vec4.hpp>

#include <StandardCyborgData/Pybind11Defs.hpp>

namespace StandardCyborg {

class ColorImage {

public:
    /** Construct an empty image with 0x0 size */
    ColorImage();
    
    /** Construct an empty image with a size */
    ColorImage(int width, int height);

    /** Construct an image with size and data */
    ColorImage(int width, int height, const std::vector<Vec4>& rgba);
    
    #ifdef PYBIND11_ONLY
    ColorImage(int width, int height, const NPFloat& rgba_);
    #endif // PYBIND11_ONLY
    

    
    // Delete evil constructors in favor of explicitly needing to copy the geometry
    ColorImage(ColorImage&&) = delete;
    ColorImage& operator=(ColorImage&&) = delete;
    ColorImage(ColorImage const& other) = delete;
    ColorImage& operator=(ColorImage const& other) = delete;

    /** Copy another image into this instance */
    void copy(const ColorImage& src);

    /** Reset the size and data of the image */
    void reset(int width, int height, const std::vector<Vec4>& rgba);
    
    /** Reset the size and clear the data of the image */
    void resetSize(int width, int height);
    
    /** Get the image width */
    int getWidth() const;
    
    /** Get the image height */
    int getHeight() const;
    
    /** Get a constant vector of linear-colorspace floating point RGBA data in the range [0-1] */
    const std::vector<Vec4>& getData() const;
    
    /** Get a non-constant vector of linear-colorspace floating point RGBA data in the range [0-1] */
    std::vector<Vec4>& getData();
    
    /** Get a pixel value by column and row */
    inline Vec4 getPixelAtColRow(int col, int row);
    inline Vec4 getPixelAtColRow(int col, int row) const;
    
    /** Set a pixel value by column and row */
    inline void setPixelAtColRow(int col, int row, Vec4 value);
    
    /** Resize a source image into this image's current shape */
    void resizeFrom(const ColorImage& src);
    
    /** Resize an image in-place */
    void resize(int width, int height);
    
    /** Flip an image horizontally in-place */
    void flipX();
    
    /** Flip an image vertically in-place */
    void flipY();
    
    /** Return the pixel location in [0, 1] x [0, 1] texture coordinates */
    inline Vec2 getTexCoordAtColRow(int col, int row) const;
    
    /** Get the approximate size of the image in bytes */
    int getSizeInBytes() const;
    
    /** Get the perceptual ligthness at pixel (row, col) */
    inline float getLightness(int col, int row) const;
    
    /** Mutate the image by passing a lambda function which receives the column, row
      * and current RGBA value and returns a mutated value. */
    void mutatePixelsByColRow(const std::function<Vec4(int col, int row, Vec4 rgba)>& mapFn);
    
    void premultiplyAlpha();
    
private:
    /** Floating point rgba pixel data in a linear colorspace */
    std::vector<Vec4> rgba;

    /** ColorImage width */
    int width;
    
    /** ColorImage height */
    int height;
};

bool operator==(const ColorImage& lhs, const ColorImage& rhs);

inline Vec4 ColorImage::getPixelAtColRow(int col, int row)
{
    SCASSERT(col >= 0 &&
             col < width &&
             row >= 0 &&
             row < height,
             "Row or column out of bounds");
    
    return rgba[row * width + col];
}

inline Vec4 ColorImage::getPixelAtColRow(int col, int row) const
{
    SCASSERT(col >= 0 &&
             col < width &&
             row >= 0 &&
             row < height,
             "Row or column out of bounds");
    
    return rgba[row * width + col];
}

inline void ColorImage::setPixelAtColRow(int col, int row, Vec4 value)
{
    SCASSERT(col >= 0 &&
             col < width &&
             row >= 0 &&
             row < height,
             "Row or column out of bounds");
    
    rgba[row * width + col] = value;
}

inline Vec2 ColorImage::getTexCoordAtColRow(int col, int row) const
{
    return Vec2(
        (col + 0.5f) / width,
        (row + 0.5f) / height
    );
}

float ColorImage::getLightness(int col, int row) const
{
    return Vec3::dot(
        rgba[row * width + col].xyz(),
        { 0.2126, 0.7152, 0.0722 }
    );
}


} // namespace StandardCyborg
