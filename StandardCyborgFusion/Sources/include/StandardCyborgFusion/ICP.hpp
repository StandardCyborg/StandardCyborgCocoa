//
//  ICP.hpp
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 7/10/18.
//

#ifdef __cplusplus

#pragma once

#import <standard_cyborg/math/Mat4x4.hpp>
#import <standard_cyborg/math/Vec3.hpp>
#import <standard_cyborg/sc3d/Geometry.hpp>

#import <TargetConditionals.h>
#if DEBUG
#import <vector>
#endif

using namespace standard_cyborg;

struct ICPConfiguration {
    float tolerance = 1e-4; // if the relative correspondence error is below this tolerance value, then the ICP is done.
    int maxIterations = 18; // maximum number of iterations to run ICP.
    // Standard-deviations multiplier for outlier rejection. A correspondence whose distance exceeds
    // `meanDistance + outlierDeviationsThreshold * stdDev(distance)` is dropped from this iteration.
    // Typical values: 2.5–3.5 for moderately noisy TrueDepth data.
    float outlierDeviationsThreshold = 3.0;
    int threadCount = 1; // number of threads allowed to use, for the correspondence search of ICP.
};

struct ICPResult {
    // Running the ICP algorithm, yielded this transform as a result.
    // This is a rigid transform that brings 'souceCloud' into alignment with 'targetCloud'
    math::Mat4x4 sourceTransform;
    
    // A value that measures the error in alignment between source and target.
    // Weighted by the outlier-rejection mask, so it reflects the cost that ICP is
    // actually minimizing (and on which the convergence test depends).
    float rmsCorrespondenceError = 1e10;
    
    // The number of iterations, the ICP algorithm needed to align. This
    // can be used to estimate how much ICP struggled with performing the alignment.
    int iterationCount = 0;
    
    // Whether ICP succeed at the alignment.
    bool succeeded = false;
    
#if DEBUG && TARGET_OS_MAC
    // For debugging visualization
    std::shared_ptr<std::vector<math::Vec3>> sourceVertices, targetVertices, targetNormals;
#endif
};

typedef std::function<void(ICPResult)> ICPIterationCallback;

class ICP {
public:
    static ICPResult run(ICPConfiguration config,
                         sc3d::Geometry& sourceCloud,
                         sc3d::Geometry& targetCloud,
                         ICPIterationCallback callback = nullptr);
};

#endif
