//
//  PointCloudIO.mm
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 8/4/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#include <StandardCyborgFusion/PointCloudIO.hpp>
#include <StandardCyborgFusion/GeometryHelpers.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <sys/mman.h>

using namespace Eigen;

#define OBJ_LINE_LENGTH 128

static inline size_t __countVerticesInOBJFileWithHandle(FILE *fileHandle)
{
    int vertexCount = 0;
    char line[OBJ_LINE_LENGTH];
    memset(&line, 0, OBJ_LINE_LENGTH);
    
    while (fileHandle != NULL
           && !feof(fileHandle)
           && fgets(line, OBJ_LINE_LENGTH, fileHandle))
    {
        if (strncmp(line, "v ", 2) == 0) { vertexCount++; }
    }
    
    fseek(fileHandle, 0, 0); // Please be kind, rewind
    
    return vertexCount;
}

bool PointCloudIO::ReadVerticesFromOBJFile(Matrix3Xf& vertices,
                                           Matrix3Xf& normals,
                                           Matrix3Xf& colors,
                                           std::string filename)
{
    FILE *fileHandle = fopen(filename.c_str(), "r");
    if (fileHandle == NULL) { return false; }
    
    std::cout << "Reading vertices from OBJ file \"" << filename << "\"" << std::endl;
    
    size_t vertexCount = __countVerticesInOBJFileWithHandle(fileHandle);
    vertices.resize(Eigen::NoChange, vertexCount);
    normals.resize(Eigen::NoChange, vertexCount);
    colors.resize(Eigen::NoChange, vertexCount);
    
    char line[OBJ_LINE_LENGTH];
    memset(&line, 0, OBJ_LINE_LENGTH);
    
    int vertexIndex = 0;
    bool flipNormals = true; // Due to a bug, normals were inverted until version 1.1.0
    float x, y, z, r, g, b;
    
    while (fileHandle != NULL
           && !feof(fileHandle)
           && fgets(line, OBJ_LINE_LENGTH, fileHandle))
    {
        if (strncmp(line, "vn ", 3) == 0) {
            if (sscanf(line, "vn %f %f %f", &x, &y, &z)) {
                normals(0, vertexIndex) = x;
                normals(1, vertexIndex) = y;
                normals(2, vertexIndex) = z;
                // Don't skip to the next vertex index yet because we expect the corresponding vertex to be on the next line
            }
        }
        else if (strncmp(line, "v ", 2) == 0) {
            if (sscanf(line, "v %f %f %f %f %f %f", &x, &y, &z, &r, &g, &b)) {
                vertices(0, vertexIndex) = x;
                vertices(1, vertexIndex) = y;
                vertices(2, vertexIndex) = z;
                colors(0, vertexIndex) = unapplyGammaCorrection(r);
                colors(1, vertexIndex) = unapplyGammaCorrection(g);
                colors(2, vertexIndex) = unapplyGammaCorrection(b);
                vertexIndex++;
            }
        } else if (strncmp(line, "# StandardCyborgFusionVersion ", strlen("# StandardCyborgFusionVersion ")) == 0) {
            // Don't even bother reading the version string, as we didn't write it until version 1.1.0
            flipNormals = false;
        }
        
        // Clear the line for the next iteration
        memset(&line, 0, OBJ_LINE_LENGTH);
    }
    
    fclose(fileHandle);
    
    if (flipNormals) {
        for (size_t i = 0; i < vertexCount; ++i) {
            normals(0, i) = -normals(0, i);
            normals(1, i) = -normals(1, i);
            normals(2, i) = -normals(2, i);
        }
    }
    
    return true;
}

