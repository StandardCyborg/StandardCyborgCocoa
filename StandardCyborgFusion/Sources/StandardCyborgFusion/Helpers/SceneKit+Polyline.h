//
//  SceneKit+Polyline.h
//  StandardCyborgSDK
//
//  Created by Ricky Reusser on 5/20/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <SceneKit/SceneKit.h>
#import <standard_cyborg/sc3d/Polyline.hpp>

using namespace standard_cyborg;

NS_ASSUME_NONNULL_BEGIN

@interface SCNNode (StandardCyborgDataPolyline)

+ (instancetype)nodeFromPolyline:(const sc3d::Polyline&)polyline;

@end

NS_ASSUME_NONNULL_END
