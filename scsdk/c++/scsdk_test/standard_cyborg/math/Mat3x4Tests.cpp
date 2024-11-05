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

#include "standard_cyborg/math/Mat3x4.hpp"
#include "standard_cyborg/math/Mat3x3.hpp"
#include "standard_cyborg/math/Mat4x4.hpp"

namespace math = standard_cyborg::math;
using math::Mat3x4;

TEST(Mat3x4Tests, testEquality) {
    EXPECT_TRUE(Mat3x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12) == Mat3x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12));
    EXPECT_TRUE(Mat3x4(1, 2, 8, 4, 5, 6, 7, 8, 9, 10, 11, 12) != Mat3x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12));
    EXPECT_TRUE(Mat3x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12) != Mat3x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 30, 12));
}

TEST(Mat3x4Tests, testDefaultConstructor) {
    Mat3x4 m0 = {
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12
    };
    
    EXPECT_EQ(Mat3x4{}, Mat3x4(1, 0, 0, 0,
                               0, 1, 0, 0,
                               0, 0, 1, 0));
}

TEST(Mat3x4Tests, testMat3x3WithTranslationConstructor) {
    Mat3x4 m0 ({
        1, 2, 3,
        5, 6, 7,
        9, 10, 11
    }, {
        4, 8, 12
    });
    
    EXPECT_EQ(m0, Mat3x4({
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12
    }));
}

TEST(Mat3x4Tests, testIdentity) {
    EXPECT_EQ(Mat3x4::Identity(), Mat3x4({1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0}));
}

TEST(Mat3x4Tests, testZeros) {
    EXPECT_EQ(Mat3x4::Zeros(), Mat3x4({0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0}));
}

TEST(Mat3x4Tests, testMatMultMat) {
    Mat3x4 m0 = {
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12
    };
    
    Mat3x4 m1 = {
        13, 14, 15, 16,
        17, 18, 19, 20,
        21, 22, 23, 24
    };
    
    Mat3x4 r = m0 * m1;
    
    EXPECT_EQ(r, Mat3x4({
        110, 116, 122, 132,
        314, 332, 350, 376,
        518, 548, 578, 620
    }));
}

TEST(Mat3x4Tests, testMatMultVec) {
    Mat3x4 m0 {
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12
    };
    
    math::Vec3 r = m0 * math::Vec3{3, 4, 5};
    
    EXPECT_EQ(r, math::Vec3(30, 82, 134));
}

TEST(Mat3x4Tests, testInverse) {
    Mat3x4 m {
        1, -1, 2, 3,
        2, 3, -1, 0,
        -2, -3, 2, 1
    };
    
    Mat3x4 mInv(m.inverse());
    
    Mat3x4 expected {
        0.6f, -0.8f, -1.0f, -0.8f,
        -0.4f, 1.2f, 1.0f, 0.2f,
        0.0f, 1.0f, 1.0f, -1.0f
    };
    
    EXPECT_TRUE(Mat3x4::almostEqual(mInv, expected));
}

TEST(Mat3x4Tests, testInPlaceInverse) {
    Mat3x4 m {
        1, -1, 2, 3,
        2, 3, -1, 0,
        -2, -3, 2, 1
    };
    
    Mat3x4 expected {
        0.6f, -0.8f, -1.0f, -0.8f,
        -0.4f, 1.2f, 1.0f, 0.2f,
        0.0f, 1.0f, 1.0f, -1.0f
    };
    
    // Confirm it return the inverse by reference
    EXPECT_TRUE(Mat3x4::almostEqual(m.invert(), expected));
    
    // Confirm the matrix has also been mutated
    EXPECT_TRUE(Mat3x4::almostEqual(m, expected));
}

TEST(Mat3x4Tests, testFromRotationX) {
    Mat3x4 m {
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f
    };
    
    Mat3x4 expected {
        1.0f,  2.0f,  3.0f,  4.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        -5.0f, -6.0f, -7.0f, -8.0f
    };
    
    EXPECT_TRUE(Mat3x4::almostEqual(Mat3x4::fromRotationX(M_PI * 0.5) * m, expected));
}

