//
//  Geometry.hpp
//  StandardCyborgFusion
//
//  Created by Eric Arneback on 2019-03-20.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <vector>

#include <StandardCyborgData/Face3.hpp>
#include <StandardCyborgData/Mat3x4.hpp>
#include <StandardCyborgData/Vec3.hpp>
#include <StandardCyborgData/VertexSelection.hpp>
#include <StandardCyborgData/ColorImage.hpp>

#include <StandardCyborgData/Pybind11Defs.hpp>

namespace StandardCyborg {

struct RayTraceResult {
    float t = INFINITY;
    int index = -1; // index of hit triangle. -1 if no triangle was hit.
    Vec3 hitPoint;
};

class Geometry {
public:
    // A comment noting that at one point the function on the line below constructed from std::vector of Vec3
    Geometry(const std::vector<Vec3>& positions,
             const std::vector<Vec3>& normals = std::vector<Vec3>(),
             const std::vector<Vec3>& colors = std::vector<Vec3>(),
             const std::vector<Face3>& faces = std::vector<Face3>()
    );
    
    Geometry(const std::vector<Vec3>& positions, const std::vector<Face3>& faces);
    
    Geometry();
    
    Geometry(Geometry&&) = delete;
    Geometry& operator=(Geometry&&) = delete;
    Geometry(Geometry const& other) = delete;
    Geometry& operator=(Geometry const& other) = delete;
    
    /* Construct Geometry with points and triangle faces. */
    //Geometry(Vec3* positions, Vec3* normals, Vec3* colors, int numVertices, int* faces, int numFaces);
    
    /* Construct Geometry with only points; a point cloud. */
    //Geometry(Vec3* positions, Vec3* normals, Vec3* colors, int numVertices);
    
    ~Geometry();
    
    /* Makes this Geometry into a deep copy of 'that'*/
    void copy(const Geometry& that);
    
    /* Return the index of the vertex that is closest to queryPosition, in terms of the Euclidean distance. */
    int getClosestVertexIndex(const Vec3& queryPosition) const;
    
    /* Return the vertex position that is closest to queryPosition, in terms of the Euclidean distance. */
    Vec3 getClosestVertexPosition(const Vec3& queryPosition) const;
    
    /* Return the indices of the `n` vertices that are closest to queryPosition, in terms of the Euclidean distance. */
    std::vector<int> getNClosestVertexIndices(const Vec3& queryPosition, int n) const;
    
    /* Return the indices of all vertices that are within the radius of queryPosition, in terms of the Euclidean distance. Unsorted. */
    std::vector<int> getVertexIndicesInRadius(const Vec3& queryPosition, float radius) const;
    
    RayTraceResult rayTrace(Vec3 rayOrigin, Vec3 rayDirection, float rayMin = 0.001f, float rayMax = 1.0e+30f) const;
    
    void deleteVertices(const VertexSelection& vertexIndices);
    
    void transform(const Mat3x4& mat);
    
    const std::vector<Vec3>& getPositions() const;
    const std::vector<Vec3>& getNormals() const;
    const std::vector<Vec3>& getColors() const;
    const std::vector<Vec2>& getTexCoords() const;
    const std::vector<Face3>& getFaces() const;
    const ColorImage& getTexture() const;
    
    #ifdef EMBIND_ONLY
        ColorImage* getTexturePtr();
    #endif // EMBIND_ONLY
    
    bool hasPositions() const;
    bool hasNormals() const;
    bool hasColors() const;
    bool hasTexCoords() const;
    bool hasFaces() const;
    bool hasTexture() const;
    
    bool normalsEncodeSurfelRadius() const;
    void setNormalsEncodeSurfelRadius(bool normalsEncodeSurfelRadius);
    
    void normalizeNormals();
    
    int vertexCount() const;
    int faceCount() const;
    
    bool setVertexData(const std::vector<Vec3>& positions,
                       const std::vector<Vec3>& normals = std::vector<Vec3>(),
                       const std::vector<Vec3>& colors = std::vector<Vec3>());
    
    bool setPositions(const std::vector<Vec3>& positions);
    bool setNormals(const std::vector<Vec3>& normals);
    bool setColors(const std::vector<Vec3>& colors);
    bool setTexCoords(const std::vector<Vec2>& texCoords);
    bool setFaces(const std::vector<Face3>& faces);
    bool setTexture(const ColorImage& texture);
    
    void setColor(const Vec3& color, float alpha);
    void setColor(const Vec3& color, float alpha, const VertexSelection& vertexIndices);
    
    void mutatePositionsWithFunction(const std::function<Vec3(int index, Vec3 position, Vec3 normal, Vec3 color)>& mapFn);
    void mutateNormalsWithFunction(const std::function<Vec3(int index, Vec3 position, Vec3 normal, Vec3 color)>& mapFn);
    void mutateColorsWithFunction(const std::function<Vec3(int index, Vec3 position, Vec3 normal, Vec3 color)>& mapFn);
    
    void mutatePositionsWithFunction(const std::function<Vec3(int index, Vec3 position, Vec3 normal, Vec3 color)>& mapFn, const VertexSelection& vertexIndices);
    void mutateNormalsWithFunction(const std::function<Vec3(int index, Vec3 position, Vec3 normal, Vec3 color)>& mapFn, const VertexSelection& vertexIndices);
    void mutateColorsWithFunction(const std::function<Vec3(int index, Vec3 position, Vec3 normal, Vec3 color)>& mapFn, const VertexSelection& vertexIndices);
    
    Vec3 getFaceCenter(int faceIndex) const;
    
    int getSize();
    
    static std::set<int> getAllocatedIds();
    static int getNumAllocatedIds();
    static void resetAllocatedIds();
    static void resetIdCounter();
    
    void didMutateExternally() const;
    
private:
    void updateDataStructures() const;
    
    // These variables are marked mutable since pImpl is strictly a cache of the acceleration
    // data structure that does not affect the externally visible state of this object. In specific
    // cases like JS-interop where it's conceivable that data may be mutated directly outside of
    // C++ const-correctness, you may call `didMutateExternally()` to explicitly trigger updates.
    struct Impl;
    mutable std::unique_ptr<Impl> pImpl;
    mutable bool _isDirty = true;
    
    std::vector<Vec3> _positions;
    std::vector<Vec3> _normals;
    std::vector<Vec3> _colors;
    std::vector<Vec2> _texCoords;
    std::vector<Face3> _faces;
    
    std::shared_ptr<ColorImage> _texture;
    
    static std::set<int> _allocatedIds;
    
    int _id;
    bool _normalsEncodeSurfelRadius = false;
};


} // namespace StandardCyborg
