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

#include <standard_cyborg/math/TransformRegistry.hpp>
#include <standard_cyborg/math/Transform.hpp>
#include <standard_cyborg/math/Mat3x4.hpp>
#include <standard_cyborg/math/Mat3x3.hpp>
#include <standard_cyborg/util/DebugHelpers.hpp>
#include <standard_cyborg/util/Result.hpp>

using standard_cyborg::math::TransformRegistry;
using standard_cyborg::math::Transform;
using standard_cyborg::math::Mat3x4;
namespace math = standard_cyborg::math;
using ::standard_cyborg::Result;
using math::Vec3;

TEST_CASE("TransformRegistryTests.testRegistrationDeregistration") {
    TransformRegistry tr;
    tr.registerTransform(Transform{.srcFrame="t1", .destFrame="t0"});
    tr.registerTransform(Transform{.srcFrame="t1", .destFrame="t2"});
    
    // This is not the best proxy for ensuring the transform is registered without testing
    // other unrelated behavior, but it'll do.
    CHECK(tr.areConnected("t0", "t1").IsOk());
    CHECK(tr.areConnected("t1", "t2").IsOk());
    CHECK_FALSE(tr.areConnected("t0", "invalid").IsOk());
    CHECK_FALSE(tr.areConnected("invalid", "t0").IsOk());
    
    // Cannot deregister a frame with itself
    CHECK_FALSE(tr.deregisterTransform(Transform{.srcFrame="t0", .destFrame="t0"}).IsOk());
    
    CHECK(*(tr.areConnected("t0", "t1").value));
    CHECK(*(tr.areConnected("t1", "t2").value));
    
    // Successfully deregister
    CHECK(tr.deregisterTransform(Transform{.srcFrame="t0", .destFrame="t1"}).IsOk());
    
    CHECK_FALSE(*(tr.areConnected("t0", "t1").value));
    CHECK(*(tr.areConnected("t1", "t2").value));
    
    CHECK(tr.deregisterTransform(Transform{.srcFrame="t1", .destFrame="t2"}).IsOk());
    
    CHECK_FALSE(*(tr.areConnected("t0", "t1").value));
    CHECK_FALSE(*(tr.areConnected("t1", "t2").value));
}

TEST_CASE("TransformRegistryTests.testCircularReference") {
    //   t0
    //  /  \
    // t1 - t2
    
    TransformRegistry tr1;
    CHECK_FALSE(tr1.registerTransforms({
        Transform{.srcFrame="t0", .destFrame="t1"},
        Transform{.srcFrame="t1", .destFrame="t2"},
        Transform{.srcFrame="t2", .destFrame="t0"},
    }).IsOk());
    
    //   t0 -- t1
    //  /     /
    // t2 - t3
    TransformRegistry tr2;
    CHECK_FALSE(tr2.registerTransforms({
        Transform{.srcFrame="t0", .destFrame="t1"},
        Transform{.srcFrame="t3", .destFrame="t2"},
        Transform{.srcFrame="t2", .destFrame="t0"},
        Transform{.srcFrame="t3", .destFrame="t1"}
    }).IsOk());
}

TEST_CASE("TransformRegistryTests.testShortestPath") {
    //
    //   t0        t6
    //   |         |
    //   t1        t7
    //  /  \
    // t2  t3
    //    /  \
    //   t4  t5
    //
    TransformRegistry tr ({
        Transform{.srcFrame="t1", .destFrame="t0"},
        Transform{.srcFrame="t2", .destFrame="t1"},
        Transform{.srcFrame="t3", .destFrame="t1"},
        Transform{.srcFrame="t3", .destFrame="t4"},
        Transform{.srcFrame="t5", .destFrame="t3"},
        Transform{.srcFrame="t7", .destFrame="t6"}
    });
    
    CHECK_EQ(*(tr.path("t0", "t1").value), std::vector<std::string>({"t0", "t1"}));
    CHECK_EQ(*(tr.path("t0", "t2").value), std::vector<std::string>({"t0", "t1", "t2"}));
    CHECK_EQ(*(tr.path("t0", "t3").value), std::vector<std::string>({"t0", "t1", "t3"}));
    CHECK_EQ(*(tr.path("t6", "t7").value), std::vector<std::string>({"t6", "t7"}));
    
    // Can query either direction
    CHECK_EQ(*(tr.path("t2", "t5").value), std::vector<std::string>({"t2", "t1", "t3", "t5"}));
    CHECK_EQ(*(tr.path("t5", "t2").value), std::vector<std::string>({"t5", "t3", "t1", "t2"}));
    
    // Identity paths return a single-element path
    CHECK_EQ(*(tr.path("t0", "t0").value), std::vector<std::string>({"t0"}));
    
    // Unreachable paths return the empty path
    CHECK_EQ(*(tr.path("t0", "t6").value), std::vector<std::string>({}));
    CHECK_EQ(*(tr.path("t6", "t0").value), std::vector<std::string>({}));
    
    // Paths between frames not present at all return an error
    CHECK_FALSE(tr.path("t0", "invalid").IsOk());
    CHECK_FALSE(tr.path("invalid", "t0").IsOk());
}

