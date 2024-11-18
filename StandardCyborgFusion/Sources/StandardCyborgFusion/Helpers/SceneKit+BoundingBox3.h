//
//  SceneKit+BoundingBox3.h
//  StandardCyborgSDK
//
//  Created by Eric Arneback on 5/21/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <SceneKit/SceneKit.h>
#import <standard_cyborg/sc3d/BoundingBox3.hpp>

NS_ASSUME_NONNULL_BEGIN

@interface SCNNode (StandardCyborgDataBoundingBox3)

+ (instancetype)nodeFromBoundingBox3:(const standard_cyborg::sc3d::BoundingBox3&)boundingBox;

@end

NS_ASSUME_NONNULL_END
