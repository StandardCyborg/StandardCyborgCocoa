//
//  PointCloudIO.mm
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 3/13/19.
//

#import <Foundation/Foundation.h>
#import "PointCloudIO.hpp"

const char *SCFrameworkVersion() {
    static const char *__version;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        NSBundle *frameworkBundle = [NSBundle bundleWithIdentifier:@"com.standardcyborg.StandardCyborgFusion"];
        NSString *frameworkVersionString = [frameworkBundle objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
        __version = [frameworkVersionString UTF8String];
    });
    
    return __version;
}