bool PointCloudIO::ReadSurfelsFromOBJFile(Surfels& surfels,
                                          std::string filename)
{
    FILE *fileHandle = fopen(filename.c_str(), "r");
    if (fileHandle == NULL) { return false; }
    
    std::cout << "Reading vertices from OBJ file \"" << filename << "\"" << std::endl;
    
    size_t vertexCount = __countVerticesInOBJFileWithHandle(fileHandle);
    surfels.resize(vertexCount);
    
    char line[OBJ_LINE_LENGTH];
    memset(&line, 0, OBJ_LINE_LENGTH);
    
    int vertexIndex = 0;
    int normalIndex = 0;
    bool flipNormals = true; // Due to a bug, normals were inverted until version 1.1.0
    float x, y, z, r, g, b;
    
    while (fileHandle != NULL
           && !feof(fileHandle)
           && fgets(line, OBJ_LINE_LENGTH, fileHandle))
    {
        if (strncmp(line, "vn ", 3) == 0) {
            if (sscanf(line, "vn %f %f %f", &x, &y, &z)) {
                Surfel &surfel = surfels[normalIndex];
                surfel.normal.x() = x;
                surfel.normal.y() = y;
                surfel.normal.z() = z;
                // Don't skip to the next vertex index yet because we expect the corresponding vertex to be on the next line
                normalIndex++;
            }
        }
        else if (strncmp(line, "v ", 2) == 0) {
            if (sscanf(line, "v %f %f %f %f %f %f", &x, &y, &z, &r, &g, &b)) {
                Surfel &surfel = surfels[vertexIndex];
                surfel.position.x() = x;
                surfel.position.y() = y;
                surfel.position.z() = z;
                surfel.color.x() = unapplyGammaCorrection(r);
                surfel.color.y() = unapplyGammaCorrection(g);
                surfel.color.z() = unapplyGammaCorrection(b);
                vertexIndex++;
            }
        } else if (strncmp(line, "# StandardCyborgFusionVersion ", strlen("# StandardCyborgFusionVersion ")) == 0) {
            // Don't even bother reading the version string, as we didn't write it until version 1.1.0
            flipNormals = false;
        }
        
        // Clear the line for the next iteration
        memset(&line, 0, OBJ_LINE_LENGTH);
    }
    
    fclose(fileHandle);
    
    if (flipNormals) {
        for (size_t i = 0; i < vertexCount; ++i) {
            Surfel &surfel = surfels[i];
            surfel.normal.x() = -surfel.normal.x();
            surfel.normal.y() = -surfel.normal.y();
            surfel.normal.z() = -surfel.normal.z();
        }
    }
    
    return true;
}

extern bool PointCloudIO::WriteVerticesToOBJFile(const Matrix3Xf& vertices,
                                                 const Matrix3Xf& normals,
                                                 const Matrix3Xf& colors,
                                                 std::string filename)
{
    FILE *file = fopen(filename.c_str(), "w");
    if (file == NULL) {
        return false;
    }
    
    fprintf(file, "# StandardCyborgFusionVersion %s\n", SCFrameworkVersion());
    fprintf(file, "# StandardCyborgFusionMetadata { \"color_space\": \"sRGB\" }\n");
    
    for (size_t i = 0; i < vertices.cols(); ++i) {
        fprintf(file, "v %f %f %f %f %f %f\n",
                vertices(0, i),
                vertices(1, i),
                vertices(2, i),
                applyGammaCorrection(colors(0, i)),
                applyGammaCorrection(colors(1, i)),
                applyGammaCorrection(colors(2, i)));
    }
    
    for (size_t i = 0; i < normals.cols(); ++i) {
        Vector3f normal = normals.col(i).normalized();
        fprintf(file, "vn %f %f %f\n",
                normal.x(),
                normal.y(),
                normal.z());
    }
    
    fclose(file);
    
    return true;
}

extern bool PointCloudIO::WriteSurfelsToOBJFile(const Surfel* surfels,
                                                size_t surfelCount,
                                                std::string filename)
{
    FILE *file = fopen(filename.c_str(), "w");
    if (file == NULL) {
        return false;
    }
    
    fprintf(file, "# StandardCyborgFusionVersion %s\n", SCFrameworkVersion());
    fprintf(file, "# StandardCyborgFusionMetadata { \"color_space\": \"sRGB\" }\n");
    
    for (size_t i = 0; i < surfelCount; ++i) {
        const Surfel& surfel = surfels[i];
        fprintf(file, "v %f %f %f %f %f %f\n",
                surfel.position.x(),
                surfel.position.y(),
                surfel.position.z(),
                applyGammaCorrection(surfel.color.x()),
                applyGammaCorrection(surfel.color.y()),
                applyGammaCorrection(surfel.color.z()));
    }
    
    for (size_t i = 0; i < surfelCount; ++i) {
        const Surfel& surfel = surfels[i];
        Vector3f normalizedNormal = surfel.normal;
        fprintf(file, "vn %f %f %f\n",
                normalizedNormal.x(),
                normalizedNormal.y(),
                normalizedNormal.z());
    }
    
    fclose(file);
    
    return true;
}
