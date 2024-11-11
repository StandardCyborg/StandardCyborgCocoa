//
//  ICP.hpp
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 7/10/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#pragma once

#include <standard_cyborg/math/Mat4x4.hpp>
#include <standard_cyborg/math/Vec3.hpp>
#include <standard_cyborg/sc3d/Geometry.hpp>

#include <TargetConditionals.h>
#if DEBUG
#include <vector>
#endif

using namespace standard_cyborg;

struct ICPConfiguration {
    float tolerance = 1e-4; // if the relative correspondence error is below this tolerance value, then the ICP is done.
    int maxIterations = 18; // maximum number of iterations to run ICP.
    float outlierDeviationsThreshold = 1.0; // threshold value, used for filtering out outlier points.
    int threadCount = 1; // number of threads allowed to use, for the correspondence search of ICP. 
};

struct ICPResult {
    // Running the ICP algorithm, yielded this transform as a result.
    // This is a rigid transform that brings 'souceCloud' into alignment with 'targetCloud'
    math::Mat4x4 sourceTransform;
    
    // A value that measures the error in alignment between source and target.
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
