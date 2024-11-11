//
//  SCLandmark2D_Private.h
//  StandardCyborgSDK
//
//  Created by Aaron Thompson on 7/4/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <StandardCyborgFusion/SCLandmark2D.h>

@interface SCLandmark2D ()

- (instancetype)_init;

// Re-declares these as read-write
@property (nonatomic) NSString *landmarkName;
@property (nonatomic) int landmarkIndex;
@property (nonatomic) float x;
@property (nonatomic) float y;
@property (nonatomic) float confidence;
@property (nonatomic) simd_float2 simdPosition;

@end
