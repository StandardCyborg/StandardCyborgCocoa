//
//  SCMesh_Private.h
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 12/17/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <CoreImage/CoreImage.h>
#import <StandardCyborgFusion/SCMesh.h>

#ifdef __cplusplus
#import <vector>
#endif

@interface SCMesh ()
@property (nonatomic) NSString *textureJPEGPath;
@end

@interface SCMesh (Private)

- (instancetype)initWithPositionData:(NSData *)positionData
                          normalData:(NSData *)normalData
                           colorData:(NSData *)colorData
                           facesData:(NSData *)facesData;

- (instancetype)initWithPositionData:(NSData *)positionData
                          normalData:(NSData *)normalData
                        texCoordData:(NSData *)texCoordData
                           facesData:(NSData *)facesData
                         textureData:(NSData *)textureData
                        textureWidth:(NSInteger)textureWidth
                       textureHeight:(NSInteger)textureHeight;

- (instancetype)initWithPositionData:(NSData *)positionData
                          normalData:(NSData *)normalData
                        texCoordData:(NSData *)texCoordData
                           facesData:(NSData *)facesData
                     textureJPEGPath:(NSString *)textureJPEGPath;

- (CIImage *)textureAsCIImage;
- (NSData *)encodeTextureToJPEGData;

#ifdef __cplusplus
+ (BOOL)readTextureFromImageAtPath:(NSString *)imagePath
                        intoVector:(std::vector<float> &)textureData
                 textureResolution:(NSInteger *)textureResolution;
#endif

@end
