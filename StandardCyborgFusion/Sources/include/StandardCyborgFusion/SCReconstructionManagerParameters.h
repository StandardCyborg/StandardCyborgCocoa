//
//  SCReconstructionManagerParameters.h
//  StandardCyborgFusion
//
//

@protocol SCReconstructionManagerParameters

/** Default value is 2. Recommended value: # high-performance CPU cores on the current device. */
@property (nonatomic) int maxThreadCount;

/** The minimum depth, in meters, below which incoming depth buffer values are clipped before reconstruction. Default value is 0. */
@property (nonatomic) float minDepth;

/** The maximum depth, in meters, above which incoming depth buffer values are clipped before reconstruction. Default value is FLT_MAX. */
@property (nonatomic) float maxDepth;

#pragma mark - Algorithm parameters, tune at your own risk

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
