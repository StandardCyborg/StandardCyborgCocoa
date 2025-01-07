//
//  SCPointCloud+Geometry.h
//  StandardCyborgData
//
//


#import <Foundation/Foundation.h>
#import <StandardCyborgFusion/SCPointCloud.h>

#ifdef __cplusplus
#import <standard_cyborg/sc3d/Geometry.hpp>

NS_ASSUME_NONNULL_BEGIN

using namespace standard_cyborg;

@interface SCPointCloud (Geometry)

- (void)toGeometry:(sc3d::Geometry&)geometry;

@end

NS_ASSUME_NONNULL_END

#endif // __cplusplus
