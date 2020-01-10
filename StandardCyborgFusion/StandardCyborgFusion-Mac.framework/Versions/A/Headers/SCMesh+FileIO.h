//
//  SCMesh+FileIO.h
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 10/19/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <StandardCyborgFusion/SCMesh.h>

NS_ASSUME_NONNULL_BEGIN

@interface SCMesh (FileIO)

- (instancetype)initWithPLYPath:(NSString *)PLYPath
                       JPEGPath:(NSString *)JPEGPath;

- (BOOL)writeTextureToJPEGAtPath:(NSString *)JPEGPath;

- (BOOL)writeToPLYAtPath:(NSString *)PLYPath;

- (BOOL)writeToGLBAtPath:(NSString *)GLBPath;

- (BOOL)writeToOBJZipAtPath:(NSString *)OBJZipPath;

- (BOOL)writeToUSDCAtPath:(NSString *)USDZPath;

- (BOOL)writeToUSDZAtPath:(NSString *)USDZPath;

@end

NS_ASSUME_NONNULL_END
