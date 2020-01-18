//
//  SCMesh+Geometry.h
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 10/17/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#ifdef __cplusplus

#import <Foundation/Foundation.h>
#import <StandardCyborgData/Geometry.hpp>
#import <StandardCyborgFusion/SCMesh.h>

NS_ASSUME_NONNULL_BEGIN

@interface SCMesh (StandardCyborgGeometry)

+ (SCMesh * _Nullable)meshFromGeometry:(const StandardCyborg::Geometry &)geometry
                           textureData:(const std::vector<float> &)textureData
                     textureResolution:(NSInteger)textureResolution;

+ (SCMesh * _Nullable)meshFromGeometry:(const StandardCyborg::Geometry &)geometry
                       textureJPEGPath:(NSString *)JPEGPath;

- (void)toGeometry:(StandardCyborg::Geometry &)geo;

@end

NS_ASSUME_NONNULL_END

#endif // __cplusplus
