//
//  SurfaceTrimmerExecute.hpp
//  PoissonRecon
//
//  Created by Aaron Thompson on 2/12/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#ifndef SurfaceTrimmerExecute_hpp
#define SurfaceTrimmerExecute_hpp

#include "MyMiscellany.h"
#include "FEMTree.h"
#include "Ply.h"
#include "PointStreamData.h"
#include <iostream>

/**
 Command-line parameters:
 {
 "In": "/path/to/input.ply",
 "Out": "/path/to/output.ply",
 "Smooth": 5,
 "Trim": 5,
 "IslandAreaRatio": 0.001,
 "PolygonMesh": false,
 "Verbose": false
 }
 */

long long EdgeKey(int key1, int key2)
{
    if (key1 < key2) {
        return (((long long)key1) << 32) | ((long long)key2);
    } else {
        return (((long long)key2) << 32) | ((long long)key1);
    }
}

template <typename ... VertexData>
PlyVertexWithData<float, 3, MultiPointStreamData<float, PointStreamValue<float>, VertexData...>>
InterpolateVertices(const PlyVertexWithData<float, 3, MultiPointStreamData<float, PointStreamValue<float>, VertexData...>>& v1,
                    const PlyVertexWithData<float, 3, MultiPointStreamData<float, PointStreamValue<float>, VertexData...>>& v2,
                    float value)
{
    if (std::get<0>(v1.data.data).data == std::get<0>(v2.data.data).data) {
        return (v1 + v2) / float(2.0);
    }
    
    float dx = (std::get<0>(v1.data.data).data - value) / (std::get<0>(v1.data.data).data - std::get<0>(v2.data.data).data);
    
    return v1 * (1.f - dx) + v2 * dx;
}

template <typename ... VertexData>
void SmoothValues(std::vector<PlyVertexWithData<float, 3, MultiPointStreamData<float, PointStreamValue<float>, VertexData...>>>& vertices,
                  const std::vector<std::vector<int>>& polygons)
{
    std::vector<int> count(vertices.size());
    std::vector<float> sums(vertices.size(), 0);
    
    for (size_t i = 0; i < polygons.size(); i++) {
        int sz = int(polygons[i].size());
        
        for (int j = 0; j < sz; j++) {
            int j1 = j;
            int j2 = (j + 1) % sz;
            int v1 = polygons[i][j1];
            int v2 = polygons[i][j2];
            count[v1]++;
            count[v2]++;
            sums[v1] += std::get<0>(vertices[v2].data.data).data;
            sums[v2] += std::get<0>(vertices[v1].data.data).data;
        }
    }
    
    for (size_t i = 0; i < vertices.size(); i++) {
        std::get<0>(vertices[i].data.data).data = (sums[i] + std::get<0>(vertices[i].data.data).data) / (count[i] + 1);
    }
}

