//
//  SCMeshingOperation.h
//  StandardCyborgFusion
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

extern NSString * const SCMeshingAPIErrorDomain;

/**
 Error codes within SCMeshingAPIErrorDomain
 These are only reported for Standard Cyborg API usage
 */
typedef NS_ENUM(NSUInteger, SCMeshingAPIError) {
    SCMeshingAPIErrorInvalidAPIKey = 1,
    SCMeshingAPIErrorExceededMeshingCountLimit = 2
};

@interface SCMeshingParameters : NSObject

/**
 The resolution of the reconstructed mesh vertices.
 Higher values will result in more vertices per meshes,
 and also take longer to reconstruct.
 Range is 1-10, default is 5.
 */
@property (nonatomic) int resolution;

/**
 The smoothness of the reconstructed mesh vertex positions.
 Range is 1-10, default is 2.
 */
@property (nonatomic) int smoothness;

/**
 The amount of surface trimming for low-density mesh regions.
 Range is 0-10, default is 5. Higher numbers trim more away.
 0 = don't trim.
 */
@property (nonatomic) int surfaceTrimmingAmount;

/**
 If YES, attempts to build a closed mesh. Defaults to YES.
 */
@property (nonatomic) BOOL closed;

@end


@interface SCMeshingOperation : NSOperation

- (instancetype)init NS_UNAVAILABLE;

- (instancetype)initWithInputPLYPath:(NSString *)inputPath
                       outputPLYPath:(NSString *)outputPath;

@property (nonatomic) SCMeshingParameters *parameters;

/**
 Set this to be informed about the progress of the meshing operation.
 @param progress From 0.0-1.0
 */
@property (nonatomic, copy) void (^progressHandler)(float progress);

/**
 If non-nil, the error that occurred when performing this operation.
 */
@property (nonatomic, nullable) NSError *error;

@end

NS_ASSUME_NONNULL_END
