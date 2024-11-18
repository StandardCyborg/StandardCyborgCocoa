//
//  CameraControl.h
//  VisualTesterMac
//
//  Created by Aaron Thompson on 9/7/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#import <simd/simd.h>

@protocol CameraControlDelegate;

@interface CameraControl : NSObject

@property (nonatomic, weak) id<CameraControlDelegate> delegate;

- (void)installInView:(NSView *)view;
- (void)setCenterX:(float)centerX centerY:(float)centerY centerZ:(float)centerZ;

- (matrix_float4x4)viewMatrix;
- (matrix_float4x4)projectionMatrix;

@end

@protocol CameraControlDelegate
- (void)cameraDidMove:(CameraControl *)control;
@end
