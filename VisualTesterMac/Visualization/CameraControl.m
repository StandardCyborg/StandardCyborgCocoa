//
//  CameraControl.m
//  VisualTesterMac
//
//  Created by Aaron Thompson on 9/7/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <GLKit/GLKMatrix4.h>
#import "CameraControl.h"

static matrix_float4x4 MatrixFloat4x4FromGLKMatrix4(GLKMatrix4 m) {
    return (matrix_float4x4){
        (simd_float4){ m.m00, m.m01, m.m02, m.m03 },
        (simd_float4){ m.m10, m.m11, m.m12, m.m13 },
        (simd_float4){ m.m20, m.m21, m.m22, m.m23 },
        (simd_float4){ m.m30, m.m31, m.m32, m.m33 }
    };
}


@implementation CameraControl {
    NSView *_view;
    CGFloat _referenceRadius;
    CGFloat _phi, _theta, _radius;
    CGFloat _near, _far;
    CGFloat _fovY, _aspectRatio;
    vector_float3 _center, _eye, _up;
    matrix_float4x4 _viewMatrix, _projectionMatrix;
    bool _dirty;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        _phi = 0.0;
        _theta = -M_PI_2;
        _referenceRadius = _radius = 0.5;

        _up[0] = 0.0;
        _up[1] = 1.0;
        _up[2] = 0.0;
        
        _center[0] = 0.0;
        _center[1] = 0.0;
        _center[2] = 0.0;
        
        _near = 0.001;
        _far = 8.0;
        
        _fovY = 45.0 * M_PI / 180.0;
        _dirty = true;
    }
    return self;
}

- (void)installInView:(NSView *)view
{
    _view = view;
    [view addGestureRecognizer:[[NSPanGestureRecognizer alloc] initWithTarget:self action:@selector(_viewDragged:)]];
    [view addGestureRecognizer:[[NSMagnificationGestureRecognizer alloc] initWithTarget:self action:@selector(_viewZoomed:)]];
}

- (void)_updateView
{
    _dirty = true;
    [_delegate cameraDidMove:self];
}

- (void)setCenterX:(float)centerX centerY:(float)centerY centerZ:(float)centerZ
{
    _dirty = true;
    _center[0] = centerX;
    _center[1] = centerY;
    _center[2] = centerZ;
}

- (void)_viewZoomed:(NSMagnificationGestureRecognizer *)recognizer
{
    _radius = _referenceRadius * expf(-[recognizer magnification]);
    if (recognizer.state == NSGestureRecognizerStateEnded) {
        _referenceRadius = _radius;
    }
    [self _updateView];
}

- (void)_viewDragged:(NSPanGestureRecognizer *)recognizer
{
    NSPoint velocity = [recognizer velocityInView:_view];
    _theta += velocity.x * 0.0001;
    _phi = fmax(-M_PI_2 + 1e-4, fmin(M_PI_2 - 1e-4, _phi - velocity.y * 0.0001));
    [self _updateView];
}

- (void)_computeMatrices
{
    _eye[0] = _center[0] + _radius * cos(_theta) * cos(_phi);
    _eye[1] = _center[1] + _radius * sin(_phi);
    _eye[2] = _center[2] + _radius * sin(_theta) * cos(_phi);
    GLKMatrix4 matrix = GLKMatrix4MakeLookAt(_eye[0], _eye[1], _eye[2],
                                             _center[0], _center[1], _center[2],
                                             _up[0], _up[1], _up[2]);
    _viewMatrix = MatrixFloat4x4FromGLKMatrix4(matrix);
    
    _aspectRatio = _view.bounds.size.width / _view.bounds.size.height;
    _projectionMatrix = MatrixFloat4x4FromGLKMatrix4(GLKMatrix4MakePerspective(_fovY, _aspectRatio, _near, _far));
    
    _dirty = false;
}

- (matrix_float4x4)projectionMatrix
{
    if (_dirty) [self _computeMatrices];
    return _projectionMatrix;
}

- (matrix_float4x4)viewMatrix
{
    if (_dirty) [self _computeMatrices];
    return _viewMatrix;
}

@end
