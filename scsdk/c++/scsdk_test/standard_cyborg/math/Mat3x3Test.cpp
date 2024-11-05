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

#include "standard_cyborg/math/Mat3x3.hpp"
#include "standard_cyborg/math/Quaternion.hpp"

namespace math = standard_cyborg::math;
using math::Mat3x3;

TEST(Mat3x3Tests, testDefaultConstructor) {
    
    Mat3x3 m0 = {
        1, 2, 3,
        5, 6, 7,
        9, 10, 11
    };
    
    EXPECT_EQ(
              Mat3x3{},
              Mat3x3(
                     1, 0, 0,
                     0, 1, 0,
                     0, 0, 1));
}

TEST(Mat3x3Tests, testEquality) {
    EXPECT_TRUE(Mat3x3(1, 2, 3, 4, 5, 6, 7, 8, 9) == Mat3x3(1, 2, 3, 4, 5, 6, 7, 8, 9));
    EXPECT_TRUE(Mat3x3(1, 2, 8, 4, 5, 6, 7, 8, 9) != Mat3x3(1, 2, 3, 4, 5, 6, 7, 8, 9));
}

TEST(Mat3x3Tests, testIdentity) {
    EXPECT_EQ(Mat3x3::Identity(), Mat3x3({1, 0, 0,  0, 1, 0,  0, 0, 1}));
}

TEST(Mat3x3Tests, testZeros) {
    EXPECT_EQ(Mat3x3::Zeros(), Mat3x3({0, 0, 0,  0, 0, 0,  0, 0, 0}));
}

TEST(Mat3x3Tests, testMatMultMat) {
    Mat3x3 m0 = {
        1, 2, 3,
        4, 5, 6,
        7, 8, 9
    };
    
    Mat3x3 m1 = {
        13, 14, 15,
        16, 17, 18,
        19, 20, 21
    };
    
    Mat3x3 r = m0 * m1;
    
    EXPECT_EQ(r, Mat3x3({
        102, 108, 114,
        246, 261, 276,
        390, 414, 438
    }));
}

TEST(Mat3x3Tests, testMatMultVec) {
    Mat3x3 m0 = {
        1, 2, 3,
        4, 5, 6,
        7, 8, 9
    };
    
    math::Vec3 r = m0 * math::Vec3{3, 4, 5};
    
    EXPECT_EQ(r, math::Vec3({26, 62, 98}));
}

TEST(Mat3x3Tests, testInverse) {
    Mat3x3 m {
        1, -1, 2,
        2, 3, -1,
        -2, -3, 2
    };
    
    Mat3x3 mInv(m.inverse());
    
    Mat3x3 expected {
        0.6f, -0.8f, -1.0f,
        -0.4f, 1.2f, 1.0f,
        0.0f, 1.0f, 1.0f
    };
    
    EXPECT_TRUE(Mat3x3::almostEqual(mInv, expected));
}

TEST(Mat3x3Tests, testInPlaceInverse) {
    Mat3x3 m {
        1, -1, 2,
        2, 3, -1,
        -2, -3, 2
    };
    
    Mat3x3 expected {
        0.6f, -0.8f, -1.0f,
        -0.4f, 1.2f, 1.0f,
        0.0f, 1.0f, 1.0f
    };
    
    // Confirm it returns the inverse by reference
    EXPECT_TRUE(Mat3x3::almostEqual(m.invert(), expected));
    
    // Confirm it has mutated the original
    EXPECT_TRUE(Mat3x3::almostEqual(m, expected));
}

TEST(Mat3x3Tests, testFromRowMajorVector) {
    Mat3x3 expected {
        0, 1, 2,
        3, 4, 5,
        6, 7, 8
    };
    
    Mat3x3 m (Mat3x3::fromRowMajorVector({ 0, 1, 2, 3, 4, 5, 6, 7, 8 }));
    
    EXPECT_TRUE(Mat3x3::almostEqual(m, expected));
}

TEST(Mat3x3Tests, testFromColumnMajorVector) {
    Mat3x3 expected {
        0, 3, 6,
        1, 4, 7,
        2, 5, 8
    };
    
    Mat3x3 m (Mat3x3::fromColumnMajorVector({ 0, 1, 2, 3, 4, 5, 6, 7, 8 }));
    
    EXPECT_TRUE(Mat3x3::almostEqual(m, expected));
}


TEST(Mat3x3Tests, testToRowMajorVector) {
    Mat3x3 m {
        0, 1, 2,
        3, 4, 5,
        6, 7, 8
    };
    
    EXPECT_EQ(m.toRowMajorVector(), std::vector<float>({0, 1, 2, 3, 4, 5, 6, 7, 8}));
}

TEST(Mat3x3Tests, testToColumnMajorVector) {
    Mat3x3 m {
        0, 3, 6,
        1, 4, 7,
        2, 5, 8
    };
    
    EXPECT_EQ(m.toColumnMajorVector(), std::vector<float>({0, 1, 2, 3, 4, 5, 6, 7, 8}));
}

TEST(Mat3x3Tests, testFromQuaternion) {
    math::Quaternion q {1, -2, 2, -1};
    q.normalize();
    
    Mat3x3 m (Mat3x3::fromQuaternion(q));
    
    EXPECT_TRUE(Mat3x3::almostEqual(m, {
        -0.6f, 0.0f, 0.8f,
        -0.8f, 0.0f, -0.6f,
        0.0f, -1.0f, 0.0f
    }));
}

TEST(Mat3x3Tests, testFromDiagonal) {
    Mat3x3 m (Mat3x3::fromDiagonal({1, 2, 3}));
    EXPECT_EQ(m, Mat3x3(1, 0, 0,  0, 2, 0,  0, 0, 3));
}


TEST(Mat3x3Tests, testDiagonal) {
    Mat3x3 m (1, 0, 0,  0, 2, 0,  0, 0, 3);
    EXPECT_EQ(m.diagonal(), math::Vec3(1, 2, 3));
}

TEST(Mat3x3Tests, testTrace) {
    Mat3x3 m (1, 2, 3, 4, 5, 6, 7, 8, 9);
    EXPECT_EQ(m.trace(), 15.0f);
}

