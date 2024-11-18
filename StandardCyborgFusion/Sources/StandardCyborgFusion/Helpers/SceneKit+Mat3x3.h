//
//  SceneKit+Mat3x3.h
//  StandardCyborgSDK
//
//  Created by Eric Arneback on 5/21/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <SceneKit/SceneKit.h>
#import <standard_cyborg/math/Mat3x3.hpp>

NS_ASSUME_NONNULL_BEGIN

@interface SCNNode (StandardCyborgDataMat3x3)

+ (instancetype)nodeFromMat3x3:(const standard_cyborg::math::Mat3x3&)mat withScale:(float)scale;

@end

NS_ASSUME_NONNULL_END
