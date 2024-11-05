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

#include "standard_cyborg/sc3d/PerspectiveCamera.hpp"

using namespace standard_cyborg;

TEST(PerspectiveCameraTests, testDistortionTableConstantsComputation) {
    sc3d::PerspectiveCamera camera;
    
    camera.setLensDistortionLookupTable({
        -0.000002120365707014571, 0.000041151542973238975, 0.00017070896865334362, 0.0003857532574329525,
        0.0006848758202977479, 0.0010659483959898353, 0.0015259817009791732, 0.0020609647035598755,
        0.0026656973641365767, 0.0033336353953927755, 0.004056759178638458, 0.0048254914581775665,
        0.005628669634461403, 0.00645359605550766, 0.00728617561981082, 0.008111140690743923,
        0.008912383578717709, 0.00967337191104889, 0.010377663187682629, 0.011009485460817814,
        0.011554376222193241, 0.011999855749309063, 0.01233610324561596, 0.012556605972349644,
        0.012658767402172089, 0.012644429691135883, 0.012520307675004005, 0.012298309244215488,
        0.011995756067335606, 0.01163549069315195, 0.011245905421674252, 0.010860903188586235,
        0.010519820265471935, 0.010267329402267933, 0.010153334587812424, 0.01023282390087843,
        0.010565624572336674, 0.011215916834771633, 0.012251267209649086, 0.0137407798320055,
        0.01575177162885666, 0.018344050273299217
    });
    
    math::Vec4 curveFit = camera.getLensDistortionCurveFit();
    
    EXPECT_NEAR(curveFit.x, 0.049931, 1e-6);
    EXPECT_NEAR(curveFit.y, 0.194135, 1e-6);
    EXPECT_NEAR(curveFit.z, -0.574323, 1e-6);
    EXPECT_NEAR(curveFit.w, 0.348729, 1e-6);
}

TEST(PerspectiveCameraTests, testConstructor) {
    sc3d::PerspectiveCamera camera;
    
    camera.setIntrinsicMatrixReferenceSize({3088.0f, 2316.0f});
    camera.setNominalIntrinsicMatrix({
        2881.15576171875, 0.0, 1536.593505859375,
        0.0, 2881.15576171875, 1149.3253173828125,
        0.0, 0.0, 1.0
    });
    camera.setExtrinsicMatrix({
        0.0, 1.0, 0.0, 0.0,
        1.0, 0.0, 0.0, 0.0,
        0.0, 0.0, -1.0, 0.0,
    });
    camera.setFocalLengthScaleFactor(1.017f);
    
    EXPECT_TRUE(math::Vec4::almostEqual(camera.getLensDistortionCurveFit(), math::Vec4(0)));
    EXPECT_TRUE(math::Vec4::almostEqual(camera.getInverseLensDistortionCurveFit(), math::Vec4(0)));
    EXPECT_EQ(camera.getFocalLengthScaleFactor(), 1.017f);
    EXPECT_EQ(camera.getExtrinsicMatrix(), math::Mat3x4({
        0.0, 1.0, 0.0, 0.0,
        1.0, 0.0, 0.0, 0.0,
        0.0, 0.0, -1.0, 0.0,
    }));
    EXPECT_TRUE(math::Mat3x3::almostEqual(camera.getIntrinsicMatrix(), math::Mat3x3({
        2930.14, 0.0, 1536.59,
        0.0, 2930.14, 1149.33,
        0.0, 0.0, 1.0
    }), 1.0e-5f, 1.0e-5f));
}

TEST(PerspectiveCameraTests, UnprojectDepthSampleTests) {
    namespace sc3d = standard_cyborg::sc3d;
    using standard_cyborg::math::Mat3x4;
    using standard_cyborg::math::Mat3x3;
    using standard_cyborg::math::Vec2;
    using standard_cyborg::math::Vec3;
    
    sc3d::PerspectiveCamera camera;
    
    camera.setFocalLengthScaleFactor(1.019f);
    
    Mat3x4 extrinsicMatrix(Mat3x4(1, 0, 0, 0,
                                  0, 1, 0, 0,
                                  0, 0, 1, 0));
    
    camera.setExtrinsicMatrix(extrinsicMatrix);
    
    Mat3x4 desiredOrientation({
        0, 1, 0, 0,
        1, 0, 0, 0,
        0, 0, -1, 0
    });
    
    Mat3x4 baselineExtrinsicMatrix = Mat3x4::Identity();
    Mat3x4 orientationMatrix(desiredOrientation * baselineExtrinsicMatrix.inverse());
    
    camera.setOrientationMatrix(orientationMatrix);
    
    camera.setNominalIntrinsicMatrix( Mat3x3(
                                             2874.18774, 0,  1911.88721,
                                             
                                             0, 2874.18774, 1078.8269,
                                             
                                             0,  0,  1) );
    camera.setIntrinsicMatrixReferenceSize(Vec2(3840, 2160));
    
    camera.setLegacyImageSize({320, 180});
    
    float depth = 0.2141;
    Vec3 unprojectedPos = camera.unprojectDepthSample(320, 180, 124, 174, depth);
    
    float EPS = 0.00001f;
    
    EXPECT_NEAR(unprojectedPos.x, 0.073601f, EPS);
    EXPECT_NEAR(unprojectedPos.y, 0.030987f, EPS);
    EXPECT_NEAR(unprojectedPos.z, 0.214100f, EPS);
}
