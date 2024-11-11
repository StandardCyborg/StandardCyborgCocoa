//
//  SCReconstructionManagerParameters.h
//  StandardCyborgFusion
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

@protocol SCReconstructionManagerParameters

/** Default value is 2. Recommended value: # high-performance CPU cores on the current device. */
@property (nonatomic) int maxThreadCount;

/** The minimum depth, in meters, below which incoming depth buffer values are clipped before reconstruction. Default value is 0. */
@property (nonatomic) float minDepth;

/** The maximum depth, in meters, above which incoming depth buffer values are clipped before reconstruction. Default value is FLT_MAX. */
@property (nonatomic) float maxDepth;

@end
