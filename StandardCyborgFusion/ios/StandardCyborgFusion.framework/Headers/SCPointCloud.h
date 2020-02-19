//
//  SCPointCloud.h
//  StandardCyborgFusion
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <simd/simd.h>

@interface SCPointCloud : NSObject

/** The interval between each point's interleaved position, normal, etc data */
+ (size_t)pointStride;

/** The offset within each interleaved stride interval of the start of each XYZ position */
+ (size_t)positionOffset;
+ (size_t)normalOffset;
+ (size_t)colorOffset;
+ (size_t)weightOffset;
+ (size_t)pointSizeOffset;

/** The number of components in an XYZ position (hint: it's 3) */
+ (size_t)positionComponentCount;
+ (size_t)normalComponentCount;
+ (size_t)colorComponentCount;
+ (size_t)weightComponentCount;
+ (size_t)pointSizeComponentCount;

/** The size, in bytes, of each position component. An XYZ position is positionComponentSize * positionComponentCount bytes long */
+ (size_t)positionComponentSize;
+ (size_t)normalComponentSize;
+ (size_t)colorComponentSize;
+ (size_t)weightComponentSize;
+ (size_t)pointSizeComponentSize;

/** The number of points within this cloud */
@property (nonatomic, readonly) NSInteger pointCount;

/** Buffer for all points in the cloud. Position, normal, and other data for each point are interleaved at the offsets provided. */
@property (nonatomic, readonly) NSData *pointsData;

@property (nonatomic, readonly) simd_float3 gravity;

/** Computes the geometric mean of this point cloud */
- (simd_float3)centerOfMass;

- (instancetype)init NS_UNAVAILABLE;

@end
