//
//  SCReconstructionManagerParameters_Private.h
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 1/15/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <StandardCyborgFusion/SCReconstructionManagerParameters.h>

@protocol SCReconstructionManagerParameters_Private <SCReconstructionManagerParameters>

/** The fraction by which incoming depth data is downsampled for alignment with the existing model.
 Default value is 0.05. Recommended range: 0.04-0.12 */
@property (nonatomic, setter=setICPDownsampleFraction:) float icpDownsampleFraction;

/** The number of standard deviations outside of which a depth value being aligned is coinsidered an outlier.
 Defaults to 1. Recommended range: 0.8-3.0 */
@property (nonatomic, setter=setICPOutlierDeviationsThreshold:) float icpOutlierDeviationsThreshold;

/** The error tolerance for aligning incoming depth data. Default value is 2e-5. Recommended range: 1e-5 - 8e-5 */
@property (nonatomic, setter=setICPTolerance:) float icpTolerance;

/** The maximum number of iterations for aligning incoming depth buffers. Default value is 24. Recommended range: 16-50 */
@property (nonatomic) int maxICPIterations;

@end
