//
//  SceneKit+Geometry.h
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 3/28/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <SceneKit/SceneKit.h>

namespace standard_cyborg {
namespace sc3d {
class Geometry;
}
}

using namespace standard_cyborg;

NS_ASSUME_NONNULL_BEGIN

@interface SCNGeometrySource (StandardCyborgDataGeometry)

+ (instancetype)vertexSourceFromGeometry:(const sc3d::Geometry&)geometry;
+ (instancetype)normalSourceFromGeometry:(const sc3d::Geometry&)geometry;
+ (instancetype)colorSourceFromGeometry:(const sc3d::Geometry&)geometry;
+ (instancetype)texCoordSourceFromGeometry:(const sc3d::Geometry&)geometry;

@end

@interface SCNGeometryElement (StandardCyborgDataGeometry)

+ (instancetype)pointElementFromGeometry:(const sc3d::Geometry&)geometry;
+ (instancetype)faceElementFromGeometry:(const sc3d::Geometry&)geometry;

@end

@interface SCNGeometry (StandardCyborgDataGeometry)

+ (instancetype)geometryFromGeometry:(const sc3d::Geometry&)geometry;
- (void)toGeometry:(sc3d::Geometry&)geometryOut;

@end

@interface SCNNode (StandardCyborgDataGeometry)

+ (instancetype)nodeFromGeometry:(const sc3d::Geometry&)geometry withDefaultTransform:(BOOL)useDefaultTransform;

@end

NS_ASSUME_NONNULL_END
