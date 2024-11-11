//
//  SCMesh.mm
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 12/13/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>
#import <SceneKit/SceneKit.h>
#import <standard_cyborg/math/Vec3.hpp>
#import <standard_cyborg/sc3d/Face3.hpp>

#import "SCMesh.h"
#import "SCMesh_Private.h"

using namespace standard_cyborg;

@implementation SCMesh

- (instancetype)initWithPositionData:(NSData *)positionData
                          normalData:(NSData *)normalData
                           colorData:(NSData *)colorData
                           facesData:(NSData *)facesData
{
    self = [super init];
    if (self) {
        _positionData = positionData;
        _normalData = normalData;
        _colorData = colorData;
        _facesData = facesData;
        
        _vertexCount = [positionData length] / sizeof(math::Vec3);
        _faceCount = [facesData length] / sizeof(sc3d::Face3);
    }
    return self;
}

- (instancetype)initWithPositionData:(NSData *)positionData
                          normalData:(NSData *)normalData
                        texCoordData:(NSData *)texCoordData
                           facesData:(NSData *)facesData
                         textureData:(NSData *)textureData
                        textureWidth:(NSInteger)textureWidth
                       textureHeight:(NSInteger)textureHeight
{
    self = [super init];
    if (self) {
        _positionData = positionData;
        _normalData = normalData;
        _texCoordData = texCoordData;
        _facesData = facesData;
        
        _vertexCount = [positionData length] / sizeof(math::Vec3);
        _faceCount = [facesData length] / sizeof(sc3d::Face3);
        
        _textureData = textureData;
        _textureWidth = textureWidth;
        _textureHeight = textureHeight;
    }
    return self;
}

- (instancetype)initWithPositionData:(NSData *)positionData
                          normalData:(NSData *)normalData
                        texCoordData:(NSData *)texCoordData
                           facesData:(NSData *)facesData
                     textureJPEGPath:(NSString *)textureJPEGPath
{
    self = [super init];
    if (self) {
        _positionData = positionData;
        _normalData = normalData;
        _texCoordData = texCoordData;
        _facesData = facesData;
        _textureJPEGPath = textureJPEGPath;
        
        _vertexCount = [positionData length] / sizeof(math::Vec3);
        _faceCount = [facesData length] / sizeof(sc3d::Face3);
    }
    return self;
}

// MARK: - Private

- (CIImage *)textureAsCIImage
{
    CGColorSpaceRef linearColorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGBLinear);
    
    CIImage *ciImage = [CIImage imageWithBitmapData:_textureData
                                        bytesPerRow:4 * _textureWidth * sizeof(float)
                                               size:CGSizeMake(_textureWidth, _textureHeight)
                                             format:kCIFormatRGBAf
                                         colorSpace:linearColorSpace];
    
    CGColorSpaceRelease(linearColorSpace);
    
    return ciImage;
}

- (NSData *)encodeTextureToJPEGData
{
    CIImage *ciImage = [self textureAsCIImage];
    CIContext *context = [CIContext context];
    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
    
    NSData *result = [context JPEGRepresentationOfImage:ciImage
                                             colorSpace:colorSpace
                                                options:@{}];
    
    CGColorSpaceRelease(colorSpace);
    
    return result;
}

+ (BOOL)readTextureFromImageAtPath:(NSString *)imagePath
                        intoVector:(std::vector<float> &)textureData
                 textureResolution:(NSInteger *)textureResolution
{
    CIImage *image = [CIImage imageWithContentsOfURL:[NSURL fileURLWithPath:imagePath]];
    if (image == nil) { return NO; }
    
    CIContext *context = [CIContext context];
    CGColorSpaceRef linearColorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGBLinear);
    NSInteger imageWidth = image.extent.size.width;
    NSInteger imageHeight = image.extent.size.height;
    
    textureData.resize(imageWidth * imageHeight * 4);
    [context render:image
           toBitmap:textureData.data()
           rowBytes:4 * imageWidth * sizeof(float)
             bounds:image.extent
             format:kCIFormatRGBAf
         colorSpace:linearColorSpace];
    
    CGColorSpaceRelease(linearColorSpace);
    
    *textureResolution = imageWidth;
    return YES;
}

@end
