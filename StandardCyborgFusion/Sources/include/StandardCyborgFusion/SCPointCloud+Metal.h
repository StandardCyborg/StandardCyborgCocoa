//
//  SCPointCloud+Metal.h
//  StandardCyborgFusion
//
//

#import <Foundation/Foundation.h>
#import <StandardCyborgFusion/SCPointCloud.h>
#import <Metal/Metal.h>

NS_ASSUME_NONNULL_BEGIN

@interface SCPointCloud (Metal)

- (id<MTLBuffer>)buildPointsMTLBufferWithDevice:(id<MTLDevice>)device;

+ (MTLVertexFormat)positionMTLVertexFormat;
+ (MTLVertexFormat)normalMTLVertexFormat;
+ (MTLVertexFormat)colorMTLVertexFormat;
+ (MTLVertexFormat)weightMTLVertexFormat;

@end

NS_ASSUME_NONNULL_END
