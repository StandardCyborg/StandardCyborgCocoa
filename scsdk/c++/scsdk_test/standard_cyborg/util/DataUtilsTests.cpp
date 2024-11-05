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

/*
 #import <XCTest/XCTest.h>
 #include <iostream>
 #include <vector>
 
 #include <StandardCyborgData/DataUtils.hpp>
 #include <StandardCyborgData/DebugHelpers.hpp>
 #include <StandardCyborgData/Vec3.hpp>
 #include <StandardCyborgData/Face3.hpp>
 
 using standard_cyborg::Face3;
 
 namespace math = standard_cyborg::math;
 using math::Vec3;
 
 @interface DataUtilsTests : XCTestCase
 
 @end
 
 @implementation DataUtilsTests
 
 // MARK: Vec3 Tests
 */

#include <gtest/gtest.h>

#include <vector>

#include "standard_cyborg/util/DataUtils.hpp"
#include "standard_cyborg/sc3d/Face3.hpp"

using standard_cyborg::sc3d::Face3;

namespace math = standard_cyborg::math;
using math::Vec3;

TEST(DataUtilsTests, testVec3ArrayToEigen3Xf) {
    std::vector<Vec3> data {
        { 1.0f, 2.0f, 3.0f },
        { 4.0f, 5.0f, 6.0f},
        { 7.0f, 8.0f, 9.0f },
        { 10.0f, 11.0f, 12.0f },
    };
    
    Eigen::Ref<Eigen::Matrix3Xf> matrix (standard_cyborg::toMatrix3Xf(data));
    
    Eigen::Matrix3Xf expected (3, 4);
    expected <<
    1.0f, 4.0f, 7.0f, 10.0f,
    2.0f, 5.0f, 8.0f, 11.0f,
    3.0f, 6.0f, 9.0f, 12.0f;
    
    EXPECT_EQ(matrix, expected);
    
    EXPECT_EQ(matrix(0, 0), 1.0f);
    EXPECT_EQ(matrix(1, 0), 2.0f);
    EXPECT_EQ(matrix(2, 0), 3.0f);
    EXPECT_EQ(matrix(0, 1), 4.0f);
    EXPECT_EQ(matrix(1, 1), 5.0f);
    EXPECT_EQ(matrix(2, 1), 6.0f);
    EXPECT_EQ(matrix(0, 2), 7.0f);
    EXPECT_EQ(matrix(1, 2), 8.0f);
    EXPECT_EQ(matrix(2, 2), 9.0f);
    EXPECT_EQ(matrix(0, 3), 10.0f);
    EXPECT_EQ(matrix(1, 3), 11.0f);
    EXPECT_EQ(matrix(2, 3), 12.0f);
}

TEST(DataUtilsTests, testVec3ArrayToEigen3XfDataMutation) {
    std::vector<Vec3> data {
        { 1.0f, 2.0f, 3.0f },
        { 4.0f, 5.0f, 6.0f},
        { 7.0f, 8.0f, 9.0f },
        { 10.0f, 11.0f, 12.0f },
    };
    
    Eigen::Ref<Eigen::Matrix3Xf> matrix {standard_cyborg::toMatrix3Xf(data)};
    
    // Sanity-check it starts out correct
    EXPECT_EQ(data[2].y, 8.0f);
    
    // Mutate an entry (remember, 3Xf has three *rows* and X *columns*!)
    matrix(1, 2) = 100.0f;
    
    // Assert the matrix has (trivially) changed
    EXPECT_EQ(matrix(1, 2), 100.0f);
    
    // Assert the underlying data has changed
    EXPECT_EQ(data[2].y, 100.0f);
}

TEST(DataUtilsTests, testConstVec3ArrayToEigen3Xf) {
    const std::vector<Vec3> data {
        { 1.0f, 2.0f, 3.0f },
        { 4.0f, 5.0f, 6.0f},
        { 7.0f, 8.0f, 9.0f },
        { 10.0f, 11.0f, 12.0f },
    };
    
    const Eigen::Ref<const Eigen::Matrix3Xf> matrix {standard_cyborg::toMatrix3Xf(data)};
    
    // woot, "Expression is not assignable"
    // matrix(1, 2) = 100.0f;
    
    EXPECT_EQ(matrix(0, 0), 1.0f);
    EXPECT_EQ(matrix(1, 0), 2.0f);
    EXPECT_EQ(matrix(2, 0), 3.0f);
    EXPECT_EQ(matrix(0, 1), 4.0f);
    EXPECT_EQ(matrix(1, 1), 5.0f);
    EXPECT_EQ(matrix(2, 1), 6.0f);
    EXPECT_EQ(matrix(0, 2), 7.0f);
    EXPECT_EQ(matrix(1, 2), 8.0f);
    EXPECT_EQ(matrix(2, 2), 9.0f);
    EXPECT_EQ(matrix(0, 3), 10.0f);
    EXPECT_EQ(matrix(1, 3), 11.0f);
    EXPECT_EQ(matrix(2, 3), 12.0f);
}

