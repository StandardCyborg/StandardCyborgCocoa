//
//  SCNNode+StandardCyborgNode.h
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 8/6/19.
//  Copyright © 2019 StandardCyborg. All rights reserved.
//

#import <SceneKit/SceneKit.h>
#import <standard_cyborg/scene_graph/SceneGraph.hpp>
#import <memory>

NS_ASSUME_NONNULL_BEGIN

@interface SCNNode (StandardCyborgNode)

+ (SCNNode * _Nullable)nodeFromStandardCyborgNode:(std::shared_ptr<standard_cyborg::scene_graph::Node>)node
                             withDefaultTransform:(BOOL)useDefaultTransform;

@end

NS_ASSUME_NONNULL_END
