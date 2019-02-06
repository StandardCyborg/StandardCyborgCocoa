//
//  SCPointCloud.h
//  StandardCyborgFusion
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#ifndef SCPointCloud_h
#define SCPointCloud_h

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

/** The number of components in an XYZ position (hint: it's 3) */
+ (size_t)positionComponentCount;
+ (size_t)normalComponentCount;
+ (size_t)colorComponentCount;
+ (size_t)weightComponentCount;

/** The size, in bytes, of each position component. An XYZ position is positionComponentSize * positionComponentCount bytes long */
+ (size_t)positionComponentSize;
+ (size_t)normalComponentSize;
+ (size_t)colorComponentSize;
+ (size_t)weightComponentSize;


/** The number of points within this cloud */
@property (nonatomic, readonly) NSInteger pointCount;

/** Buffer for all points in the cloud. Position, normal, and other data for each point are interleaved at the offsets provided. */
@property (nonatomic, readonly) NSData *pointsData;

- (instancetype)init NS_UNAVAILABLE;

@end

#endif /* SCPointCloud_h */
