//
//  ReconstructionHelpers.cpp
//  StandardCyborgFusionTests
//
//  Created by Ricky Reusser on 5/1/19.
//

#import "ReconstructionHelpers.hpp"

#import "MetalDepthProcessor.hpp"
#import "MetalSurfelIndexMap.hpp"
#import "PointCloudIO.hpp"
#import "SCOfflineReconstructionManager.h"
#import "PathHelpers.h"

std::unique_ptr<PBFModel> assimilatePointCloud(NSString *depthFramesDir,
                                               ICPConfiguration icpConfig,
                                               PBFConfiguration pbfConfig,
                                               SurfelFusionConfiguration surfelFusionConfig)
{
    return assimilatePointCloud(depthFramesDir, icpConfig, pbfConfig, surfelFusionConfig, [](int frameNumber) {
        return std::vector<ScreenSpaceLandmark>();
    });
}

std::unique_ptr<PBFModel> assimilatePointCloud(NSString *depthFramesDir,
                                               ICPConfiguration icpConfig,
                                               PBFConfiguration pbfConfig,
                                               SurfelFusionConfiguration surfelFusionConfig,
                                               std::function<std::vector<ScreenSpaceLandmark>(int)> getFrameLandmarks)
{
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    id<MTLCommandQueue> commandQueue = [device newCommandQueue];
    NSBundle *scFusionBundle = [PathHelpers scFusionBundle];
    id<MTLLibrary> library = [device newDefaultLibraryWithBundle:scFusionBundle error:NULL];

    MetalDepthProcessor depthProcessor(device, library, commandQueue);
    std::shared_ptr<MetalSurfelIndexMap> surfelIndexMap(new MetalSurfelIndexMap(device, library, commandQueue));
    std::unique_ptr<PBFModel> pbf(new PBFModel(surfelIndexMap));
    pbf->reset();
    
    // assimilate all frames
    for (int iFrame = 0; ; ++iFrame) {
        NSString *filePath = [depthFramesDir stringByAppendingFormat:@"/frame-%03d.ply", (int)iFrame];
        if ([[NSFileManager defaultManager] fileExistsAtPath:filePath] == NO) { break; }
        
        std::unique_ptr<RawFrame> rawFrame = PointCloudIO::ReadRawFrameFromBPLYFile([filePath UTF8String]);
        
        ProcessedFrame processedFrame(*rawFrame);
        
        depthProcessor.computeFrameValues(processedFrame, *rawFrame, false);
        
        std::vector<ScreenSpaceLandmark> landmarks(getFrameLandmarks(iFrame));
        
        pbf->assimilate(processedFrame, pbfConfig, icpConfig, surfelFusionConfig, rawFrame->timestamp, &landmarks);
    }
    
    pbf->finishAssimilating(surfelFusionConfig);
    
    return pbf;
}
