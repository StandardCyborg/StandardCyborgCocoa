//
//  SceneKit+Mat3x4.h
//  StandardCyborgSDK
//
//  Created by Eric Arneback on 5/21/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <SceneKit/SceneKit.h>
#include <standard_cyborg/math/Mat3x4.hpp>


NS_ASSUME_NONNULL_BEGIN

@interface SCNNode (StandardCyborgDataMat3x4)

+ (instancetype)nodeFromMat3x4:(const standard_cyborg::math::Mat3x4&)mat  withScale:(float)scale;

@end

NS_ASSUME_NONNULL_END
