//
//  DrawRawDepths.hpp
//  VisualTesterMac
//

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <StandardCyborgFusion/EigenHelpers.hpp>
#import <StandardCyborgFusion/RawFrame.hpp>
#import <StandardCyborgFusion/StandardCyborgFusion.h>

@protocol CAMetalDrawable;

NS_ASSUME_NONNULL_BEGIN

@interface DrawRawDepths : NSObject

- (instancetype)initWithDevice:(id<MTLDevice>)device
                  commandQueue:(id<MTLCommandQueue>)commandQueue
                       library:(id<MTLLibrary>)library;

- (void)draw:(const std::shared_ptr<RawFrame>)rawFrame into:(id<CAMetalDrawable>)drawable;

@end

NS_ASSUME_NONNULL_END
