//
//  SCPointCloud+FileIO.h
//  StandardCyborgFusion
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <StandardCyborgFusion/SCPointCloud.h>

NS_ASSUME_NONNULL_BEGIN

@interface SCPointCloud (FileIO)

- (nullable instancetype)initWithPLYPath:(NSString *)PLYPath;

- (BOOL)writeToPLYAtPath:(NSString *)PLYPath;

- (BOOL)writeToUSDZAtPath:(NSString *)USDZPath;

@end

NS_ASSUME_NONNULL_END