TEST(DataUtilsTests, testVec3ArrayToEigenX3f) {
    std::vector<Vec3> data {
        { 1.0f, 2.0f, 3.0f },
        { 4.0f, 5.0f, 6.0f},
        { 7.0f, 8.0f, 9.0f },
        { 10.0f, 11.0f, 12.0f },
    };
    
    auto matrix = standard_cyborg::toMatrixX3f(data);
    
    EXPECT_EQ(matrix(0, 0), 1.0f);
    EXPECT_EQ(matrix(0, 1), 2.0f);
    EXPECT_EQ(matrix(0, 2), 3.0f);
    EXPECT_EQ(matrix(1, 0), 4.0f);
    EXPECT_EQ(matrix(1, 1), 5.0f);
    EXPECT_EQ(matrix(1, 2), 6.0f);
    EXPECT_EQ(matrix(2, 0), 7.0f);
    EXPECT_EQ(matrix(2, 1), 8.0f);
    EXPECT_EQ(matrix(2, 2), 9.0f);
    EXPECT_EQ(matrix(3, 0), 10.0f);
    EXPECT_EQ(matrix(3, 1), 11.0f);
    EXPECT_EQ(matrix(3, 2), 12.0f);
}

TEST(DataUtilsTests, testVec3ArrayToEigenX3fDataMutation) {
    std::vector<Vec3> data {
        { 1.0f, 2.0f, 3.0f },
        { 4.0f, 5.0f, 6.0f},
        { 7.0f, 8.0f, 9.0f },
        { 10.0f, 11.0f, 12.0f },
    };
    
    // I would, of course, *highly* recomment `auto` here. I have elected instead to be
    // abundantly clear about the type.
    Eigen::Ref<Eigen::Matrix<float, Eigen::Dynamic, 3, Eigen::RowMajor>> matrix {standard_cyborg::toMatrixX3f(data)};
    
    // Sanity-check it starts out correct
    EXPECT_EQ(data[2].y, 8.0f);
    
    // Mutate the matrix
    matrix(2, 1) = 100.0f;
    
    // Assert the matrix has (trivially, obviously) changed
    EXPECT_EQ(matrix(2, 1), 100.0f);
    
    // Assert the underlying data has changed
    EXPECT_EQ(data[2].y, 100.0f);
}

TEST(DataUtilsTests, testConstVec3ArrayToEigenX3f) {
    const std::vector<Vec3> data {
        { 1.0f, 2.0f, 3.0f },
        { 4.0f, 5.0f, 6.0f},
        { 7.0f, 8.0f, 9.0f },
        { 10.0f, 11.0f, 12.0f },
    };
    
    const auto matrix {standard_cyborg::toMatrixX3f(data)};
    
    // woot, "Expression is not assignable"
    // matrix(1, 2) = 100.0f;
    
    EXPECT_EQ(matrix(0, 0), 1.0f);
    EXPECT_EQ(matrix(0, 1), 2.0f);
    EXPECT_EQ(matrix(0, 2), 3.0f);
    EXPECT_EQ(matrix(1, 0), 4.0f);
    EXPECT_EQ(matrix(1, 1), 5.0f);
    EXPECT_EQ(matrix(1, 2), 6.0f);
    EXPECT_EQ(matrix(2, 0), 7.0f);
    EXPECT_EQ(matrix(2, 1), 8.0f);
    EXPECT_EQ(matrix(2, 2), 9.0f);
    EXPECT_EQ(matrix(3, 0), 10.0f);
    EXPECT_EQ(matrix(3, 1), 11.0f);
    EXPECT_EQ(matrix(3, 2), 12.0f);
}

// MARK: Face3 Tests

TEST(DataUtilsTests, testFace3ArrayToEigen3Xi) {
    std::vector<Face3> data {
        { 1, 2, 3 },
        { 4, 5, 6},
        { 7, 8, 9 },
        { 10, 11, 12 },
    };
    
    Eigen::Ref<Eigen::Matrix3Xi> matrix (standard_cyborg::toMatrix3Xi(data));
    
    Eigen::Matrix3Xi expected (3, 4);
    expected <<
    1, 4, 7, 10,
    2, 5, 8, 11,
    3, 6, 9, 12;
    
    EXPECT_EQ(matrix, expected);
    
    EXPECT_EQ(matrix(0, 0), 1);
    EXPECT_EQ(matrix(1, 0), 2);
    EXPECT_EQ(matrix(2, 0), 3);
    EXPECT_EQ(matrix(0, 1), 4);
    EXPECT_EQ(matrix(1, 1), 5);
    EXPECT_EQ(matrix(2, 1), 6);
    EXPECT_EQ(matrix(0, 2), 7);
    EXPECT_EQ(matrix(1, 2), 8);
    EXPECT_EQ(matrix(2, 2), 9);
    EXPECT_EQ(matrix(0, 3), 10);
    EXPECT_EQ(matrix(1, 3), 11);
    EXPECT_EQ(matrix(2, 3), 12);
}

