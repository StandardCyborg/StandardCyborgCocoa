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


#include <doctest/doctest.h>

#include <standard_cyborg/math/Vec4.hpp>
#include <standard_cyborg/sc3d/ColorImage.hpp>

using standard_cyborg::sc3d::ColorImage;

namespace math = standard_cyborg::math;
using math::Vec4;

TEST_CASE("ImageTests.testConstructor") {
    ColorImage image;
    CHECK_EQ(image.getWidth(), 0);
    CHECK_EQ(image.getHeight(), 0);
    CHECK_EQ(image.getData().size(), 0);
}

TEST_CASE("ImageTests.testConstructorWithSize") {
    ColorImage image (2, 2);
    CHECK_EQ(image.getWidth(), 2);
    CHECK_EQ(image.getHeight(), 2);
    CHECK_EQ(image.getData().size(), 4);
}

TEST_CASE("ImageTests.testConstructorWithSizeAndData") {
    ColorImage image (1, 1, {{1, 0, 1, 0}});
    CHECK_EQ(image.getWidth(), 1);
    CHECK_EQ(image.getHeight(), 1);
    CHECK(Vec4::almostEqual(image.getData()[0], {1, 0, 1, 0}));
}

TEST_CASE("ImageTests.testGetPixel") {
    ColorImage image (2, 3, {
        {1, 0, 0, 1}, {1, 1, 0, 1},
        {0, 1, 0, 1}, {0, 1, 1, 1},
        {0, 0, 1, 1}, {1, 0, 1, 1}
    });
    
    CHECK_EQ(image.getPixelAtColRow(0, 0), Vec4({1, 0, 0, 1}));
    CHECK_EQ(image.getPixelAtColRow(1, 0), Vec4({1, 1, 0, 1}));
    
    CHECK_EQ(image.getPixelAtColRow(0, 1), Vec4({0, 1, 0, 1}));
    CHECK_EQ(image.getPixelAtColRow(1, 1), Vec4({0, 1, 1, 1}));
    
    CHECK_EQ(image.getPixelAtColRow(0, 2), Vec4({0, 0, 1, 1}));
    CHECK_EQ(image.getPixelAtColRow(1, 2), Vec4({1, 0, 1, 1}));
}

TEST_CASE("ImageTests.testGetPixelConst") {
    const ColorImage image (2, 3, {
        {1, 0, 0, 1}, {1, 1, 0, 1},
        {0, 1, 0, 1}, {0, 1, 1, 1},
        {0, 0, 1, 1}, {1, 0, 1, 1}
    });
    
    CHECK_EQ(image.getPixelAtColRow(0, 0), Vec4({1, 0, 0, 1}));
    CHECK_EQ(image.getPixelAtColRow(1, 0), Vec4({1, 1, 0, 1}));
    
    CHECK_EQ(image.getPixelAtColRow(0, 1), Vec4({0, 1, 0, 1}));
    CHECK_EQ(image.getPixelAtColRow(1, 1), Vec4({0, 1, 1, 1}));
    
    CHECK_EQ(image.getPixelAtColRow(0, 2), Vec4({0, 0, 1, 1}));
    CHECK_EQ(image.getPixelAtColRow(1, 2), Vec4({1, 0, 1, 1}));
}

TEST_CASE("ImageTests.testGetLightness") {
    const ColorImage image (2, 2, {
        {1, 1, 1, 1}, {0, 0, 0, 1},
        {0, 0, 0, 1}, {1, 1, 1, 1},
        
    });
    
    CHECK_EQ(image.getLightness(0,0), 1.0f);
    CHECK_EQ(image.getLightness(1,0), 0.0f);
    
}


