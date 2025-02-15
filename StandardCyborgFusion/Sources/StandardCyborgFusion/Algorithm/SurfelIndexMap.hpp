//
//  SurfelIndexMap.hpp
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 7/26/18.
//

#pragma once

#import "EigenHelpers.hpp"
#import "RawFrame.hpp"
#import "Surfel.hpp"
#import <vector>

class SurfelIndexMap {
public:
    virtual bool draw(const std::vector<Surfel>& surfels,
                      const Eigen::Matrix4f& modelMatrix,
                      const RawFrame& rawFrame,
                      std::vector<uint32_t>& indexLookups) = 0;
    
    virtual bool drawForColor(const Surfel* surfels,
                              size_t surfelCount,
                              Eigen::Matrix4f viewProjectionMatrix,
                              size_t frameWidth,
                              size_t frameHeight,
                              std::vector<uint32_t>& indexLookups) = 0;
    
    virtual Eigen::Matrix4f getViewProjectionMatrix() = 0;
};