TEST(DataUtilsTests, testFace3ArrayToEigen3XiDataMutation) {
    std::vector<Face3> data {
        { 1, 2, 3 },
        { 4, 5, 6},
        { 7, 8, 9 },
        { 10, 11, 12 },
    };
    
    Eigen::Ref<Eigen::Matrix3Xi> matrix {standard_cyborg::toMatrix3Xi(data)};
    
    // Sanity-check it starts out correct
    EXPECT_EQ(data[2][1], 8);
    
    // Mutate an entry (remember, 3Xi has three *rows* and X *columns*!)
    matrix(1, 2) = 100;
    
    // Assert the matrix has (trivially) changed
    EXPECT_EQ(matrix(1, 2), 100);
    
    // Assert the underlying data has changed
    EXPECT_EQ(data[2][1], 100);
}

TEST(DataUtilsTests, testConstFace3ArrayToEigen3Xi) {
    const std::vector<Face3> data {
        { 1, 2, 3 },
        { 4, 5, 6},
        { 7, 8, 9 },
        { 10, 11, 12 },
    };
    
    const Eigen::Ref<const Eigen::Matrix3Xi> matrix {standard_cyborg::toMatrix3Xi(data)};
    
    // woot, "Expression is not assignable"
    // matrix(1, 2) = 100;
    
    EXPECT_EQ(matrix(0, 0), 1);
    EXPECT_EQ(matrix(1, 0), 2);
    EXPECT_EQ(matrix(2, 0), 3);
    EXPECT_EQ(matrix(0, 1), 4);
    EXPECT_EQ(matrix(1, 1), 5);
    EXPECT_EQ(matrix(2, 1), 6);
    EXPECT_EQ(matrix(0, 2), 7);
    EXPECT_EQ(matrix(1, 2), 8);
    EXPECT_EQ(matrix(2, 2), 9);
    EXPECT_EQ(matrix(0, 3), 10);
    EXPECT_EQ(matrix(1, 3), 11);
    EXPECT_EQ(matrix(2, 3), 12);
}

TEST(DataUtilsTests, testFace3ArrayToEigenX3i) {
    std::vector<Face3> data {
        { 1, 2, 3 },
        { 4, 5, 6},
        { 7, 8, 9 },
        { 10, 11, 12 },
    };
    
    auto matrix = standard_cyborg::toMatrixX3i(data);
    
    EXPECT_EQ(matrix(0, 0), 1);
    EXPECT_EQ(matrix(0, 1), 2);
    EXPECT_EQ(matrix(0, 2), 3);
    EXPECT_EQ(matrix(1, 0), 4);
    EXPECT_EQ(matrix(1, 1), 5);
    EXPECT_EQ(matrix(1, 2), 6);
    EXPECT_EQ(matrix(2, 0), 7);
    EXPECT_EQ(matrix(2, 1), 8);
    EXPECT_EQ(matrix(2, 2), 9);
    EXPECT_EQ(matrix(3, 0), 10);
    EXPECT_EQ(matrix(3, 1), 11);
    EXPECT_EQ(matrix(3, 2), 12);
}

TEST(DataUtilsTests, testFace3ArrayToEigenX3iDataMutation) {
    std::vector<Face3> data {
        { 1, 2, 3 },
        { 4, 5, 6},
        { 7, 8, 9 },
        { 10, 11, 12 },
    };
    
    // I would, of course, *highly* recomment `auto` here. I have elected instead to be
    // abundantly clear about the type.
    Eigen::Ref<Eigen::Matrix<int, Eigen::Dynamic, 3, Eigen::RowMajor>> matrix {standard_cyborg::toMatrixX3i(data)};
    
    // Sanity-check it starts out correct
    EXPECT_EQ(data[2][1], 8);
    
    // Mutate the matrix
    matrix(2, 1) = 100;
    
    // Assert the matrix has (trivially, obviously) changed
    EXPECT_EQ(matrix(2, 1), 100);
    
    // Assert the underlying data has changed
    EXPECT_EQ(data[2][1], 100);
}

TEST(DataUtilsTests, testConstFace3ArrayToEigenX3i) {
    const std::vector<Face3> data {
        { 1, 2, 3 },
        { 4, 5, 6},
        { 7, 8, 9 },
        { 10, 11, 12 },
    };
    
    const auto matrix {standard_cyborg::toMatrixX3i(data)};
    
    // woot, "Expression is not assignable"
    // matrix(1, 2) = 100;
    
    EXPECT_EQ(matrix(0, 0), 1);
    EXPECT_EQ(matrix(0, 1), 2);
    EXPECT_EQ(matrix(0, 2), 3);
    EXPECT_EQ(matrix(1, 0), 4);
    EXPECT_EQ(matrix(1, 1), 5);
    EXPECT_EQ(matrix(1, 2), 6);
    EXPECT_EQ(matrix(2, 0), 7);
    EXPECT_EQ(matrix(2, 1), 8);
    EXPECT_EQ(matrix(2, 2), 9);
    EXPECT_EQ(matrix(3, 0), 10);
    EXPECT_EQ(matrix(3, 1), 11);
    EXPECT_EQ(matrix(3, 2), 12);
}