TEST_CASE("TransformRegistryTests.testShortestPathWithOppositeOrientations") {
    //
    //   t0        t6
    //   |         |
    //   t1        t7
    //  /  \
    // t2  t3
    //    /  \
    //   t4  t5
    //
    TransformRegistry tr ({
        Transform{.srcFrame="t0", .destFrame="t1"},
        Transform{.srcFrame="t1", .destFrame="t2"},
        Transform{.srcFrame="t1", .destFrame="t3"},
        Transform{.srcFrame="t4", .destFrame="t3"},
        Transform{.srcFrame="t3", .destFrame="t5"},
        Transform{.srcFrame="t6", .destFrame="t7"}
    });
    
    CHECK_EQ(*(tr.path("t0", "t1").value), std::vector<std::string>({"t0", "t1"}));
    CHECK_EQ(*(tr.path("t0", "t2").value), std::vector<std::string>({"t0", "t1", "t2"}));
    CHECK_EQ(*(tr.path("t0", "t3").value), std::vector<std::string>({"t0", "t1", "t3"}));
    CHECK_EQ(*(tr.path("t6", "t7").value), std::vector<std::string>({"t6", "t7"}));
    
    // Can query either direction
    CHECK_EQ(*(tr.path("t2", "t5").value), std::vector<std::string>({"t2", "t1", "t3", "t5"}));
    CHECK_EQ(*(tr.path("t5", "t2").value), std::vector<std::string>({"t5", "t3", "t1", "t2"}));
    
    // Identity paths return a single-element path
    CHECK_EQ(*(tr.path("t0", "t0").value), std::vector<std::string>({"t0"}));
    
    // Unreachable paths return the empty path
    CHECK_EQ(*(tr.path("t0", "t6").value), std::vector<std::string>({}));
    CHECK_EQ(*(tr.path("t6", "t0").value), std::vector<std::string>({}));
    
    // Paths frames not present at all return an error
    CHECK_FALSE(tr.path("t0", "invalid").IsOk());
    CHECK_FALSE(tr.path("invalid", "t0").IsOk());
}

TEST_CASE("TransformRegistryTests.areConnected") {
    //
    //   t0        t6
    //   |         |
    //   t1        t7
    //  /  \
    // t2  t3
    //     |  \
    //     t4 t5
    //
    TransformRegistry tr ({
        Transform{.srcFrame="t1", .destFrame="t0"},
        Transform{.srcFrame="t2", .destFrame="t1"},
        Transform{.srcFrame="t3", .destFrame="t1"},
        Transform{.srcFrame="t4", .destFrame="t3"},
        Transform{.srcFrame="t5", .destFrame="t3"},
        Transform{.srcFrame="t7", .destFrame="t6"}
    });
    
    // 
    CHECK(*(tr.areConnected("t0", "t1").value));
    CHECK(*(tr.areConnected("t1", "t0").value));
    CHECK(*(tr.areConnected("t0", "t2").value));
    CHECK(*(tr.areConnected("t2", "t0").value));
    CHECK(*(tr.areConnected("t6", "t7").value));
    CHECK(*(tr.areConnected("t7", "t6").value));
    
    // Unreachable paths return false
    CHECK_FALSE(*(tr.areConnected("t0", "t6").value));
    CHECK_FALSE(*(tr.areConnected("t6", "t0").value));
    CHECK_FALSE(*(tr.areConnected("t7", "t0").value));
    CHECK_FALSE(*(tr.areConnected("t0", "t7").value));
    
    // Invalid frames return an error
    CHECK_FALSE(tr.areConnected("t0", "invalid").IsOk());
    CHECK_FALSE(tr.areConnected("invalid", "t0").IsOk());
}

