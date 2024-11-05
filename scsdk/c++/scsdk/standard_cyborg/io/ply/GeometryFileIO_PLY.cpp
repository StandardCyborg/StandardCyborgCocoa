/*
Copyright 2020 Standard Cyborg

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/


#include "standard_cyborg/io/ply/GeometryFileIO_PLY.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <math.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "happly.h"
#pragma clang diagnostic pop

#include "standard_cyborg/util/DebugHelpers.hpp"
#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/math/Vec3.hpp"


#define PLY_LINE_LENGTH 128

namespace standard_cyborg {
namespace io {
namespace ply {

using sc3d::Geometry;
using sc3d::Face3;

static const float kGammaCorrection = 1.0 / 2.2;
static const float kGammaCorrectionInv = 2.2 / 1.0;

// See also LinearToApproximateSRGB
static inline float applyGammaCorrection(float x)
{
    return powf(x, kGammaCorrection);
}

// See also ApproximateSRGBGammaToLinear
static inline float unapplyGammaCorrection(float x)
{
    return powf(x, kGammaCorrectionInv);
}

bool WriteGeometryToPLYFile(std::string filename, const Geometry& geometry)
{
    std::ofstream file(filename, std::ios::out | std::ios::binary);

    if (!file.good()) {
        file.close();
        return false;
    }

    bool status = WriteGeometryToPLYStream(file, geometry);

    file.close();

    return status;
}

bool ReadGeometryFromPLYFile(Geometry& geometryOut, std::string filename)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary);

    if (!file.good()) {
        file.close();
        return false;
    }

    bool status = ReadGeometryFromPLYStream(geometryOut, file);

    file.close();

    return status;
}

bool ReadGeometryFromPLYStream(Geometry& geometry, std::istream& inStream)
{
    using math::Vec3;
    using math::Vec2;
    
    happly::PLYData plyIn(inStream);
 
    if (!plyIn.hasElement("vertex")) {
        return false;
    }

    if (plyIn.getElement("vertex").hasProperty("x") && plyIn.getElement("vertex").hasProperty("y") && plyIn.getElement("vertex").hasProperty("z")) {
        std::vector<float> vertexX = plyIn.getElement("vertex").getProperty<float>("x");
        std::vector<float> vertexY = plyIn.getElement("vertex").getProperty<float>("y");
        std::vector<float> vertexZ = plyIn.getElement("vertex").getProperty<float>("z");

        size_t n = vertexX.size();
        std::vector<Vec3> positions(n);
        for (int i = 0; i < n; i++) {
            positions[i] = Vec3(
                vertexX[i],
                vertexY[i],
                vertexZ[i]);
        }

        geometry.setPositions(positions);
    }

    if (plyIn.getElement("vertex").hasProperty("s") && plyIn.getElement("vertex").hasProperty("t")) {
        std::vector<float> vertexS = plyIn.getElement("vertex").getProperty<float>("s");
        std::vector<float> vertexT = plyIn.getElement("vertex").getProperty<float>("t");

        size_t n = vertexS.size();
        std::vector<Vec2> texCoords(n);
        for (int i = 0; i < n; i++) {
            texCoords[i] = Vec2(
                vertexS[i],
                vertexT[i]);
        }

        geometry.setTexCoords(texCoords);
    }

    if (plyIn.getElement("vertex").hasProperty("nx") && plyIn.getElement("vertex").hasProperty("ny") && plyIn.getElement("vertex").hasProperty("nz")) {
        std::vector<float> vertexNx = plyIn.getElement("vertex").getProperty<float>("nx");
        std::vector<float> vertexNy = plyIn.getElement("vertex").getProperty<float>("ny");
        std::vector<float> vertexNz = plyIn.getElement("vertex").getProperty<float>("nz");

        size_t normalCount = vertexNx.size();
        std::vector<Vec3> normals(normalCount);
        for (int i = 0; i < normalCount; i++) {
            normals[i] = Vec3({vertexNx[i], vertexNy[i], vertexNz[i]});
        }

        if (plyIn.getElement("vertex").hasProperty("surfel_radius")) {
            geometry.setNormalsEncodeSurfelRadius(true);
            std::vector<float> surfelRadius = plyIn.getElement("vertex").getProperty<float>("surfel_radius");
            for (int i = 0; i < normalCount; i++) {
                // Hopefully not necessary, but if we assign a specific magnitude, normalize defensively, just in case.
                normals[i].normalize();
                normals[i] *= surfelRadius[i];
            }
        } else {
            geometry.setNormalsEncodeSurfelRadius(false);
        }

        geometry.setNormals(normals);
    }

    if (plyIn.getElement("vertex").hasProperty("red") && plyIn.getElement("vertex").hasProperty("green") && plyIn.getElement("vertex").hasProperty("blue")) {
        std::unique_ptr<happly::Property>& prop = plyIn.getElement("vertex").getPropertyPtr("red");
        std::vector<Vec3> colors;

        if (prop->propertyTypeName() == "uchar") {
            std::vector<unsigned char> vertexR = plyIn.getElement("vertex").getProperty<unsigned char>("red");
            std::vector<unsigned char> vertexG = plyIn.getElement("vertex").getProperty<unsigned char>("green");
            std::vector<unsigned char> vertexB = plyIn.getElement("vertex").getProperty<unsigned char>("blue");

            size_t n = vertexR.size();
            colors.resize(n);
            for (int i = 0; i < n; i++) {
                colors[i] = Vec3::pow(Vec3(
                                          vertexR[i] / 255.0f,
                                          vertexG[i] / 255.0f,
                                          vertexB[i] / 255.0f),
                                      kGammaCorrectionInv);
            }
        } else {
            std::vector<float> vertexR = plyIn.getElement("vertex").getProperty<float>("red");
            std::vector<float> vertexG = plyIn.getElement("vertex").getProperty<float>("green");
            std::vector<float> vertexB = plyIn.getElement("vertex").getProperty<float>("blue");

            size_t n = vertexR.size();
            colors.resize(n);
            for (int i = 0; i < n; i++) {
                colors[i] = Vec3::pow(Vec3(vertexR[i], vertexG[i], vertexB[i]), kGammaCorrectionInv);
            }
        }

        geometry.setColors(colors);
    }

    if (plyIn.hasElement("face")) {
        std::vector<std::vector<int>> faceData(plyIn.getFaceIndices<int>());
        std::vector<Face3> faces;
        faces.reserve(faceData.size());
        for (auto faceList : faceData) {
            faces.push_back(Face3({faceList[0], faceList[1], faceList[2]}));
        }
        geometry.setFaces(faces);
    }

    return true;
}


bool WriteGeometryToPLYStream(std::ostream& output, const Geometry& geometry)
{
    using math::Vec3;
    using math::Vec2;
    
    happly::PLYData plyOut;
    

    int vertexCount = geometry.vertexCount();

    if (geometry.hasPositions() || geometry.hasNormals() || geometry.hasColors()) {
        plyOut.addElement("vertex", vertexCount);
    }

    if (geometry.hasPositions()) {
        std::vector<float> vertexX(vertexCount);
        std::vector<float> vertexY(vertexCount);
        std::vector<float> vertexZ(vertexCount);

        const std::vector<Vec3>& positions = geometry.getPositions();
        for (int i = 0; i < vertexCount; i++) {
            Vec3 position = positions[i];
            vertexX[i] = position.x;
            vertexY[i] = position.y;
            vertexZ[i] = position.z;
        }

        plyOut.getElement("vertex").addProperty<float>("x", vertexX);
        plyOut.getElement("vertex").addProperty<float>("y", vertexY);
        plyOut.getElement("vertex").addProperty<float>("z", vertexZ);
    }

    if (geometry.hasNormals()) {
        std::vector<float> normalX(vertexCount);
        std::vector<float> normalY(vertexCount);
        std::vector<float> normalZ(vertexCount);
        std::vector<float> surfelRadius(vertexCount);

        const std::vector<Vec3>& normals = geometry.getNormals();
        for (int i = 0; i < vertexCount; i++) {
            Vec3 normal = normals[i];
            normalX[i] = normal.x;
            normalY[i] = normal.y;
            normalZ[i] = normal.z;
            if (geometry.normalsEncodeSurfelRadius()) {
                float norm = normal.norm();
                normalX[i] /= norm;
                normalY[i] /= norm;
                normalZ[i] /= norm;
                surfelRadius[i] = norm;
            }
        }

        plyOut.getElement("vertex").addProperty<float>("nx", normalX);
        plyOut.getElement("vertex").addProperty<float>("ny", normalY);
        plyOut.getElement("vertex").addProperty<float>("nz", normalZ);

        if (geometry.normalsEncodeSurfelRadius()) {
            plyOut.getElement("vertex").addProperty<float>("surfel_radius", surfelRadius);
        }
    }

    if (geometry.hasColors()) {
        std::vector<unsigned char> red(vertexCount);
        std::vector<unsigned char> green(vertexCount);
        std::vector<unsigned char> blue(vertexCount);

        const std::vector<Vec3>& colors = geometry.getColors();
        for (int i = 0; i < vertexCount; i++) {
            Vec3 color = Vec3::pow(colors[i], kGammaCorrection);
            red[i] = std::clamp(static_cast<int>(color.x * 255.0f), 0, 255);
            green[i] = std::clamp(static_cast<int>(color.y * 255.0f), 0, 255);
            blue[i] = std::clamp(static_cast<int>(color.z * 255.0f), 0, 255);
        }

        plyOut.getElement("vertex").addProperty<unsigned char>("red", red);
        plyOut.getElement("vertex").addProperty<unsigned char>("green", green);
        plyOut.getElement("vertex").addProperty<unsigned char>("blue", blue);
    }

    if (geometry.hasTexCoords()) {
        std::vector<float> vertexS(vertexCount);
        std::vector<float> vertexT(vertexCount);

        const std::vector<Vec2>& texCoords = geometry.getTexCoords();
        for (int i = 0; i < vertexCount; i++) {
            Vec2 texCoord = texCoords[i];
            vertexS[i] = texCoord.x;
            vertexT[i] = texCoord.y;
        }

        plyOut.getElement("vertex").addProperty<float>("s", vertexS);
        plyOut.getElement("vertex").addProperty<float>("t", vertexT);
    }

    if (geometry.hasFaces()) {
        const std::vector<Face3>& faces = geometry.getFaces();

        std::vector<std::vector<int>> facesOut(geometry.faceCount());

        for (int i = 0; i < geometry.faceCount(); i++) {
            Face3 face = faces[i];
            facesOut[i] = {face[0], face[1], face[2]};
        }

        plyOut.addElement("face", geometry.faceCount());
        plyOut.getElement("face").addListProperty<int>("vertex_indices", facesOut);
    }

    plyOut.write(output, happly::DataFormat::Binary);

    return true;
}


/** Assumes vertices are in linear color space, and writes to file in sRGB color space */
void FragileWriteGeometryToPLYStream(std::ostream& output, const Geometry& geometry)
{
    using math::Vec3;
    
    int vertexCount = geometry.vertexCount();
    int faceCount = geometry.faceCount();

    output << "ply\n";
    output << "format ascii 1.0\n";
    output << "comment StandardCyborgFusionMetadata { \"color_space\": \"sRGB\" }\n";
    output << "element vertex " << vertexCount << "\n";
    output << "property float x\n";
    output << "property float y\n";
    output << "property float z\n";
    output << "property float nx\n";
    output << "property float ny\n";
    output << "property float nz\n";
    output << "property uchar red\n";
    output << "property uchar green\n";
    output << "property uchar blue\n";
    output << "property float surfel_radius\n";
    output << "element face " << faceCount << "\n";
    output << "property list uchar int vertex_indices\n";
    output << "end_header\n";

    char buf[1024];

    for (size_t i = 0; i < vertexCount; ++i) {
        Vec3 position = geometry.getPositions()[i];
        Vec3 normal = geometry.getNormals()[i];

        Vec3 color;
        if (geometry.getColors().size() == 0) {
            color = Vec3(0.0, 0.0, 0.0);
        } else {
            color = geometry.getColors()[i];
        }

        float surfelRadius = normal.norm();
        normal /= surfelRadius;

        snprintf(buf, 1024, "%.4f %.4f %.4f %.4f %.4f %.4f %d %d %d %f\n",
                 position.x,
                 position.y,
                 position.z,
                 normal.x,
                 normal.y,
                 normal.z,
                 (int)(applyGammaCorrection(color.x) * 255.0f),
                 (int)(applyGammaCorrection(color.y) * 255.0f),
                 (int)(applyGammaCorrection(color.z) * 255.0f),
                 surfelRadius);

        output << buf;
    }

    for (size_t i = 0; i < faceCount; ++i) {
        Face3 face = geometry.getFaces()[i];
        snprintf(buf, 1024, "3 %d %d %d\n", face[0], face[1], face[2]);
        output << buf;
    }

    return;
}

