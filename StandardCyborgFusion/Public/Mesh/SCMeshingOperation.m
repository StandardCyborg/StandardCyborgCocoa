//
//  SCMeshingOperation.m
//  StandardCyborgFusion
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import "SCMeshingOperation.h"

#import <MeshingOperation.h>

@implementation SCMeshingParameters

- (instancetype)init
{
    self = [super init];
    if (self) {
        _resolution = 5;
        _smoothness = 2;
        _surfaceTrimmingAmount = 5;
        _closed = YES;
    }
    return self;
}

@end

@implementation SCMeshingOperation {
    MeshingOperation *_operation;
    NSString *_APIKey;
}

- (instancetype)initWithInputPLYPath:(NSString *)inputPath
                       outputPLYPath:(NSString *)outputPath
{
    self = [super init];
    if (self) {
        _operation = [[MeshingOperation alloc] initWithInputFilePath:inputPath outputFilePath:outputPath];
        _parameters = [[SCMeshingParameters alloc] init];
    }
    return self;
}

- (void)main
{
    _operation.progressHandler = _progressHandler ?: ^(float progress) {};
    _operation.resolution = _parameters.resolution;
    _operation.smoothness = _parameters.smoothness;
    _operation.surfaceTrimmingAmount = _parameters.surfaceTrimmingAmount;
    _operation.closed = _parameters.closed;
    [_operation start];
}

- (void)cancel
{
    [_operation cancel];
}

- (BOOL)isCancelled
{
    return [_operation isCancelled];
}

@end
