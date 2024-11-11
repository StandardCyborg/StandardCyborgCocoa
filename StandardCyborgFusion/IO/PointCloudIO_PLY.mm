//
//  PointCloudIO_PLY.mm
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 3/11/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#include "PointCloudIO.hpp"
#include "GeometryHelpers.hpp"

#include <iostream>
#import <Foundation/Foundation.h>

#define PLY_LINE_LENGTH 128

bool PointCloudIO::ReadSurfelsFromPLYFile(Surfels& surfels, std::string filename, bool normalizeNormals)
{
    FILE *fileHandle = fopen(filename.c_str(), "r");
    if (fileHandle == NULL) { return false; }
    
    std::cout << "Reading surfels from PLY file \"" << filename << "\"" << std::endl;
    
    char line[PLY_LINE_LENGTH];
    memset(&line, 0, PLY_LINE_LENGTH);
    
    int vertexCount = 0;
    bool flipNormals = true; // Due to a bug, normals were inverted until version 1.1.0
    bool unapplyGamma = false; // Due to a bug, gamma was not applied correctly until version 1.1.0
    Surfel surfel = {};
    float x, y, z, nx, ny, nz, r, g, b, surfelRadius;
    
    while (fileHandle != NULL
           && !feof(fileHandle)
           && fgets(line, PLY_LINE_LENGTH, fileHandle))
    {
        if (strncmp(line, "element vertex ", 15) == 0) {
            if (sscanf(line, "element vertex %d", &vertexCount)) {
                surfels.reserve(vertexCount);
            }
        }
        else if (sscanf(line, "%f %f %f %f %f %f %f %f %f %f", &x, &y, &z, &nx, &ny, &nz, &r, &g, &b, &surfelRadius)) {
            if (flipNormals) {
                nx = -nx;
                ny = -ny;
                nz = -nz;
            }
            // This should be normalized already, but just in case so that we are sure to preserve
            // the physical meaning of the normal magnitude, normalize it before mulitplying it by
            // the surfel radius.
            float normalMagnitude = std::sqrt(nx * nx + ny * ny + nz * nz);
            float scaleFactor = (normalizeNormals ? 1.0 : surfelRadius) / normalMagnitude;
            nx *= scaleFactor;
            ny *= scaleFactor;
            nz *= scaleFactor;

            surfel.position.x() = x;
            surfel.position.y() = y;
            surfel.position.z() = z;
            surfel.normal.x() = nx;
            surfel.normal.y() = ny;
            surfel.normal.z() = nz;
            surfel.color.x() = unapplyGamma ? unapplyGammaCorrection(r / 255.0f) : r / 255.0f;
            surfel.color.y() = unapplyGamma ? unapplyGammaCorrection(g / 255.0f) : g / 255.0f;
            surfel.color.z() = unapplyGamma ? unapplyGammaCorrection(b / 255.0f) : b / 255.0f;
            surfel.surfelSize = surfelRadius;

            surfels.push_back(surfel);
        }
        else if (strncmp(line, "comment StandardCyborgFusionVersion ", strlen("comment StandardCyborgFusionVersion ")) == 0) {
            // Don't even bother reading the version string, as we didn't write it until version 1.1.0
            flipNormals = false;
            unapplyGamma = true;
        }
        
        // Clear the line for the next iteration
        memset(&line, 0, PLY_LINE_LENGTH);
    }
    
    fclose(fileHandle);
    
    return true;
}

bool PointCloudIO::WriteSurfelsToPLYFile(const Surfel* surfels,
                                         size_t surfelCount,
                                         Eigen::Vector3f gravity,
                                         std::string filename)
{
    FILE *file = fopen(filename.c_str(), "w");
    if (file == NULL) {
        return false;
    }
    
    fprintf(file, "ply\n");
    fprintf(file, "format ascii 1.0\n");
    fprintf(file, "comment StandardCyborgFusionVersion %s\n", SCFrameworkVersion());
    fprintf(file, "comment StandardCyborgFusionMetadata { \"color_space\": \"sRGB\" }\n");
    fprintf(file, "element vertex %ld\n", surfelCount);
    fprintf(file, "property float x\n");
    fprintf(file, "property float y\n");
    fprintf(file, "property float z\n");
    fprintf(file, "property float nx\n");
    fprintf(file, "property float ny\n");
    fprintf(file, "property float nz\n");
    fprintf(file, "property uchar red\n");
    fprintf(file, "property uchar green\n");
    fprintf(file, "property uchar blue\n");
    fprintf(file, "property float surfel_radius\n");
    fprintf(file, "element face 0\n");
    fprintf(file, "property list uchar int vertex_indices\n");
    fprintf(file, "end_header\n");
    
    for (size_t i = 0; i < surfelCount; ++i) {
        const Surfel &surfel = surfels[i];
        
        Vector3f normal = surfel.normal;
        float surfelRadius = surfel.surfelSize;
        
        fprintf(file, "%.4f %.4f %.4f %.4f %.4f %.4f %d %d %d %f\n",
                surfel.position.x(),
                surfel.position.y(),
                surfel.position.z(),
                normal.x(),
                normal.y(),
                normal.z(),
                (int)(applyGammaCorrection(surfel.color.x()) * 255.0f),
                (int)(applyGammaCorrection(surfel.color.y()) * 255.0f),
                (int)(applyGammaCorrection(surfel.color.z()) * 255.0f),
                surfelRadius
                );
    }
    
    fclose(file);
    
    return true;
}

