//
//  PBFConfiguration.hpp
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 12/17/18.
//

#pragma once

#import <ostream>
#import <cmath>

struct PBFConfiguration {
    float maxCameraVelocity = 6.0;
    float maxCameraAngularVelocity = 20.0;
    float icpDownsampleFraction = 0.05;
    
    int kdTreeRebuildInterval = 6;

    // Constant-velocity motion prediction: seed ICP's source-cloud pre-transform
    // with the previous accepted delta pose instead of the (stale) previous pose.
    // Disable to fall back to the original constant-pose seed, e.g. for A/B tests.
    bool enableMotionPrediction = true;

    // Per-frame damping applied to the cached pose delta.
    // 1.0 = pure constant-velocity extrapolation, 0.0 = constant-pose (prediction disabled).
    // ~0.9 is a good trade: resilient to a single jittery frame, still recovers most of
    // the leverage on smooth motion.
    float motionPredictionDamping = 0.9f;
};

std::ostream& operator<<(std::ostream& os, PBFConfiguration const& config);