TEST(Mat3x4Tests, testFromRotationY) {
    Mat3x4 m {
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f
    };
    
    Mat3x4 expected {
        -9.0f, -10.0f, -11.0f, -12.0f,
        5.0f,   6.0f,   7.0f,   8.0f,
        1.0f,   2.0f,   3.0f,   4.0f
    };
    
    // Those floating point sines and cosines aren't floating-point perfect.
    float tolerance = 4.0f * FLT_EPSILON;
    
    EXPECT_TRUE(Mat3x4::almostEqual(Mat3x4::fromRotationY(M_PI * 0.5) * m, expected, tolerance, tolerance));
}

TEST(Mat3x4Tests, testRotateZ) {
    Mat3x4 m {
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f
    };
    
    Mat3x4 expected {
        5,  6,  7,  8,
        -1, -2, -3, -4,
        9, 10, 11, 12
    };
    
    // Those floating point sines and cosines aren't floating-point perfect.
    float tolerance = 2.0f * FLT_EPSILON;
    
    // Assert returns reference to mutated original
    EXPECT_TRUE(Mat3x4::almostEqual(Mat3x4::fromRotationZ(M_PI * 0.5) * m, expected, tolerance, tolerance));
}

TEST(Mat3x4Tests, testTranslate) {
    Mat3x4 m {
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12
    };
    
    Mat3x4 expected {
        1, 2, 3, 5,
        5, 6, 7, 10,
        9, 10, 11, 15
    };
    
    EXPECT_TRUE(Mat3x4::almostEqual(Mat3x4::fromTranslation({1, 2, 3}) * m, expected, FLT_EPSILON, FLT_EPSILON));
}

TEST(Mat3x4Tests, testScale) {
    Mat3x4 m {
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12
    };
    
    Mat3x4 expected {
        2, 4, 6, 8,
        15, 18, 21, 24,
        36, 40, 44, 48
    };
    
    EXPECT_TRUE(Mat3x4::almostEqual(Mat3x4::fromScale({2, 3, 4}) * m, expected, FLT_EPSILON, FLT_EPSILON));
}

TEST(Mat3x4Tests, testFromRowMajorVector) {
    Mat3x4 expected {
        0, 1, 2, 3,
        4, 5, 6, 7,
        8, 9, 10, 11
    };
    
    Mat3x4 m (Mat3x4::fromRowMajorVector({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 }));
    
    EXPECT_TRUE(Mat3x4::almostEqual(m, expected));
}

TEST(Mat3x4Tests, testFromColumnMajorVector) {
    Mat3x4 expected {
        0, 3, 6, 9,
        1, 4, 7, 10,
        2, 5, 8, 11,
    };
    
    Mat3x4 m (Mat3x4::fromColumnMajorVector({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 }));
    
    EXPECT_TRUE(Mat3x4::almostEqual(m, expected));
}

TEST(Mat3x4Tests, testToRowMajorVector) {
    Mat3x4 m {
        0, 1, 2, 3,
        4, 5, 6, 7,
        8, 9, 10, 11
    };
    
    EXPECT_EQ(m.toRowMajorVector(), std::vector<float>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}));
}

TEST(Mat3x4Tests, testToColumnMajorVector) {
    Mat3x4 m {
        0, 3, 6, 9,
        1, 4, 7, 10,
        2, 5, 8, 11,
    };
    
    EXPECT_EQ(m.toColumnMajorVector(), std::vector<float>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}));
}


TEST(Mat3x4Tests, testFromMat3x3) {
    EXPECT_TRUE(Mat3x4::almostEqual(Mat3x4::fromMat3x3({
        1, 2, 3,
        4, 5, 6,
        7, 8, 9
    }), Mat3x4 {
        1, 2, 3, 0,
        4, 5, 6, 0,
        7, 8, 9, 0
    }));
}
