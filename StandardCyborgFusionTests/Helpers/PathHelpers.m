//
//  PathHelpers.m
//  StandardCyborgFusionTests
//
//  Created by Ricky Reusser on 5/1/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "PathHelpers.h"

@implementation PathHelpers

+ (NSString *)testCasesPath
{
    NSString *projectDirectory = [NSString stringWithUTF8String:PROJECT_DIR];
    return [projectDirectory stringByAppendingPathComponent: @"/StandardCyborgFusionTests"];
}

@end