std::unique_ptr<RawFrame> PointCloudIO::ReadRawFrameFromBPLYFile(std::string filename)
{
    
    FILE *fileHandle = fopen(filename.c_str(), "r");
    if (fileHandle == NULL) { return nullptr; }
    
    char line[PLY_LINE_LENGTH];
    memset(&line, 0, PLY_LINE_LENGTH);
    
    int metadataLength = 0;
    int colorCount = 0;
    int depthCount = 0;
    
    while (fileHandle != NULL
           && !feof(fileHandle)
           && fgets(line, PLY_LINE_LENGTH, fileHandle))
    {
        if (strncmp(line, "element metadata ", strlen("element metadata ")) == 0) {
            sscanf(line, "element metadata %d", &metadataLength);
        }
        else if (strncmp(line, "element color ", strlen("element color ")) == 0) {
            sscanf(line, "element color %d", &colorCount);
        }
        else if (strncmp(line, "element depth ", strlen("element depth ")) == 0) {
            sscanf(line, "element depth %d", &depthCount);
        }
        else if (strncmp(line, "end_header", strlen("end_header")) == 0) {
            break;
        }
        
        // Clear the line for the next iteration
        memset(&line, 0, PLY_LINE_LENGTH);
    }
    
    long startOfBodyFilePosition = ftell(fileHandle);
    NSData *fileData = [NSData dataWithContentsOfFile:[NSString stringWithUTF8String:filename.c_str()]
                                              options:NSDataReadingMapped
                                                error:NULL];
    uint8_t *data = (uint8_t *)[fileData bytes];
    
    size_t metadataStartOffset = (size_t)startOfBodyFilePosition;
    size_t colorStartOffset = metadataStartOffset + metadataLength;
    size_t depthStartOffset = colorStartOffset + colorCount * 3 * sizeof(float);
    char *metadataData = (char *)(data + metadataStartOffset);
    float *colorData = (float *)(data + colorStartOffset);
    float *depthData = (float *)(data + depthStartOffset);
    
    // Workaround: I don't know how to tell JSON to stop parsing a certain number of bytes in,
    //             so I'm just copying the whole thing to a null-terminated char array
    char *nullTerminatedJSONString = (char *)calloc(metadataLength + 1, sizeof(char));
    memcpy(nullTerminatedJSONString, metadataData, metadataLength);
    JSON metadataJSON = JSON::parse(nullTerminatedJSONString);
    free(nullTerminatedJSONString);
    
    double timestamp = metadataJSON["timestamp"];
    sc3d::PerspectiveCamera camera = PerspectiveCameraFromJSON(metadataJSON["camera_intrinsics"]);
    
    int width = metadataJSON["width"];
    int height = metadataJSON["height"];
    
    std::vector<math::Vec3> colorValues(colorCount, math::Vec3());
    for (int ii = 0; ii < colorCount; ++ii) {
        colorValues[ii] = math::Vec3(colorData[3*ii+0], colorData[3*ii+1], colorData[3*ii+2]);
    }
    
    std::vector<float> depthValues(depthCount, 0.0f);
    for (int ii = 0; ii < depthValues.size(); ++ii) {
        depthValues[ii] = depthData[ii];
    }
    
    fclose(fileHandle);
    
    std::unique_ptr<RawFrame> rawFrame(new RawFrame(camera, width, height, depthValues, colorValues, timestamp));
    
    return rawFrame;
}

void PointCloudIO::WriteRawFrameToBPLYFile(const RawFrame& rawFrame, std::string filename)
{
    // Build the metadata first so we can know its size when writing the headers
    JSON metadata;
    metadata["color_space"] = "sRGB";
    metadata["absolute_timestamp"] = CFAbsoluteTimeGetCurrent();
    metadata["timestamp"] = [[NSProcessInfo processInfo] systemUptime];
    metadata["camera_intrinsics"] = JSONFromPerspectiveCamera(rawFrame.camera);
    metadata["width"] = rawFrame.width;
    metadata["height"] = rawFrame.height;
    std::string metadataString = metadata.dump();
    
    
    FILE *file = fopen(filename.c_str(), "w");
    
    // Header
    fprintf(file, "ply\n");
    fprintf(file, "format binary_little_endian 1.0\n");
    fprintf(file, "comment StandardCyborgFusionVersion %s\n", SCFrameworkVersion());
    fprintf(file, "element metadata %ld\n", metadataString.size());
    fprintf(file, "property uchar char\n");
    fprintf(file, "element color %ld\n", rawFrame.colors.size());
    fprintf(file, "property float r\n");
    fprintf(file, "property float g\n");
    fprintf(file, "property float b\n");
    fprintf(file, "element depth %ld\n", rawFrame.depths.size());
    fprintf(file, "property float d\n");
    fprintf(file, "end_header\n");
    
    // Metadata
    fwrite(metadataString.c_str(), 1, metadataString.size(), file);
    
    // Color buffer
    const size_t componentSize = sizeof(float);
    const size_t componentCount = 3;
    for (int i = 0; i < rawFrame.colors.size(); i++) {
        const void* address = &(rawFrame.colors[i]);
        fwrite(address, componentSize, componentCount, file);
    }
    
    // Depth buffer
    const float *depthBaseAddress = rawFrame.depths.data();
    const size_t depthSizeBytes = sizeof(float);
    const size_t depthCount = rawFrame.depths.size();
    fwrite((const void *)depthBaseAddress, depthSizeBytes, depthCount, file);
    
    fclose(file);
}
