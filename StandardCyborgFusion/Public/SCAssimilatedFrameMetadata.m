//
//  SCAssimilatedFrameMetadata.c
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 6/4/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import "SCAssimilatedFrameMetadata.h"
#include <math.h>

simd_float3 EulerAnglesFromSCAssimilatedFrameMetadata(SCAssimilatedFrameMetadata metadata) {
    matrix_float4x4 matrix = metadata.viewMatrix;
    
    float xAngle = atan2(-matrix.columns[2].y, matrix.columns[2].z);
    
    float cosYAngle = sqrt(pow(matrix.columns[0].x, 2) + pow(matrix.columns[1].x, 2));
    float yAngle = atan2(matrix.columns[2].x, cosYAngle);
    
    float sinXAngle = sin(xAngle);
    float cosXAngle = cos(xAngle);
    float zAngle = atan2(cosXAngle * matrix.columns[0].y + sinXAngle * matrix.columns[0].z, cosXAngle * matrix.columns[1].y + sinXAngle * matrix.columns[1].z);
    
    return simd_make_float3(xAngle, yAngle, zAngle);
}