TEST_CASE("TransformRegistryTests.getTransform") {
    // Our graph, along with scale factors we'll use to check that it concatenates the correct
    // transforms in the correct direction. The fact that they're prime numbers ensures we can
    // uniquely test that each transform has been hit in the correct direction, but it does *not*
    // protect ordering since scaling commutes. For that we'll need rotation transforms, which 
    // we'll test separately since they're hard to reason about.
    //
    //     t0        t6
    //     |         |
    //    (2)       (13)
    //     |         |
    //     t1        t7
    //    /  \
    //  (3)  (5)
    //  /      \
    // t2      t3
    //        /  \
    //      (7)  (11)
    //      /      \
    //     t4       t5
    TransformRegistry tr ({
        Transform{.scale={2}, .srcFrame="t0", .destFrame="t1"},
        Transform{.scale={3}, .srcFrame="t1", .destFrame="t2"},
        Transform{.scale={5}, .srcFrame="t1", .destFrame="t3"},
        Transform{.scale={7}, .srcFrame="t3", .destFrame="t4"},
        Transform{.scale={11}, .srcFrame="t3", .destFrame="t5"},
        Transform{.scale={13}, .srcFrame="t6", .destFrame="t7"}
    });

    CHECK(Mat3x4::almostEqual(Mat3x4::fromTransform(*(tr.getTransform("t0", "t0").value)), Mat3x4::Identity()));
    CHECK(Mat3x4::almostEqual(Mat3x4::fromTransform(*(tr.getTransform("t4", "t4").value)), Mat3x4::Identity()));
    
    CHECK(Mat3x4::almostEqual(Mat3x4::fromTransform(*(tr.getTransform("t0", "t1").value)), Mat3x4::fromScale({2.0})));
    CHECK(Mat3x4::almostEqual(Mat3x4::fromTransform(*(tr.getTransform("t0", "t2").value)), Mat3x4::fromScale({2.0 * 3.0})));
    CHECK(Mat3x4::almostEqual(Mat3x4::fromTransform(*(tr.getTransform("t0", "t5").value)), Mat3x4::fromScale({2.0 * 5.0 * 11.0})));
    CHECK(Mat3x4::almostEqual(Mat3x4::fromTransform(*(tr.getTransform("t5", "t0").value)), Mat3x4::fromScale({1.0 / (2.0 * 5.0 * 11.0)})));
    CHECK(Mat3x4::almostEqual(Mat3x4::fromTransform(*(tr.getTransform("t2", "t4").value)), Mat3x4::fromScale({7.0 * 5.0 / 3.0})));
    CHECK(Mat3x4::almostEqual(Mat3x4::fromTransform(*(tr.getTransform("t4", "t2").value)), Mat3x4::fromScale({3.0 / (7.0 * 5.0)})));
    CHECK(Mat3x4::almostEqual(Mat3x4::fromTransform(*(tr.getTransform("t7", "t6").value)), Mat3x4::fromScale({1.0 / 13.0})));
    CHECK(Mat3x4::almostEqual(Mat3x4::fromTransform(*(tr.getTransform("t6", "t7").value)), Mat3x4::fromScale({13.0})));
}

TEST_CASE("TransformRegistryTests.testOrdering") {
    // t0 -- t1 -- t2
    
    Mat3x4 m01 (Mat3x4::fromRotationX(M_PI_2));
    Mat3x4 m12 (Mat3x4::fromRotationY(M_PI_2));
    
    Transform t1 (Transform::fromMat3x4(m01, "t0", "t1"));
    Transform t2 (Transform::fromMat3x4(m12, "t1", "t2"));
    
    TransformRegistry tr ({t1, t2});
    
    float tol = 1e-6;
    
    // These results are somewhat obvious and just echo the input (though we check for approximate equality since
    // we did some conversion from mat3x4 to transforms)
    CHECK(Mat3x4::almostEqual(Mat3x4::fromTransform(*(tr.getTransform("t0", "t1").value)), m01, tol));
    CHECK(Mat3x4::almostEqual(Mat3x4::fromTransform(*(tr.getTransform("t1", "t2").value)), m12, tol));
    
    // This result checks ordering.
    CHECK(Mat3x4::almostEqual(Mat3x4::fromTransform(*(tr.getTransform("t0", "t2").value)), m01 * m12, tol));
    
    // This result checks the relatively obvious ordering, in reverse
    CHECK(Mat3x4::almostEqual(Mat3x4::fromTransform(*(tr.getTransform("t1", "t0").value)), m01.inverse(), tol));
    CHECK(Mat3x4::almostEqual(Mat3x4::fromTransform(*(tr.getTransform("t2", "t1").value)), m12.inverse(), tol));
    
    // Check ordering in reverse
    CHECK(Mat3x4::almostEqual(Mat3x4::fromTransform(*(tr.getTransform("t2", "t0").value)), (m01 * m12).inverse(), tol));
}

TEST_CASE("TransformRegistryTests.testToArray") {
    std::vector<Transform> transformList {
        Transform{.scale={2}, .srcFrame="t0", .destFrame="t1"},
        Transform{.scale={3}, .srcFrame="t1", .destFrame="t2"},
        Transform{.scale={5}, .srcFrame="t1", .destFrame="t3"},
        Transform{.scale={7}, .srcFrame="t3", .destFrame="t4"},
        Transform{.scale={11}, .srcFrame="t3", .destFrame="t5"},
        Transform{.scale={13}, .srcFrame="t6", .destFrame="t7"}
    };

    TransformRegistry tr (transformList);
    
    std::vector<Transform> outputList = tr.toList();
    
    CHECK_EQ(outputList.size(), 6);
    
    // As an indirect means of checking since there are few guarantees about order
    // or direction, we'll just pass it to a new registry and do some spot checks
    TransformRegistry copy (outputList);
    
    CHECK(*(copy.areConnected("t0", "t1").value));
    CHECK(*(copy.areConnected("t0", "t2").value));
    CHECK(*(copy.areConnected("t0", "t3").value));
    CHECK(*(copy.areConnected("t0", "t4").value));
    CHECK(*(copy.areConnected("t0", "t5").value));
    CHECK(*(copy.areConnected("t6", "t7").value));
    CHECK_FALSE(*(copy.areConnected("t6", "t1").value));
    CHECK_FALSE(*(copy.areConnected("t7", "t1").value));
}
