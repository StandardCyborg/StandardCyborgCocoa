//
//  SCFusionBundle.m
//  StandardCyborgFusion
//

#import <Foundation/Foundation.h>

#import "SCFusionBundle.h"

@implementation SCFusionBundle

+ (NSBundle *)fusionBundle
{
    NSBundle *frameworkBundle = [NSBundle bundleForClass:[self class]];
    NSString *fusionBundlePath = [frameworkBundle pathForResource:@"StandardCyborg_StandardCyborgFusion" ofType:@"bundle"];
    if (fusionBundlePath == nil) {
        fusionBundlePath = [[NSBundle mainBundle] pathForResource:@"StandardCyborg_StandardCyborgFusion" ofType:@"bundle"];
    }
    return [NSBundle bundleWithPath:fusionBundlePath];
}

@end
