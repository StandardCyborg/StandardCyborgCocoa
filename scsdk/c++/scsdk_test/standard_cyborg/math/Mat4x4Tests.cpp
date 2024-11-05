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
#include "standard_cyborg/math/Mat4x4.hpp"
#include "standard_cyborg/math/Vec3.hpp"

namespace math = standard_cyborg::math;
using math::Mat4x4;

TEST(Mat4x4Tests, testEquality) {

    EXPECT_TRUE(Mat4x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16) == Mat4x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16));
    EXPECT_TRUE(Mat4x4(1, 2, 8, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16) != Mat4x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16));
    EXPECT_TRUE(Mat4x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16) != Mat4x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 30, 12, 13, 14, 15, 16));
}

TEST(Mat4x4Tests, testDefaultConstructor) {
    EXPECT_EQ(Mat4x4{}, Mat4x4(1, 0, 0, 0,
                                    0, 1, 0, 0,
                                    0, 0, 1, 0,
                                    0, 0, 0, 1));
    
}

TEST(Mat4x4Tests, testIdentity) {
    EXPECT_EQ(Mat4x4::Identity(), Mat4x4({1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1}));
}

TEST(Mat4x4Tests, testZeros) {
    EXPECT_EQ(Mat4x4::Zeros(), Mat4x4({0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0}));
}

TEST(Mat4x4Tests, testMatMultMat) {
    Mat4x4 m0 = {
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    };
    
    Mat4x4 m1 = {
        13, 14, 15, 16,
        17, 18, 19, 20,
        21, 22, 23, 24,
        25, 26, 27, 28
    };
    
    Mat4x4 r = m0 * m1;
    
    EXPECT_EQ(r, Mat4x4({
        210, 220, 230, 240,
        514, 540, 566, 592,
        818, 860, 902, 944,
        1122, 1180, 1238, 1296
    }));
}

TEST(Mat4x4Tests, testMatMultVec) {
    Mat4x4 m0 {
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    };
    
    math::Vec3 r = m0 * math::Vec3{3, 4, 5};
    
    EXPECT_TRUE(math::Vec3::almostEqual(r, math::Vec3{0.16129f, 0.44086f, 0.72043f}, 1e-5, 1e-5));
}

TEST(Mat4x4Tests, testInverse) {
    Mat4x4 m {
        1, -1, 2, 3,
        2, 3, -1, 0,
        -2, -3, 2, 1,
        2, 2, -3, -1
    };
    
    Mat4x4 mInv(m.inverse());
    
    Mat4x4 expected {
        1,    -3,    -4,    -1,
        -0.5,  1.75,  1.75,  0.25,
        0.5, -1.75, -2.75, -1.25,
        -0.5,  2.75,  3.75,  1.25
    };
    
    EXPECT_TRUE(Mat4x4::almostEqual(mInv, expected));
}

TEST(Mat4x4Tests, testSingularInverse) {
    Mat4x4 m {
        1, 1, 1, 1,
        2, 2, 2, 2,
        -2, -3, 2, 1,
        2, 2, -3, -1
    };
    
    Mat4x4 mInv(m.inverse());
    
    EXPECT_TRUE(std::isnan(mInv.m00));
    EXPECT_TRUE(std::isnan(mInv.m01));
    EXPECT_TRUE(std::isnan(mInv.m02));
    EXPECT_TRUE(std::isnan(mInv.m03));
    
    EXPECT_TRUE(std::isnan(mInv.m10));
    EXPECT_TRUE(std::isnan(mInv.m11));
    EXPECT_TRUE(std::isnan(mInv.m12));
    EXPECT_TRUE(std::isnan(mInv.m13));
    
    EXPECT_TRUE(std::isnan(mInv.m20));
    EXPECT_TRUE(std::isnan(mInv.m21));
    EXPECT_TRUE(std::isnan(mInv.m22));
    EXPECT_TRUE(std::isnan(mInv.m23));
    
    EXPECT_TRUE(std::isnan(mInv.m30));
    EXPECT_TRUE(std::isnan(mInv.m31));
    EXPECT_TRUE(std::isnan(mInv.m32));
    EXPECT_TRUE(std::isnan(mInv.m33));
}

TEST(Mat4x4Tests, testInPlaceInverse) {
    Mat4x4 m {
        1, -1, 2, 3,
        2, 3, -1, 0,
        -2, -3, 2, 1,
        2, 2, -3, -1
    };
    
    Mat4x4 expected {
            1,    -3,    -4,    -1,
         -0.5,  1.75,  1.75,  0.25,
          0.5, -1.75, -2.75, -1.25,
         -0.5,  2.75,  3.75,  1.25
    };
    
    // Confirm it return the inverse by reference
    EXPECT_TRUE(Mat4x4::almostEqual(m.invert(), expected));
    
    // Confirm the matrix has also been mutated
    EXPECT_TRUE(Mat4x4::almostEqual(m, expected));
}

TEST(Mat4x4Tests, testInPlaceSingularInverse) {
    Mat4x4 m {
        1, 1, 1, 1,
        2, 2, 2, 2,
        -2, -3, 2, 1,
        2, 2, -3, -1
    };
    
    m.invert();
    
    EXPECT_TRUE(std::isnan(m.m00));
    EXPECT_TRUE(std::isnan(m.m01));
    EXPECT_TRUE(std::isnan(m.m02));
    EXPECT_TRUE(std::isnan(m.m03));
    
    EXPECT_TRUE(std::isnan(m.m10));
    EXPECT_TRUE(std::isnan(m.m11));
    EXPECT_TRUE(std::isnan(m.m12));
    EXPECT_TRUE(std::isnan(m.m13));
    
    EXPECT_TRUE(std::isnan(m.m20));
    EXPECT_TRUE(std::isnan(m.m21));
    EXPECT_TRUE(std::isnan(m.m22));
    EXPECT_TRUE(std::isnan(m.m23));
    
    EXPECT_TRUE(std::isnan(m.m30));
    EXPECT_TRUE(std::isnan(m.m31));
    EXPECT_TRUE(std::isnan(m.m32));
    EXPECT_TRUE(std::isnan(m.m33));
}

TEST(Mat4x4Tests, testFromRowMajorVector) {
    Mat4x4 expected {
        0, 1, 2, 3,
        4, 5, 6, 7,
        8, 9, 10, 11,
        12, 13, 14, 15
    };
    
    Mat4x4 m (Mat4x4::fromRowMajorVector({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }));
    
    EXPECT_TRUE(Mat4x4::almostEqual(m, expected));
}

TEST(Mat4x4Tests, testFromColumnMajorVector) {
    Mat4x4 expected {
        0, 4, 8, 12,
        1, 5, 9, 13,
        2, 6, 10, 14,
        3, 7, 11, 15
    };
    
    Mat4x4 m (Mat4x4::fromColumnMajorVector({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }));
    
    EXPECT_TRUE(Mat4x4::almostEqual(m, expected));
}

TEST(Mat4x4Tests, testToRowMajorVector) {
    Mat4x4 m {
        0, 1, 2, 3,
        4, 5, 6, 7,
        8, 9, 10, 11,
        12, 13, 14, 15
    };
    
    EXPECT_EQ(m.toRowMajorVector(), std::vector<float>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}));
}

TEST(Mat4x4Tests, testToColumnMajorVector) {
    Mat4x4 m {
        0, 4, 8, 12,
        1, 5, 9, 13,
        2, 6, 10, 14,
        3, 7, 11, 15
    };
    
    EXPECT_EQ(m.toColumnMajorVector(), std::vector<float>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}));
}
