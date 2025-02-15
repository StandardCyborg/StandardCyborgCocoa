//
//  ClearPassNan.hpp
//  VisualTesterMac
//
//  Created by eric on 9/13/18.
//

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

@interface ClearPassNan : NSObject

- (void)encodeCommandsWithDevice:(id<MTLDevice>)device
                   commandBuffer:(id<MTLCommandBuffer>)commandBuffer
                     intoTexture:(id<MTLTexture>)texture
                    depthTexture:(id<MTLTexture>)depthTexture;

@end
