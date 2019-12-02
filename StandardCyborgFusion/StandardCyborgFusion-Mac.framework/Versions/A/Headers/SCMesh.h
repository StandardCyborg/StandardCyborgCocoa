//
//  SCMesh.h
//  StandardCyborgFusion
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <simd/simd.h>

@class SCNGeometryElement;
@class SCNGeometrySource;
@class SCNNode;


@interface SCMesh : NSObject

@property (nonatomic, readonly) NSInteger vertexCount;
@property (nonatomic, readonly) NSInteger faceCount;

@property (nonatomic, readonly) NSInteger textureWidth;
@property (nonatomic, readonly) NSInteger textureHeight;

@property (nonatomic, readonly) NSData *positionData;
@property (nonatomic, readonly) NSData *normalData;
@property (nonatomic, readonly) NSData *texCoordData;
@property (nonatomic, readonly) NSData *facesData;
@property (nonatomic, readonly) NSData *textureData;

- (instancetype)init NS_UNAVAILABLE;

@end
