//
//  SCPointCloud+Metal.m
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 12/17/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import "SCPointCloud+Metal.h"
#import <Metal/Metal.h>
#import <StandardCyborgFusion/MathHelpers.h>

@implementation SCPointCloud (Metal)

- (id<MTLBuffer>)buildPointsMTLBufferWithDevice:(id<MTLDevice>)device
{
    if ([self.pointsData length] == 0) {
        return [device newBufferWithLength:0 options:0];
    }
    
    return [device newBufferWithBytesNoCopy:(void *)self.pointsData.bytes
                                     length:roundUpToMultiple(self.pointsData.length, 4096)
                                    options:MTLResourceOptionCPUCacheModeWriteCombined
                                deallocator:NULL];
}

+ (MTLVertexFormat)positionMTLVertexFormat { return MTLVertexFormatFloat3; }
+ (MTLVertexFormat)normalMTLVertexFormat { return MTLVertexFormatFloat3; }
+ (MTLVertexFormat)colorMTLVertexFormat { return MTLVertexFormatFloat3; }
+ (MTLVertexFormat)weightMTLVertexFormat { return MTLVertexFormatFloat; }

@end