template <typename ... VertexData>
void SplitPolygon(const std::vector<int>& polygon,
                  std::vector<PlyVertexWithData<float, 3, MultiPointStreamData<float, PointStreamValue<float>, VertexData...>>>& vertices,
                  std::vector<std::vector<int>>* ltPolygons, std::vector<std::vector<int>>* gtPolygons,
                  std::vector<bool>* ltFlags, std::vector<bool>* gtFlags,
                  std::unordered_map<long long, int>& vertexTable,
                  float trimValue)
{
    int sz = int(polygon.size());
    std::vector<bool> gt(sz);
    int gtCount = 0;
    
    for (int j = 0; j < sz; j++) {
        gt[j] = (std::get<0>(vertices[polygon[j]].data.data).data > trimValue);
        
        if (gt[j]) {
            gtCount++;
        }
    }
    
    if (gtCount == sz) {
        if (gtPolygons) {
            gtPolygons->push_back(polygon);
        }
        if (gtFlags) {
            gtFlags->push_back(false);
        }
    }
    else if (gtCount == 0) {
        if (ltPolygons) {
            ltPolygons->push_back(polygon);
        }
        if (ltFlags) {
            ltFlags->push_back(false);
        }
    }
    else {
        int start;
        for (start = 0; start < sz; start++) {
            if (gt[start] && !gt[(start + sz - 1) % sz]) {
                break;
            }
        }
        
        bool gtFlag = true;
        std::vector<int> poly;
        
        // Add the initial vertex
        {
            int j1 = (start + int(sz) - 1) % sz, j2 = start;
            int v1 = polygon[j1], v2 = polygon[j2];
            int vIdx;
            std::unordered_map<long long, int>::iterator iter = vertexTable.find(EdgeKey(v1, v2));
            
            if (iter == vertexTable.end()) {
                vertexTable[EdgeKey(v1, v2)] = vIdx = int(vertices.size());
                vertices.push_back(InterpolateVertices(vertices[v1], vertices[v2], trimValue));
            } else {
                vIdx = iter->second;
            }
            
            poly.push_back(vIdx);
        }
        
        for (int _j = 0; _j <= sz; _j++) {
            int j1 = (_j + start + sz - 1) % sz, j2 = (_j + start) % sz;
            int v1 = polygon[j1], v2 = polygon[j2];
            
            if (gt[j2] == gtFlag) {
                poly.push_back(v2);
            } else {
                int vIdx;
                std::unordered_map<long long, int>::iterator iter = vertexTable.find(EdgeKey(v1, v2));
                
                if (iter == vertexTable.end()) {
                    vertexTable[EdgeKey(v1, v2)] = vIdx = int(vertices.size());
                    vertices.push_back(InterpolateVertices(vertices[v1], vertices[v2], trimValue));
                } else {
                    vIdx = iter->second;
                }
                
                poly.push_back(vIdx);
                
                if (gtFlag)
                {
                    if (gtPolygons) {
                        gtPolygons->push_back(poly);
                    }
                    if (ltFlags) {
                        ltFlags->push_back(true);
                    }
                }
                else
                {
                    if (ltPolygons) {
                        ltPolygons->push_back(poly);
                    }
                    if (gtFlags) {
                        gtFlags->push_back(true);
                    }
                }
                poly.clear(), poly.push_back(vIdx), poly.push_back(v2);
                gtFlag = !gtFlag;
            }
        }
    }
}

template <class Vertex>
void Triangulate(const std::vector<Vertex>& vertices, const std::vector<std::vector<int>>& polygons, std::vector<std::vector<int>>& triangles)
{
    triangles.clear();
    
    for (size_t i = 0; i < polygons.size(); i++) {
        if (polygons.size() > 3) {
            std::vector<Point<float, 3>> _vertices(polygons[i].size());
            for (int j = 0; j < int(polygons[i].size()); j++) {
                _vertices[j] = vertices[polygons[i][j]].point;
            }
            
            std::vector<TriangleIndex> _triangles = MinimalAreaTriangulation<float, 3>((ConstPointer(Point<float, 3>))GetPointer(_vertices), _vertices.size());
            
            // Add the triangles to the mesh
            size_t idx = triangles.size();
            triangles.resize(idx + _triangles.size());
            
            for (int j = 0; j < int(_triangles.size()); j++) {
                triangles[idx + j].resize(3);
                for (int k = 0; k < 3; k++) {
                    triangles[idx + j][k] = polygons[i][_triangles[j].idx[k]];
                }
            }
        }
        else if (polygons[i].size() == 3) {
            triangles.push_back(polygons[i]);
        }
    }
}

template <class Vertex>
double PolygonArea(const std::vector<Vertex>& vertices, const std::vector<int>& polygon)
{
    if (polygon.size() < 3) {
        return 0.0;
    }
    else if (polygon.size() == 3) {
        return Area(vertices[polygon[0]].point, vertices[polygon[1]].point, vertices[polygon[2]].point);
    }
    else {
        Point<float, 3> center;
        for (size_t i = 0; i < polygon.size(); i++) {
            center += vertices[polygon[i]].point;
        }
        
        center /= float(polygon.size());
        double area = 0;
        
        for (size_t i = 0; i < polygon.size(); i++) {
            area += Area(center, vertices[polygon[i]].point, vertices[polygon[(i + 1) % polygon.size()]].point);
        }
        
        return area;
    }
}

template <class Vertex>
void RemoveHangingVertices(std::vector<Vertex>& vertices, std::vector<std::vector<int>>& polygons)
{
    std::unordered_map<int, int> vMap;
    std::vector<bool> vertexFlags(vertices.size(), false);
    
    for (size_t i = 0; i < polygons.size(); i++) {
        for (size_t j = 0; j < polygons[i].size(); j++) {
            vertexFlags[polygons[i][j]] = true;
        }
    }
    
    int vCount = 0;
    for (int i = 0; i < int(vertices.size()); i++) {
        if (vertexFlags[i]) {
            vMap[i] = vCount++;
        }
    }
    
    for (size_t i = 0; i < polygons.size(); i++) {
        for (size_t j = 0; j < polygons[i].size(); j++) {
            polygons[i][j] = vMap[polygons[i][j]];
        }
    }
    
    std::vector<Vertex> _vertices(vCount);
    for (int i = 0; i < int(vertices.size()); i++) {
        if (vertexFlags[i]) {
            _vertices[vMap[i]] = vertices[i];
        }
    }
    
    vertices = _vertices;
}

