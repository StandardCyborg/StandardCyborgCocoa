//
//  PathHelpers.m
//  StandardCyborgFusionTests
//
//  Created by Ricky Reusser on 5/1/19.
//

#import <Foundation/Foundation.h>

#import "PathHelpers.h"
#import "SCOfflineReconstructionManager.h"

@implementation PathHelpers

+ (NSBundle *)scFusionBundle
{
    NSBundle *fusionFrameworkBundle = [NSBundle bundleForClass:[SCOfflineReconstructionManager class]];
    NSString *fusionBundlePath = [fusionFrameworkBundle pathForResource:@"StandardCyborgFusion_StandardCyborgFusion" ofType:@"bundle"];
    NSBundle *scFusionBundle = [NSBundle bundleWithPath:fusionBundlePath];
    return scFusionBundle;
}

+ (NSBundle *)scFusionTestBundle
{
    NSBundle *fusionFrameworkBundle = [NSBundle bundleForClass:[SCOfflineReconstructionManager class]];
    NSString *fusionBundlePath = [fusionFrameworkBundle pathForResource:@"StandardCyborgFusion_StandardCyborgFusionTests" ofType:@"bundle"];
    NSBundle *scFusionBundle = [NSBundle bundleWithPath:fusionBundlePath];
    return scFusionBundle;
}

+ (NSString *)testCasesPath
{
    NSBundle *bundle = [self scFusionTestBundle];
    return [[bundle resourcePath] stringByAppendingPathComponent:@"Data"];
}

@end
