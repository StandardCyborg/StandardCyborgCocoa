//
//  ReconstructionHelpers.hpp
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 5/1/19.
//

#include "../../Sources/StandardCyborgFusion/Algorithm/PBFModel.hpp"
#include "ScreenSpaceLandmark.hpp"

#include <functional>
#include <memory>
#include <vector>

#include <vector>

 
#ifdef __OBJC__
#import <Foundation/Foundation.h>
#endif


std::unique_ptr<PBFModel> assimilatePointCloud(NSString* depthFramesDir,
                                               ICPConfiguration icpConfig,
                                               PBFConfiguration pbfConfig,
                                               SurfelFusionConfiguration surfelFusionConfig);

std::unique_ptr<PBFModel> assimilatePointCloud(NSString* depthFramesDir,
                                               ICPConfiguration icpConfig,
                                               PBFConfiguration pbfConfig,
                                               SurfelFusionConfiguration surfelFusionConfig,
                                               std::function<std::vector<ScreenSpaceLandmark>(int)>getFrameLandmarks);
