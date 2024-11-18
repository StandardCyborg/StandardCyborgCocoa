//
//  SceneKit+Lines.h
//  StandardCyborgSDK
//
//  Created by Eric Arnebäck on 5/20/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <SceneKit/SceneKit.h>
#import <standard_cyborg/sc3d/Line.hpp>

using namespace standard_cyborg;

NS_ASSUME_NONNULL_BEGIN

@interface SCNNode (StandardCyborgDataLines)

+ (instancetype)nodeFromLines:(const std::vector<sc3d::Line>&)lines withColors:(const std::vector<math::Vec3>&)colors;

@end

NS_ASSUME_NONNULL_END

