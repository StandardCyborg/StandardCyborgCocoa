//
//  EigenSceneKitHelpers.hpp
//  VisualTesterMac
//
//  Created by Aaron Thompson on 7/14/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#ifndef EigenHelpers_hpp
#define EigenHelpers_hpp

#import <standard_cyborg/util/IncludeEigen.hpp>

#import <SceneKit/SceneKit.h>

static inline SCNMatrix4 SCNMatrix4FromEigenMatrix4f(Eigen::Matrix4f m)
{
    SCNMatrix4 result;
    
    result.m11 = m(0, 0); result.m21 = m(0, 1); result.m31 = m(0, 2); result.m41 = m(0, 3);
    result.m12 = m(1, 0); result.m22 = m(1, 1); result.m32 = m(1, 2); result.m42 = m(1, 3);
    result.m13 = m(2, 0); result.m23 = m(2, 1); result.m33 = m(2, 2); result.m43 = m(2, 3);
    result.m14 = m(3, 0); result.m24 = m(3, 1); result.m34 = m(3, 2); result.m44 = m(3, 3);
    
    return result;
}

static inline void FillSCNVector3ArrayFromEigenMatrix3Xf(SCNVector3 *result, Eigen::Matrix3Xf& matrix) {
    for (off_t i = 0; i < matrix.cols(); ++i) {
        result[i] = { matrix(0, i), matrix(1, i), matrix(2, i) };
    }
}

#endif /* EigenHelpers_hpp */
