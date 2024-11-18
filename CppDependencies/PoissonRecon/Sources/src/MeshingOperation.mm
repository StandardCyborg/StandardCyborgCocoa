//
//  MeshingOperation.mm
//  Meshing
//
//  Created by Aaron Thompson on 4/26/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <PoissonRecon/MeshingOperation.h>
#import <PoissonRecon/Parameters.hpp>
#import <PoissonRecon/ExecuteEntryFunctions.hpp>

using namespace std;

static float fclampf(float value, float min, float max) {
    return fmaxf(fminf(value, max), min);
}

static float remapAndClamp(float value, float originalMin, float originalMax, float newMin, float newMax) {
    float shiftedOriginal = value - originalMin;
    float scaled = shiftedOriginal * (newMax - newMin) / (originalMax - originalMin);
    float shiftedNew = scaled + newMin;
    
    return fclampf(shiftedNew, newMin, newMax);
}

static const float kPoissonProgressFraction = 0.75;

@implementation MeshingOperation {
    NSString *_inputFilePath;
    NSString *_outputFilePath;
}

- (instancetype)initWithInputFilePath:(NSString *)inputPath
                       outputFilePath:(NSString *)outputPath
{
    self = [super init];
    if (self) {
        _inputFilePath = inputPath;
        _outputFilePath = outputPath;
        
        _resolution = 5;
        _smoothness = 2;
        
        _progressHandler = ^(float){};
    }
    return self;
}

- (void)main
{
    const char *inputPath = [_inputFilePath UTF8String];
    NSString *tempPoissonOutputPathString = [NSTemporaryDirectory() stringByAppendingFormat:@"/poisson-%@.ply", [[NSUUID UUID] UUIDString]];
    const char *poissonOutputPath = [tempPoissonOutputPathString UTF8String];
    const char *surfaceTrimmerOutputPath = [_outputFilePath UTF8String];
    
    PoissonReconParameters poissonParams;
    poissonParams.Depth = (int)remapAndClamp(_resolution, 1, 10, 4, 14);
    poissonParams.SamplesPerNode = (int)remapAndClamp(_smoothness, 1, 10, 1, 15);
    
    SurfaceTrimmerParameters surfaceTrimmerParams;
    surfaceTrimmerParams.Trim = (int)remapAndClamp(_surfaceTrimmingAmount, 1, 10, 1, 10);
    
    __weak MeshingOperation *weakSelf = self;
    auto progressHandler = _progressHandler;
    
    if (![weakSelf isCancelled]) {
        PoissonReconExecute(inputPath, poissonOutputPath, _closed, poissonParams, [weakSelf, progressHandler](float progress) {
            float adjustedProgress = remapAndClamp(progress, 0, 1, 0, kPoissonProgressFraction);
            
            progressHandler(adjustedProgress);
            
            BOOL shouldContinue = [weakSelf isCancelled];
            return !shouldContinue;
        });
    }
    
    if (![weakSelf isCancelled]) {
        if (_surfaceTrimmingAmount <= 0) {
            // Trimming disabled, just move the file to the destination
            [[NSFileManager defaultManager] moveItemAtPath:tempPoissonOutputPathString toPath:_outputFilePath error:NULL];
        } else {
            SurfaceTrimmerExecute(poissonOutputPath, surfaceTrimmerOutputPath, surfaceTrimmerParams, [weakSelf, progressHandler](float progress) {
                float adjustedProgress = remapAndClamp(progress, 0, 1, kPoissonProgressFraction, 1);
                progressHandler(adjustedProgress);
                
                BOOL shouldContinue = [weakSelf isCancelled];
                return !shouldContinue;
            });
        }
    }
    
    [[NSFileManager defaultManager] removeItemAtPath:tempPoissonOutputPathString error:NULL];
}

@end
