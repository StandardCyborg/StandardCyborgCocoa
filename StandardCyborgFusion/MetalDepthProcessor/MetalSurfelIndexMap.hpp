//
//  MetalSurfelIndexMap.hpp
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 4/19/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <StandardCyborgFusion/SurfelIndexMap.hpp>
#import <Metal/Metal.h>

class MetalSurfelIndexMap: public SurfelIndexMap {
public:
    MetalSurfelIndexMap(id<MTLDevice> device, id<MTLCommandQueue> commandQueue, bool forColor = false);
    
    virtual bool draw(const std::vector<Surfel>& surfels,
                      const Eigen::Matrix4f& modelMatrix,
                      const RawFrame& rawFrame,
                      std::vector<uint32_t> & indexLookups);
    
    virtual bool drawForColor(const Surfel* surfels,
                              size_t surfelCount,
                              Eigen::Matrix4f viewProjectionMatrix,
                              size_t frameWidth,
                              size_t frameHeight,
                              std::vector<uint32_t> & indexLookups);
    virtual Eigen::Matrix4f getViewProjectionMatrix();
    id<MTLTexture> getDepthTexture();
    id<MTLTexture> getIndexTexture();
    
private:
    id<MTLDevice> _device;
    id<MTLLibrary> _library;
    id<MTLCommandQueue> _commandQueue;
    id<MTLRenderPipelineState> _pipelineState;
    id<MTLBuffer> _vertexBuffer;
    id<MTLBuffer> _sharedUniformsBuffer;
    simd_float4x4 _lastViewProjectionMatrix;
    id<MTLTexture> _depthTexture;
    id<MTLTexture> _indexTexture;
    id<MTLDepthStencilState> _depthStencilState;
    
    id<MTLBuffer> _createVertexBuffer();
    id<MTLBuffer> _createSurfelsBufferFromSurfels(const std::vector<Surfel>& surfels);
    void _updateSharedUniformsBuffer(const RawFrame& frame, const Eigen::Matrix4f& modelMatrix);
    void _updateSharedUniformsBufferForColor(Eigen::Matrix4f viewProjection);
};
