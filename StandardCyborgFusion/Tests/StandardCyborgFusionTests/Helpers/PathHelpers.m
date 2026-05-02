//
//  PathHelpers.m
//  StandardCyborgFusionTests
//
//  Created by Ricky Reusser on 5/1/19.
//

#import <Foundation/Foundation.h>

#import "PathHelpers.h"
#import "SCFusionBundle.h"

@implementation PathHelpers

+ (NSBundle *)scFusionBundle
{
    return [SCFusionBundle fusionBundle];
}

+ (NSString *)testCasesPath
{
    return [[SWIFTPM_MODULE_BUNDLE resourcePath] stringByAppendingPathComponent:@"Data"];
}

@end