void SetConnectedComponents(const std::vector<std::vector<int>>& polygons,
                            std::vector<std::vector<int>>& components)
{
    std::vector<int> polygonRoots(polygons.size());
    for (size_t i = 0; i < polygons.size(); i++) {
        polygonRoots[i] = int(i);
    }
    
    std::unordered_map<long long, int> edgeTable;
    for (size_t i = 0; i < polygons.size(); i++) {
        int sz = int(polygons[i].size());
        
        for (int j = 0; j < sz; j++) {
            int j1 = j, j2 = (j + 1) % sz;
            int v1 = polygons[i][j1], v2 = polygons[i][j2];
            long long eKey = EdgeKey(v1, v2);
            std::unordered_map<long long, int>::iterator iter = edgeTable.find(eKey);
            
            if (iter == edgeTable.end()) {
                edgeTable[eKey] = int(i);
            } else {
                int p = iter->second;
                while (polygonRoots[p] != p) {
                    int temp = polygonRoots[p];
                    polygonRoots[p] = int(i);
                    p = temp;
                }
                polygonRoots[p] = int(i);
            }
        }
    }
    
    for (size_t i = 0; i < polygonRoots.size(); i++) {
        int p = int(i);
        while (polygonRoots[p] != p) {
            p = polygonRoots[p];
        }
        
        int root = p;
        p = int(i);
        
        while (polygonRoots[p] != p) {
            int temp = polygonRoots[p];
            polygonRoots[p] = root;
            p = temp;
        }
    }
    
    int cCount = 0;
    std::unordered_map<int, int> vMap;
    for (int i = 0; i < int(polygonRoots.size()); i++) {
        if (polygonRoots[i] == i) {
            vMap[i] = cCount++;
        }
    }
    
    components.resize(cCount);
    for (int i = 0; i < int(polygonRoots.size()); i++) {
        components[vMap[polygonRoots[i]]].push_back(i);
    }
}

