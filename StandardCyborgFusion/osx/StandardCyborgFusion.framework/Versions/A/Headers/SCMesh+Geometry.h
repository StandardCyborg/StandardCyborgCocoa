//
//  SCMesh+Geometry.h
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 10/17/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#ifdef __cplusplus

#import <Foundation/Foundation.h>
#import <standard_cyborg/sc3d/Geometry.hpp>
#import <StandardCyborgFusion/SCMesh.h>

NS_ASSUME_NONNULL_BEGIN

using namespace standard_cyborg;

@interface SCMesh (StandardCyborgGeometry)

+ (SCMesh * _Nullable)meshFromGeometry:(const sc3d::Geometry &)geometry
                           textureData:(const std::vector<float> &)textureData
                     textureResolution:(NSInteger)textureResolution;

+ (SCMesh * _Nullable)meshFromGeometry:(const sc3d::Geometry &)geometry
                       textureJPEGPath:(NSString *)JPEGPath;

- (void)toGeometry:(sc3d::Geometry &)geo;

@end

NS_ASSUME_NONNULL_END

#endif // __cplusplus
