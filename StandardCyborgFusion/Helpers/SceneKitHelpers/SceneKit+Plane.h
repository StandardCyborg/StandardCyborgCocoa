//
//  SceneKit+Plane.h
//  StandardCyborgSDK
//
//  Created by Eric Arneback on 5/21/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <SceneKit/SceneKit.h>
#import <standard_cyborg/sc3d/Plane.hpp>

using namespace standard_cyborg;

NS_ASSUME_NONNULL_BEGIN

@interface SCNNode (StandardCyborgDataPlane)

+ (instancetype)nodeFromPlane:(const sc3d::Plane&)plane
                      ofWidth:(float)width
                       height:(float)height
                        color:(id _Nullable)color
                      opacity:(float)opacity;

@end

NS_ASSUME_NONNULL_END
