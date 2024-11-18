//
//  DrawCorrespondences.h
//  VisualTesterMac
//
//  Created by Aaron Thompson on 9/17/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "MetalVisualizationEngine.hpp"

NS_ASSUME_NONNULL_BEGIN

@interface DrawCorrespondences : NSObject <MetalVisualization>

@property (nonatomic) BOOL enabled;

@end

NS_ASSUME_NONNULL_END
