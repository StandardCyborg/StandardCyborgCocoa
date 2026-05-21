//
//  MeshingOperation.mm
//  Meshing
//
//  Created by Aaron Thompson on 4/26/18.
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

static unsigned long long FileSizeAtPath(NSString *path) {
    NSDictionary *attrs = [[NSFileManager defaultManager] attributesOfItemAtPath:path error:NULL];
    return attrs ? [attrs[NSFileSize] unsignedLongLongValue] : 0;
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

    // Treat a non-existent or trivially small PLY as silent failure from the
    // PoissonRecon/SurfaceTrimmer C++ layer. A well-formed PLY header alone is
    // ~100 bytes; anything below this means the writer never produced geometry.
    static const unsigned long long kMinValidPLYBytes = 256;

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
            BOOL cancelled = [weakSelf isCancelled];
            return !cancelled;
        });

        if (![weakSelf isCancelled] && FileSizeAtPath(tempPoissonOutputPathString) < kMinValidPLYBytes) {
            _failureReason = @"PoissonRecon produced no output (NULL solver result). The input point cloud is likely too sparse, has invalid normals, or is degenerate (all points coplanar).";
        }
    }

    if (![weakSelf isCancelled] && _failureReason == nil) {
        if (_surfaceTrimmingAmount <= 0) {
            // Trimming disabled, just move the file to the destination
            NSError *moveError = nil;
            [[NSFileManager defaultManager] moveItemAtPath:tempPoissonOutputPathString toPath:_outputFilePath error:&moveError];
            if (moveError) {
                _failureReason = [NSString stringWithFormat:@"Failed to move Poisson output to destination: %@", moveError.localizedDescription];
            }
        } else {
            int trimResult = SurfaceTrimmerExecute(poissonOutputPath, surfaceTrimmerOutputPath, surfaceTrimmerParams, [weakSelf, progressHandler](float progress) {
                float adjustedProgress = remapAndClamp(progress, 0, 1, kPoissonProgressFraction, 1);
                progressHandler(adjustedProgress);
                BOOL cancelled = [weakSelf isCancelled];
                return !cancelled;
            });

            if (![weakSelf isCancelled]) {
                if (trimResult != 0) {
                    _failureReason = @"SurfaceTrimmer failed: PLY header unreadable, missing density values, or Poisson output corrupt.";
                } else if (FileSizeAtPath(_outputFilePath) < kMinValidPLYBytes) {
                    _failureReason = @"SurfaceTrimmer produced no geometry (empty mesh after trimming).";
                }
            }
        }
    }

    [[NSFileManager defaultManager] removeItemAtPath:tempPoissonOutputPathString error:NULL];
}

@end