TEST_CASE("ImageTests.testSetPixel") {
    ColorImage image (2, 3);
    
    image.setPixelAtColRow(0, 0, {1, 0, 0, 1});
    image.setPixelAtColRow(1, 0, {1, 1, 0, 1});
    
    image.setPixelAtColRow(0, 1, {0, 1, 0, 1});
    image.setPixelAtColRow(1, 1, {0, 1, 1, 1});
    
    image.setPixelAtColRow(0, 2, {0, 0, 1, 1});
    image.setPixelAtColRow(1, 2, {1, 0, 1, 1});
    
    
    std::vector<Vec4> expectedPixels = {
        {1, 0, 0, 1}, {1, 1, 0, 1},
        {0, 1, 0, 1}, {0, 1, 1, 1},
        {0, 0, 1, 1}, {1, 0, 1, 1}
    };
    
    for (int i = 0; i < image.getWidth() * image.getHeight(); i++) {
        CHECK_EQ(image.getData()[i], expectedPixels[i]);
    }
}

TEST_CASE("ImageTests.testCopy") {
    ColorImage i1 (2, 3, {
        {1, 0, 0, 1}, {1, 1, 0, 1},
        {0, 1, 0, 1}, {0, 1, 1, 1},
        {0, 0, 1, 1}, {1, 0, 1, 1}
    });
    
    ColorImage i2;
    i2.copy(i1);
    
    CHECK_EQ(i2.getWidth(), 2);
    CHECK_EQ(i2.getHeight(), 3);
    
    for (int i = 0; i < i1.getWidth() * i1.getHeight(); i++) {
        CHECK_EQ(i1.getData()[i], i2.getData()[i]);
    }
}

TEST_CASE("ImageTests.testResize") {
    ColorImage src (2, 3, {
        {1, 0, 0, 1}, {1, 1, 0, 1},
        {0, 1, 0, 1}, {0, 1, 1, 1},
        {0, 0, 1, 1}, {1, 0, 1, 1}
    });
    
    ColorImage dst (4, 6);
    
    dst.resizeFrom(src);
}

TEST_CASE("ImageTests.testFlipYOddSize") {
    ColorImage image (2, 3, {
        {1, 0, 0, 1}, {1, 1, 0, 1},
        {0, 1, 0, 1}, {0, 1, 1, 1},
        {0, 0, 1, 1}, {1, 0, 1, 1}
    });
    
    image.flipY();
    
    ColorImage expectedImage (2, 3, {
        {0, 0, 1, 1}, {1, 0, 1, 1},
        {0, 1, 0, 1}, {0, 1, 1, 1},
        {1, 0, 0, 1}, {1, 1, 0, 1}
    });
    
    CHECK(image == expectedImage);
}

TEST_CASE("ImageTests.testFlipYEvenSize") {
    ColorImage image (2, 4, {
        {1, 0, 0, 1}, {1, 1, 0, 1},
        {1, 1, 1, 1}, {0, 0, 0, 1},
        {0, 1, 0, 1}, {0, 1, 1, 1},
        {0, 0, 1, 1}, {1, 0, 1, 1}
    });
    
    image.flipY();
    
    ColorImage expectedImage (2, 4, {
        {0, 0, 1, 1}, {1, 0, 1, 1},
        {0, 1, 0, 1}, {0, 1, 1, 1},
        {1, 1, 1, 1}, {0, 0, 0, 1},
        {1, 0, 0, 1}, {1, 1, 0, 1}
    });
    
    CHECK(image == expectedImage);
}

TEST_CASE("ImageTests.testFlipXEvenSize") {
    ColorImage image (2, 4, {
        {1, 0, 0, 1}, {1, 1, 0, 1},
        {1, 1, 1, 1}, {0, 0, 0, 1},
        {0, 1, 0, 1}, {0, 1, 1, 1},
        {0, 0, 1, 1}, {1, 0, 1, 1}
    });
    
    image.flipX();
    
    ColorImage expectedImage (2, 4, {
        {1, 1, 0, 1}, {1, 0, 0, 1},
        {0, 0, 0, 1},{1, 1, 1, 1},
        {0, 1, 1, 1}, {0, 1, 0, 1},
        {1, 0, 1, 1}, {0, 0, 1, 1}
    });
    
    CHECK(image == expectedImage);
}

