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


#pragma once

#include <vector>

#include "standard_cyborg/sc3d/Face3.hpp"
#include "standard_cyborg/math/Mat3x4.hpp"
#include "standard_cyborg/math/Vec3.hpp"
#include "standard_cyborg/sc3d/VertexSelection.hpp"
#include "standard_cyborg/sc3d/ColorImage.hpp"

namespace standard_cyborg {
namespace sc3d {

struct RayTraceResult {
    float t = INFINITY;
    int index = -1; // index of hit triangle. -1 if no triangle was hit.
    math::Vec3 hitPoint;
};

class Geometry {
public:
    // A comment noting that at one point the function on the line below constructed from std::vector of Vec3
    Geometry(const std::vector<math::Vec3>& positions,
             const std::vector<math::Vec3>& normals = std::vector<math::Vec3>(),
             const std::vector<math::Vec3>& colors = std::vector<math::Vec3>(),
             const std::vector<Face3>& faces = std::vector<Face3>()
    );
    
    Geometry(const std::vector<math::Vec3>& positions, const std::vector<Face3>& faces);
    
    Geometry();
    Geometry(Geometry const& other);

    Geometry(Geometry&&) = delete;
    Geometry& operator=(Geometry&&) = delete;
    Geometry& operator=(Geometry const& other) = delete;
    
    /* Construct Geometry with points and triangle faces. */
    //Geometry(math::Vec3* positions, math::Vec3* normals, math::Vec3* colors, int numVertices, int* faces, int numFaces);
    
    /* Construct Geometry with only points; a point cloud. */
    //Geometry(math::Vec3* positions, math::Vec3* normals, math::Vec3* colors, int numVertices);
    
    ~Geometry();
    
    /* Makes this Geometry into a deep copy of 'that'*/
    void copy(const Geometry& that);
    
    /* Return the index of the vertex that is closest to queryPosition, in terms of the Euclidean distance. */
    int getClosestVertexIndex(const math::Vec3& queryPosition) const;
    
    /* Return the vertex position that is closest to queryPosition, in terms of the Euclidean distance. */
    math::Vec3 getClosestVertexPosition(const math::Vec3& queryPosition) const;
    
    /* Return the indices of the `n` vertices that are closest to queryPosition, in terms of the Euclidean distance. */
    std::vector<int> getNClosestVertexIndices(const math::Vec3& queryPosition, int n) const;
    
    /* Return the indices of all vertices that are within the radius of queryPosition, in terms of the Euclidean distance. Unsorted. */
    std::vector<int> getVertexIndicesInRadius(const math::Vec3& queryPosition, float radius) const;
    
    RayTraceResult rayTrace(math::Vec3 rayOrigin, math::Vec3 rayDirection, float rayMin = 0.001f, float rayMax = 1.0e+30f) const;
    
    void deleteVertices(const VertexSelection& vertexIndices);
    
    void transform(const math::Mat3x4& mat);
    
    const std::vector<math::Vec3>& getPositions() const;
    const std::vector<math::Vec3>& getNormals() const;
    const std::vector<math::Vec3>& getColors() const;
    const std::vector<math::Vec2>& getTexCoords() const;
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
    
    bool setVertexData(const std::vector<math::Vec3>& positions,
                       const std::vector<math::Vec3>& normals = std::vector<math::Vec3>(),
                       const std::vector<math::Vec3>& colors = std::vector<math::Vec3>());
    
    bool setPositions(const std::vector<math::Vec3>& positions);
    bool setNormals(const std::vector<math::Vec3>& normals);
    bool setColors(const std::vector<math::Vec3>& colors);
    bool setTexCoords(const std::vector<math::Vec2>& texCoords);
    bool setFaces(const std::vector<Face3>& faces);
    bool setTexture(const ColorImage& texture);
    
    void setColor(const math::Vec3& color, float alpha);
    void setColor(const math::Vec3& color, float alpha, const VertexSelection& vertexIndices);
    
    void mutatePositionsWithFunction(const std::function<math::Vec3(int index, math::Vec3 position, math::Vec3 normal, math::Vec3 color)>& mapFn);
    void mutateNormalsWithFunction(const std::function<math::Vec3(int index, math::Vec3 position, math::Vec3 normal, math::Vec3 color)>& mapFn);
    void mutateColorsWithFunction(const std::function<math::Vec3(int index, math::Vec3 position, math::Vec3 normal, math::Vec3 color)>& mapFn);
    
    void mutatePositionsWithFunction(const std::function<math::Vec3(int index, math::Vec3 position, math::Vec3 normal, math::Vec3 color)>& mapFn, const VertexSelection& vertexIndices);
    void mutateNormalsWithFunction(const std::function<math::Vec3(int index, math::Vec3 position, math::Vec3 normal, math::Vec3 color)>& mapFn, const VertexSelection& vertexIndices);
    void mutateColorsWithFunction(const std::function<math::Vec3(int index, math::Vec3 position, math::Vec3 normal, math::Vec3 color)>& mapFn, const VertexSelection& vertexIndices);
    
    math::Vec3 getFaceCenter(int faceIndex) const;
    
    int getSize();
    
    static std::set<int> getAllocatedIds();
    static int getNumAllocatedIds();
    static void resetAllocatedIds();
    static void resetIdCounter();
    
    void didMutateExternally() const;
    
    std::string getFrame() const { return _frame; }
    void setFrame(const std::string &f) { _frame = f; }
    
private:
    void updateDataStructures() const;
    
    std::string _frame;
    
    // These variables are marked mutable since pImpl is strictly a cache of the acceleration
    // data structure that does not affect the externally visible state of this object. In specific
    // cases like JS-interop where it's conceivable that data may be mutated directly outside of
    // C++ const-correctness, you may call `didMutateExternally()` to explicitly trigger updates.
    struct Impl;
    mutable std::unique_ptr<Impl> pImpl;
    mutable bool _isDirty = true;
    
    std::vector<math::Vec3> _positions;
    std::vector<math::Vec3> _normals;
    std::vector<math::Vec3> _colors;
    std::vector<math::Vec2> _texCoords;
    std::vector<Face3> _faces;
    
    std::shared_ptr<ColorImage> _texture;
    
    static std::set<int> _allocatedIds;
    
    int _id;
    bool _normalsEncodeSurfelRadius = false;
};

bool operator==(const Geometry& lhs, const Geometry& rhs);

} // namespace sc3d
} // namespace standard_cyborg