template <typename ... VertexData>
int _SurfaceTrimmerExecute(const char *In,
                           const char *Out,
                           SurfaceTrimmerParameters params,
                           std::function<bool (float)> progressHandler)
{
    typedef MultiPointStreamData<float, PointStreamValue<float>, PointStreamNormal<float, 3>, PointStreamColor<float>> PLYCheckingVertexData;
    typedef PlyVertexWithData<float, 3, PLYCheckingVertexData> PLYCheckingVertex;
    bool readFlags[PLYCheckingVertex::PlyReadNum];
    
    if (!PlyReadHeader((char *)In, PLYCheckingVertex::PlyReadProperties(), PLYCheckingVertex::PlyReadNum, readFlags)) {
        std::cerr << "Failed to read ply header: " << In << std::endl;
        return -1;
    }
    
    if (!PLYCheckingVertexData::ValidPlyReadProperties<0>(readFlags + 3)) {
        std::cerr << "Ply file does not contain values";
        return -1;
    }
    
    if (progressHandler(0) == false) { return -1; }
    
    MessageWriter messageWriter;
    messageWriter.echoSTDOUT = false;
    
    typedef PlyVertexWithData<float, 3, MultiPointStreamData<float, PointStreamValue<float>, VertexData ...>> Vertex;
    float min, max;
    std::vector<Vertex> vertices;
    std::vector<std::vector<int>> polygons;
    
    int ft;
    std::vector<std::string> comments;
    PlyReadPolygons<Vertex>(In, vertices, polygons, Vertex::PlyReadProperties(), Vertex::PlyReadNum, ft, comments);
    
    if (progressHandler(0.1) == false) { return -1; }
    
    for (int i = 0; i < params.Smooth; i++) {
        SmoothValues(vertices, polygons);
    }
    
    if (progressHandler(0.2) == false) { return -1; }
    
    min = max = std::get<0>(vertices[0].data.data).data;
    for (size_t i = 0; i < vertices.size(); i++) {
        min = std::min<float>(min, std::get<0>(vertices[i].data.data).data), max = std::max<float>(max, std::get<0>(vertices[i].data.data).data);
    }
    
    std::unordered_map<long long, int> vertexTable;
    std::vector<std::vector<int>> ltPolygons, gtPolygons;
    std::vector<bool> ltFlags, gtFlags;
    
    messageWriter(comments, "*********************************************\n");
    messageWriter(comments, "** Running Surface Trimmer **\n");
    messageWriter(comments, "*********************************************\n");
    
    for (size_t i = 0; i < polygons.size(); i++) {
        SplitPolygon(polygons[i], vertices, &ltPolygons, &gtPolygons, &ltFlags, &gtFlags, vertexTable, params.Trim);
    }
    
    if (progressHandler(0.3) == false) { return -1; }
    
    if (params.IslandAreaRatio > 0) {
        std::vector<std::vector<int>> _ltPolygons, _gtPolygons;
        std::vector<std::vector<int>> ltComponents, gtComponents;
        SetConnectedComponents(ltPolygons, ltComponents);
        SetConnectedComponents(gtPolygons, gtComponents);
        std::vector<double> ltAreas(ltComponents.size(), 0.0), gtAreas(gtComponents.size(), 0.0);
        std::vector<bool> ltComponentFlags(ltComponents.size(), false), gtComponentFlags(gtComponents.size(), false);
        double area = 0.0;
        
        for (size_t i = 0; i < ltComponents.size(); i++) {
            for (size_t j = 0; j < ltComponents[i].size(); j++) {
                ltAreas[i] += PolygonArea<Vertex>(vertices, ltPolygons[ltComponents[i][j]]);
                ltComponentFlags[i] = (ltComponentFlags[i] || ltFlags[ltComponents[i][j]]);
            }
            area += ltAreas[i];
        }
        
        for (size_t i = 0; i < gtComponents.size(); i++) {
            for (size_t j = 0; j < gtComponents[i].size(); j++) {
                gtAreas[i] += PolygonArea<Vertex>(vertices, gtPolygons[gtComponents[i][j]]);
                gtComponentFlags[i] = (gtComponentFlags[i] || gtFlags[gtComponents[i][j]]);
            }
            area += gtAreas[i];
        }
        
        for (size_t i = 0; i < ltComponents.size(); i++) {
            if (ltAreas[i] < area * params.IslandAreaRatio && ltComponentFlags[i]) {
                for (size_t j = 0; j < ltComponents[i].size(); j++) {
                    _gtPolygons.push_back(ltPolygons[ltComponents[i][j]]);
                }
            }
            else {
                for (size_t j = 0; j < ltComponents[i].size(); j++) {
                    _ltPolygons.push_back(ltPolygons[ltComponents[i][j]]);
                }
            }
        }
        
        for (size_t i = 0; i < gtComponents.size(); i++) {
            if (gtAreas[i] < area * params.IslandAreaRatio && gtComponentFlags[i]) {
                for (size_t j = 0; j < gtComponents[i].size(); j++) {
                    _ltPolygons.push_back(gtPolygons[gtComponents[i][j]]);
                }
            }
            else {
                for (size_t j = 0; j < gtComponents[i].size(); j++) {
                    _gtPolygons.push_back(gtPolygons[gtComponents[i][j]]);
                }
            }
        }
        
        ltPolygons = _ltPolygons;
        gtPolygons = _gtPolygons;
    }
    
    if (progressHandler(0.6) == false) { return -1; }
    
    if (!params.PolygonMesh) {
        {
            std::vector<std::vector<int>> polys = ltPolygons;
            Triangulate<Vertex>(vertices, ltPolygons, polys);
            ltPolygons = polys;
        }
        {
            std::vector<std::vector<int>> polys = gtPolygons;
            Triangulate<Vertex>(vertices, gtPolygons, polys);
            gtPolygons = polys;
        }
    }
    
    if (progressHandler(0.8) == false) { return -1; }
    
    RemoveHangingVertices(vertices, gtPolygons);
    
    if (progressHandler(0.9) == false) { return -1; }
    
    if (!PlyWritePolygons<Vertex>(Out, vertices, gtPolygons, Vertex::PlyWriteProperties(), Vertex::PlyWriteNum, ft, comments)) {
        ERROR_OUT("Could not write mesh to: %s", Out);
    }
    
    progressHandler(1);
    
    return EXIT_SUCCESS;
}

#endif /* SurfaceTrimmerExecute_hpp */
