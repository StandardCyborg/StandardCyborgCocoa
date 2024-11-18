//
//  MetalTextureProjection.hpp
//  UvMapBaker
//
//  Created by eric on 2019-09-03.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#import <Metal/Metal.h>
#import <simd/simd.h>
#import <vector>


#import <standard_cyborg/math/Mat3x3.hpp>
#import <standard_cyborg/math/Vec4.hpp>
#import <standard_cyborg/sc3d/ColorImage.hpp>
#import <standard_cyborg/sc3d/PerspectiveCamera.hpp>

#import "ClearPassNan.hpp"
#import "RenderUvs.hpp"
#import "RenderPositions.hpp"

using namespace standard_cyborg;

class MetalTextureProjection {
public:
    /** Specify metal device, used for the projection calculations.
     Also, specify texture resolution of the texture we will be projecting to. */
    MetalTextureProjection(id<MTLDevice> device, int textureResolution);

    /**
     Start our texture projection algorithm
     */
    bool startProjecting(int frameWidth, int frameHeight, const sc3d::Geometry& triangleMesh, const sc3d::PerspectiveCamera& camera);

    /**
     Finish the texture projection algorithm.

     Returned is a texture, that uses the uv coords of the mesh, that contains
     the colors that has been projected onto the mesh.
     */
    std::vector<float> finishProjecting(const sc3d::Geometry& triangleMesh);

    /**
     Given a RGB frame gotten from the iPhone, and the view and projection
     matrices found by ICP when aligning each frame, project this frame
     onto the specified mesh.
     */
    bool projectSingleTexture(const matrix_float4x4& viewMatrix, const matrix_float4x4& projectionMatrix, const sc3d::ColorImage& frame, const sc3d::Geometry& triangleMesh);

private:
    int _textureResolution;
    id<MTLDevice> _device;
    id<MTLLibrary> _library;
    id<MTLCommandQueue> _commandQueue;

    ClearPassNan* _clearPassNan;
    RenderUvs* _renderUvs;
    RenderPositions* _renderPositions;

    id<MTLTexture> _uvTexture;
    id<MTLTexture> _depthTexture;
    id<MTLTexture> _positionsTexture;
    id<MTLTexture> _frameTexture;
    id<MTLTexture> _targetTexture; // texture we are projecting to
    
    id<MTLFunction> _computeKernel;
    id<MTLComputePipelineState> _computePipelineState;

    sc3d::PerspectiveCamera _camera;
    
    inline math::Vec4 _getColor(int col, int row, const std::vector<float>& textureData);
    
    /// Fills in holes where nothing was projected to
    void _fillTextureHoles(const sc3d::Geometry &filteredGeo,
                           std::vector<float> &positionsTextureData,
                           std::vector<float> &textureData);
    
    /// Pads all charts to make sure texture seams aren't visible
    void _padTextureEdges(std::vector<float> &textureData);
};