bool FragileReadGeometryFromPLYFile(Geometry& geometryOut, std::string filename)
{
    using math::Vec3;
    
    FILE* fileHandle = fopen(filename.c_str(), "r");
    if (fileHandle == NULL) { return false; }

    std::vector<Vec3> positions, normals, colors;
    std::vector<Face3> faces;

    char line[PLY_LINE_LENGTH] = {0};
    int vertexCount = 0, readVertexCount = 0, faceCount = 0, readFaceCount = 0;
    float x, y, z, nx, ny, nz, r, g, b, norm;
    int face0, face1, face2;

    while (fileHandle != NULL
           && !feof(fileHandle)
           && fgets(line, PLY_LINE_LENGTH, fileHandle)) {
        if (strncmp(line, "element vertex ", strlen("element vertex ")) == 0) {
            if (sscanf(line, "element vertex %d", &vertexCount)) {
                positions.reserve(vertexCount);
                normals.reserve(vertexCount);
                colors.reserve(vertexCount);
            }
        } else if (strncmp(line, "element face ", strlen("element face ")) == 0) {
            if (sscanf(line, "element face %d", &faceCount)) {
                faces.reserve(faceCount);
            }
        } else if (readVertexCount < vertexCount && sscanf(line, "%f %f %f %f %f %f %f %f %f %f", &x, &y, &z, &nx, &ny, &nz, &r, &g, &b, &norm) == 10) {
            positions.push_back(Vec3(x, y, z));
            normals.push_back(Vec3(nx, ny, nz));
            colors.push_back(Vec3(unapplyGammaCorrection(r / 255.0f),
                                  unapplyGammaCorrection(g / 255.0f),
                                  unapplyGammaCorrection(b / 255.0f)));
            ++readVertexCount;
        } else if (readVertexCount < vertexCount && sscanf(line, "%f %f %f %f %f %f", &x, &y, &z, &nx, &ny, &nz) == 6) {
            float len = sqrt(nx * nx + ny * ny + nz * nz);

            positions.push_back(Vec3(x, y, z));
            normals.push_back(Vec3(nx / len, ny / len, nz / len));
            colors.push_back(Vec3((+1.0f + nx / len) * 0.5f,
                                  (+1.0f + ny / len) * 0.5f,
                                  (+1.0f + nz / len) * 0.5f));

            ++readVertexCount;
        } else if (readFaceCount < faceCount && sscanf(line, "3 %d %d %d", &face0, &face1, &face2)) {
            faces.push_back(Face3(face0, face1, face2));
            ++readFaceCount;
        }

        // Clear the line for the next iteration
        memset(&line, 0, PLY_LINE_LENGTH);
    }

    fclose(fileHandle);

    geometryOut.setPositions(positions);
    geometryOut.setNormals(normals);
    geometryOut.setColors(colors);
    geometryOut.setFaces(faces);

    return true;
}

} // namespace ply
} // namespace io
} // namespace standard_cyborg
