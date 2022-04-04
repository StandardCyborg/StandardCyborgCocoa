//
//  ProcessedFrame.hpp
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 8/5/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#pragma once

#include <standard_cyborg/sc3d/PerspectiveCamera.hpp>
#include <StandardCyborgFusion/RawFrame.hpp>
#include <standard_cyborg/math/Vec3.hpp>

#define METAL_REQUIRED_ALIGNMENT 4096

// ProcessedFrame and RawFrame should have the same number of points; they are not filtered by depth
struct ProcessedFrame {
    RawFrame rawFrame; // The raw frame from which these values were calculated

    std::vector<math::Vec3> positions __attribute__((aligned(METAL_REQUIRED_ALIGNMENT)));
    std::vector<math::Vec3> normals __attribute__((aligned(METAL_REQUIRED_ALIGNMENT)));
    std::vector<float> surfelSizes __attribute__((aligned(METAL_REQUIRED_ALIGNMENT)));
    std::vector<float> weights __attribute__((aligned(METAL_REQUIRED_ALIGNMENT)));
    std::vector<float> inputConfidences __attribute__((aligned(METAL_REQUIRED_ALIGNMENT)));

    ProcessedFrame(const RawFrame& rawFrameIn) :
        rawFrame(rawFrameIn),
        positions(rawFrameIn.width * rawFrameIn.height, math::Vec3()),
        normals(rawFrameIn.width * rawFrameIn.height, math::Vec3()),
        surfelSizes(rawFrameIn.width * rawFrameIn.height, 0.0f),
        weights(rawFrameIn.width * rawFrameIn.height, 0.0f),
        inputConfidences(rawFrameIn.width * rawFrameIn.height, 0.0f)
    { }
    
    ProcessedFrame(sc3d::PerspectiveCamera camera, size_t width, size_t height) :
        rawFrame(camera, width, height),
        positions(width * height, math::Vec3()),
        normals(width * height, math::Vec3()),
        surfelSizes(width * height, 0.0f),
        weights(width * height, 0.0f),
        inputConfidences(width * height, 0.0f)
    { }
    
private:
    // Prohibit copying and assignment
    ProcessedFrame(const ProcessedFrame&) = delete;
    ProcessedFrame& operator=(const ProcessedFrame&) = delete;
};
