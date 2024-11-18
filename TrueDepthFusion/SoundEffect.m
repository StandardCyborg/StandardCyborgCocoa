//
//  SoundEffect.m
//  TrueDepthFusion
//
//  Created by Aaron Thompson on 8/22/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import "SoundEffect.h"
#import <AudioToolbox/AudioToolbox.h>

@implementation SoundEffect {
    SystemSoundID _soundID;
}

- (instancetype)initWithSoundNamed:(NSString *)name type:(NSString *)type
{
    self = [super init];
    if (self) {
        NSURL *url = [[NSBundle mainBundle] URLForResource:name withExtension:type];
        AudioServicesCreateSystemSoundID((__bridge CFURLRef)url, &_soundID);
    }
    return self;
}

- (void)dealloc
{
    AudioServicesDisposeSystemSoundID(_soundID);
}

- (void)play
{
    AudioServicesPlaySystemSound(_soundID);
}

@end
