//
//  SCPointCloud+Geometry.h
//  StandardCyborgData
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//


#import <Foundation/Foundation.h>
#import <StandardCyborgFusion/SCPointCloud.h>

#ifdef __cplusplus
#import <StandardCyborgData/Geometry.hpp>

NS_ASSUME_NONNULL_BEGIN

@interface SCPointCloud (Geometry)

- (void)toGeometry:(StandardCyborg::Geometry&)geometry;

@end

NS_ASSUME_NONNULL_END

#endif // __cplusplus