TEST_CASE("ImageTests.testFlipXOddSize") {
    ColorImage image (3, 2, {
        {1, 0, 0, 1}, {1, 1, 0, 1}, {1, 0, 1, 1},
        {1, 1, 1, 1}, {0, 0, 0, 1}, {1, 0, 1, 1},
    });
    
    image.flipX();
    
    ColorImage expectedImage (3, 2, {
        {1, 0, 1, 1}, {1, 1, 0, 1}, {1, 0, 0, 1},
        {1, 0, 1, 1}, {0, 0, 0, 1}, {1, 1, 1, 1}
    });
    
    CHECK(image == expectedImage);
}

TEST_CASE("ImageTests.testGetSizeInBytes") {
    ColorImage image (3, 2, {
        {1, 0, 0, 1}, {1, 1, 0, 1}, {1, 0, 1, 1},
        {1, 1, 1, 1}, {0, 0, 0, 1}, {1, 0, 1, 1},
    });
    
    CHECK(image.getSizeInBytes() == 3 * 2 * 4 * sizeof(float));
}


TEST_CASE("ImageTests.testPremultiplyAlpha") {
    ColorImage image (2, 2, {
        {1, 0, 0, 0.5}, {1, 1, 0, 0.75},
        {1, 1, 1, 0.25}, {0, 0, 0, 1.0}
    });
    
    image.premultiplyAlpha();
    
    ColorImage expectedImage (2, 2, {
        {0.5, 0, 0, 0.5}, {0.75, 0.75, 0, 0.75},
        {0.25, 0.25, 0.25, 0.25}, {0, 0, 0, 1.0}
    });
    
    CHECK(image == expectedImage);
}

TEST_CASE("ImageTests.testInPlaceResize") {
    ColorImage src (2, 3, {
        {1, 0, 0, 1}, {1, 1, 0, 1},
        {0, 1, 0, 1}, {0, 1, 1, 1},
        {0, 0, 1, 1}, {1, 0, 1, 1}
    });
    
    src.resize(4, 6);
    
    CHECK_EQ(src.getWidth(), 4);
    CHECK_EQ(src.getHeight(), 6);
}

TEST_CASE("ImageTests.testGetTexCoord") {
    using math::Vec2;
    
    ColorImage src (2, 3);
    
    CHECK(Vec2::almostEqual(src.getTexCoordAtColRow(0, 0), Vec2(1.0 / 4.0, 1.0 / 6.0)));
    CHECK(Vec2::almostEqual(src.getTexCoordAtColRow(1, 0), Vec2(3.0 / 4.0, 1.0 / 6.0)));
    
    CHECK(Vec2::almostEqual(src.getTexCoordAtColRow(0, 1), Vec2(1.0 / 4.0, 3.0 / 6.0)));
    CHECK(Vec2::almostEqual(src.getTexCoordAtColRow(1, 1), Vec2(3.0 / 4.0, 3.0 / 6.0)));
    
    CHECK(Vec2::almostEqual(src.getTexCoordAtColRow(0, 2), Vec2(1.0 / 4.0, 5.0 / 6.0)));
    CHECK(Vec2::almostEqual(src.getTexCoordAtColRow(1, 2), Vec2(3.0 / 4.0, 5.0 / 6.0)));
}

TEST_CASE("ImageTests.testMutatePixels") {
    ColorImage src(2, 3, {
        {1, 0, 0, 1}, {1, 1, 0, 1},
        {0, 1, 0, 1}, {0, 1, 1, 1},
        {0, 0, 1, 1}, {1, 0, 1, 1}
    });
    
    src.mutatePixelsByColRow([](int col, int row, Vec4 rgba) {
        return Vec4(col, row, rgba.z > 0.5 ? 1 : 0, 0);
    });
    
    
    ColorImage expectedImage(2, 3, {
        {0, 0, 0, 0}, {1, 0, 0, 0},
        {0, 1, 0, 0}, {1, 1, 1, 0},
        {0, 2, 1, 0}, {1, 2, 1, 0}
    });
    
    CHECK(src == expectedImage);
}

TEST_CASE("ImageTests.testResetSize") {
    ColorImage img;
    
    img.resetSize(2,3);
    
    CHECK_EQ(img.getWidth(), 2);
    CHECK_EQ(img.getHeight(), 3);
}
