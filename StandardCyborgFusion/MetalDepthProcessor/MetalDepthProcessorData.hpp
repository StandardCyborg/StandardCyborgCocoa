//
//  MetalDepthProcessorData.h
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 9/24/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <Metal/Metal.h>
#import <standard_cyborg/util/IncludeEigen.hpp>
#import <standard_cyborg/math/Vec3.hpp>


struct MetalDepthProcessorData {
    size_t width;
    size_t height;
    
    id<MTLBuffer> pointsBuffer;
    id<MTLBuffer> weightsBuffer;
    id<MTLBuffer> inputConfidencesBuffer;
    id<MTLBuffer> normalsBuffer;
    id<MTLBuffer> surfelSizesBuffer;
    
    id<MTLBuffer> workBuffer;
    
    id<MTLTexture> depthTexture;
    id<MTLTexture> workTexture;
    id<MTLTexture> smoothedDepthTexture;
    
    MetalDepthProcessorData();
    ~MetalDepthProcessorData();
    
    void fill(
        id<MTLDevice> device,
        size_t width,
        size_t height,
        const std::vector<float> &depths,      
        std::vector<standard_cyborg::math::Vec3> &points,
        std::vector<standard_cyborg::math::Vec3> &normals,
        std::vector<float> &surfelSizes,
        std::vector<float> &weights,
        std::vector<float> &inputConfidences
    );

private:
    // Prohibit copying and assignment
    MetalDepthProcessorData(const MetalDepthProcessorData&) = delete;
    MetalDepthProcessorData& operator=(const MetalDepthProcessorData&) = delete;
};
