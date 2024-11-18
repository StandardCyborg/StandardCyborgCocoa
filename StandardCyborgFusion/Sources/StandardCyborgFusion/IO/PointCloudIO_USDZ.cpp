//
//  PointCloudIO_USDZ.cpp
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 3/11/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#include "PointCloudIO.hpp"
#include "GeometryHelpers.hpp"

#import <CoreGraphics/CoreGraphics.h>
#import <CoreServices/CoreServices.h>
#import <ImageIO/ImageIO.h>

bool PointCloudIO::WriteSurfelsToUSDAFile(const Surfel *surfels,
                                          size_t surfelCount,
                                          std::string filename,
                                          std::string colorMapFilename)
{
    FILE *usda = fopen(filename.c_str(), "w");
    if (usda == NULL) {
        return false;
    }
    
    float sqrtVertexCount = sqrtf((float)surfelCount);
    size_t colorImageCols = pow(2.0, ceil(log(sqrtVertexCount) / log(2.0)));
    size_t colorImageRows = ceilf((float)surfelCount / (float)colorImageCols);
    size_t colorImageByteSize = colorImageCols * colorImageRows * 3 * sizeof(uint8_t);
    
    uint8_t *colorImageData = (uint8_t *)calloc(colorImageCols * colorImageRows * 3, sizeof(uint8_t));
    
    fprintf(usda, "#usda 1.0\n");
    fprintf(usda, "(\n");
    fprintf(usda, "    defaultPrim = \"Scan\"\n");
    fprintf(usda, "    endTimeCode = 0\n");
    fprintf(usda, "    startTimeCode = 0\n");
    fprintf(usda, "    upAxis = \"X\"\n");
    fprintf(usda, ")\n");
    
    //fprintf(usda, "def Xform \"Scan\" (\n");
    //fprintf(usda, "    customData = {\n");
    //fprintf(usda, "        bool zUp = 0\n");
    //fprintf(usda, "    }\n");
    //fprintf(usda, "    kind = \"fxAsset\"\n");
    //fprintf(usda, ")\n");
    
    fprintf(usda, "def Scope \"Materials\"\n");
    fprintf(usda, "{\n");
    fprintf(usda, "    def Material \"ScanMaterial\"\n");
    fprintf(usda, "    {\n");
    
    fprintf(usda, "        token inputs:frame:stPrimvarName = \"Texture_uv\"\n");
    fprintf(usda, "        token outputs:surface.connect = </Materials/ScanMaterial/pbrMat1.outputs:surface>\n");
    
    fprintf(usda, "        def Shader \"pbrMat1\"\n");
    fprintf(usda, "        {\n");
    fprintf(usda, "            uniform token info:id = \"UsdPreviewSurface\"\n");
    fprintf(usda, "            float inputs:clearcoat = 0\n");
    fprintf(usda, "            float inputs:clearcoatRoughness = 0\n");
    fprintf(usda, "            color3f inputs:diffuseColor.connect = </Materials/ScanMaterial/color_map.outputs:rgb>\n");
    fprintf(usda, "            token outputs:surface\n");
    fprintf(usda, "        }\n");
    
    fprintf(usda, "        def Shader \"color_map\"\n");
    fprintf(usda, "        {\n");
    fprintf(usda, "            uniform token info:id = \"UsdUVTexture\"\n");
    fprintf(usda, "            float4 inputs:default = (0, 0, 0, 1)\n");
    fprintf(usda, "            asset inputs:file = @Scan_Albedo.png@\n");
    fprintf(usda, "            float2 inputs:st.connect = </Materials/ScanMaterial/Primvar.outputs:result>\n");
    fprintf(usda, "            token inputs:wrapS = \"repeat\"\n");
    fprintf(usda, "            token inputs:wrapT = \"repeat\"\n");
    fprintf(usda, "            float3 outputs:rgb\n");
    fprintf(usda, "        }\n");
    
    fprintf(usda, "        def Shader \"Primvar\"\n");
    fprintf(usda, "        {\n");
    fprintf(usda, "            uniform token info:id = \"UsdPrimvarReader_float2\"\n");
    fprintf(usda, "            float2 inputs:default = (0, 0)\n");
    fprintf(usda, "            token inputs:varname.connect = </Materials/ScanMaterial.inputs:frame:stPrimvarName>\n");
    fprintf(usda, "            float2 outputs:result\n");
    fprintf(usda, "        }\n");
    
    fprintf(usda, "    }\n");
    
    fprintf(usda, "    def Material \"lambert1\"\n");
    fprintf(usda, "    {\n");
    fprintf(usda, "        color3f inputs:displayColor = (1.5, 0.5, 0.5)\n");
    fprintf(usda, "    }\n");
    
    fprintf(usda, "}\n");
    
    fprintf(usda, "def Mesh \"Scan\"\n");
    fprintf(usda, "{\n");
    
    
    fprintf(usda, "    uniform token orientation = \"rightHanded\"\n");
    fprintf(usda, "    rel material:binding = </Materials/ScanMaterial>\n");
    
    
    fprintf(usda, "    int[] faceVertexCounts = [\n");
    for (size_t i = 0; i < surfelCount; ++i) {
        fprintf(usda, i == surfelCount - 1 ? "3\n" : "3,\n");
    }
    fprintf(usda, "    ]\n");
    
    
    fprintf(usda, "    int[] faceVertexIndices = [\n");
    for (size_t i = 0; i < surfelCount; ++i) {
        unsigned long i3 = i * 3;
        fprintf(usda, "%lu,%lu,%lu", i3, i3 + 1, i3 + 2);
        if (i < surfelCount - 1) {
            fprintf(usda, ",\n");
        } else {
            fprintf(usda, "\n");
        }
    }
    fprintf(usda, "    ]\n");
    
    
    
    fprintf(usda, "    point3f[] points = [\n");
    
    const float radius = 0.0008; // Used to come from normal.norm()
    const float positionScale = 100.0f;
    Vector3f lo(INFINITY, INFINITY, INFINITY);
    Vector3f hi(-INFINITY, -INFINITY, -INFINITY);
    Vector3f position, normal, color, p1, p2, p3, tangent, bitangent;
    Vector3f up(0.0, 1.0, 0.0);
    
    for (size_t i = 0, i3 = 0; i < surfelCount; ++i, i3 += 3) {
        const Surfel& surfel = surfels[i];
        position = surfel.position;
        normal = surfel.normal;
        color = surfel.color;
        
        float r = unapplyGammaCorrection(color.x());
        float g = unapplyGammaCorrection(color.y());
        float b = unapplyGammaCorrection(color.z());
        colorImageData[i3]     = std::max(0, std::min(255, (int)(floor(r * 256.0))));
        colorImageData[i3 + 1] = std::max(0, std::min(255, (int)(floor(g * 256.0))));
        colorImageData[i3 + 2] = std::max(0, std::min(255, (int)(floor(b * 256.0))));
        
        lo = lo.cwiseMin(position);
        hi = hi.cwiseMax(position);
        
        tangent = normal.cross(up);
        tangent.normalize();
        bitangent = tangent.cross(normal);
        bitangent.normalize();
        
        p1 = position + radius * (-0.866 * tangent - 0.5 * bitangent);
        p2 = position + radius * ( 0.866 * tangent - 0.5 * bitangent);
        p3 = position + radius * (bitangent);
        
        p1 *= positionScale;
        p2 *= positionScale;
        p3 *= positionScale;
        
        fprintf(usda, "(%f,%f,%f),", p1.y(), p1.x(), p1.z());
        fprintf(usda, "(%f,%f,%f),", p2.y(), p2.x(), p2.z());
        fprintf(usda, "(%f,%f,%f)",  p3.y(), p3.x(), p3.z());
        
        if (i < surfelCount - 1) {
            fprintf(usda, ",\n");
        } else {
            fprintf(usda, "\n");
        }
    }
    fprintf(usda, "    ]\n");
    
    
    
    /*
     fprintf(usda, "    normal3f[] primvars:normals = [\n");
     for (size_t i = 0, i3 = 0; i < vertexCount; ++i, i3 += 3) {
     normal = normals.col(i);
     normal.normalize();
     
     fprintf(usda, "(%f,%f,%f)", -normal.y(), normal.x(), normal.z());
     
     if (i < vertexCount - 1) {
     fprintf(usda, ",\n");
     } else {
     fprintf(usda, "\n");
     }
     }
     fprintf(usda, "    ]\n");
     
     
     
     fprintf(usda, "    int[] primvars:normals:indices = [\n");
     for (size_t i = 0; i < vertexCount; ++i) {
     fprintf(usda, "%lu,%lu,%lu", i, i, i);
     if (i < vertexCount - 1) {
     fprintf(usda, ",\n");
     } else {
     fprintf(usda, "\n");
     }
     }
     fprintf(usda, "    ]\n");
     */
    
    
    
    
    
    fprintf(usda, "    float3[] extent = [\n");
    fprintf(usda, "        (%f,%f,%f),\n", lo(1) * positionScale, lo(0) * positionScale, lo(2) * positionScale);
    fprintf(usda, "        (%f,%f,%f)\n",  hi(1) * positionScale, hi(0) * positionScale, hi(2) * positionScale);
    fprintf(usda, "    ]\n");
    
    
    
    
    fprintf(usda, "    float2[] primvars:Texture_uv = [\n");
    for (size_t i = 0; i < surfelCount; ++i) {
        float s = ((float)(i / colorImageCols) + 0.5) / colorImageRows;
        float t = ((float)(i % colorImageCols) + 0.5) / colorImageCols;
        
        fprintf(usda, "(%f,%f)", t, 1.0 - s);
        
        if (i < surfelCount - 1) {
            fprintf(usda, ",\n");
        } else {
            fprintf(usda, "] (\n");
        }
    }
    fprintf(usda, "        interpolation = \"faceVarying\"\n");
    fprintf(usda, "    )\n");
    
    
    
    fprintf(usda, "    int[] primvars:Texture_uv:indices = [\n");
    for (size_t i = 0; i < surfelCount; ++i) {
        fprintf(usda, "%lu,%lu,%lu", i, i, i);
        if (i < surfelCount - 1) {
            fprintf(usda, ",\n");
        } else {
            fprintf(usda, "\n");
        }
    }
    fprintf(usda, "    ]\n");
    
    
    
    
    
    fprintf(usda, "    uniform token subdivisionScheme = \"none\"\n");
    fprintf(usda, "}\n");
    
    fclose(usda);
    
    CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
    
    CFDataRef rgbData = CFDataCreate(NULL, colorImageData, colorImageByteSize);
    CGDataProviderRef provider = CGDataProviderCreateWithCFData(rgbData);
    CGImageRef rgbImageRef = CGImageCreate(colorImageCols,
                                           colorImageRows,
                                           8,
                                           24,
                                           colorImageCols * 3,
                                           colorspace,
                                           kCGBitmapByteOrderDefault,
                                           provider,
                                           NULL,
                                           true,
                                           kCGRenderingIntentDefault);
    CFRelease(rgbData);
    CGDataProviderRelease(provider);
    CGColorSpaceRelease(colorspace);
    
    // Delete the file if it already exists
    remove(colorMapFilename.c_str());
    
    CFStringRef colorImageFilePath = CFStringCreateWithFormat(NULL, NULL, CFSTR("file://%s"), colorMapFilename.c_str());
    CFURLRef colorImageURL = CFURLCreateWithString(NULL, colorImageFilePath, NULL);
    
    CGImageDestinationRef destination = CGImageDestinationCreateWithURL(colorImageURL, kUTTypePNG, 1, nil);
    CGImageDestinationAddImage(destination, rgbImageRef, nil);
    CGImageDestinationFinalize(destination);
    
    CGImageRelease(rgbImageRef);
    if (colorImageURL != NULL) { CFRelease(colorImageURL); }
    if (colorImageFilePath != NULL) { CFRelease(colorImageFilePath); }
    if (destination != NULL) { CFRelease(destination); }
    free(colorImageData);
    
    return true;
}
