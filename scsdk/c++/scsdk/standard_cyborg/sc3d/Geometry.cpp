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

#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/util/DataUtils.hpp"
#include "standard_cyborg/util/nanort.h"

#include <nanoflann.hpp>

#define _USE_MATH_DEFINES
#include <cmath>
#include <cassert>
#include <stack>
#include <iostream>

using standard_cyborg::math::Vec2;
using standard_cyborg::math::Vec3;

namespace standard_cyborg {
namespace sc3d {



static int serialIdCounter;
std::set<int> Geometry::_allocatedIds;

template <class VectorOfVectorsType, int DIM = -1, class Distance = nanoflann::metric_L2, typename IndexType = size_t>
struct KDTreeVectorOfVectorsAdaptor {
    typedef KDTreeVectorOfVectorsAdaptor<VectorOfVectorsType, DIM, Distance> self_t;
    typedef typename Distance::template traits<float, self_t>::distance_t metric_t;
    typedef nanoflann::KDTreeSingleIndexAdaptor<metric_t, self_t, DIM, IndexType> index_t;

    index_t* index; //! The kd-tree index for the user to call its methods as usual with any other FLANN index.

    /// Constructor: takes a const ref to the vector of vectors object with the data points
    KDTreeVectorOfVectorsAdaptor(const VectorOfVectorsType& mat, const int leaf_max_size = 10) :
        m_data(mat)
    {
        const size_t dims = 3;
        index = new index_t(static_cast<int>(dims), *this /* adaptor */, nanoflann::KDTreeSingleIndexAdaptorParams(leaf_max_size, nanoflann::KDTreeSingleIndexAdaptorFlags::None, 0));
        index->buildIndex();
    }

    ~KDTreeVectorOfVectorsAdaptor()
    {
        delete index;
    }

    const VectorOfVectorsType& m_data;

    /** Query for the \a num_closest closest points to a given point (entered as query_point[0:dim-1]).
     *  Note that this is a short-cut method for index->findNeighbors().
     *  The user can also call index->... methods as desired.
     * \note nChecks_IGNORED is ignored but kept for compatibility with the original FLANN interface.
     */
    inline void query(const float* query_point, const size_t num_closest, IndexType* out_indices, float* out_distances_sq, const int nChecks_IGNORED = 10) const
    {
        nanoflann::KNNResultSet<float, IndexType> resultSet(num_closest);
        resultSet.init(out_indices, out_distances_sq);
        index->findNeighbors(resultSet, query_point);
    }

    /** @name Interface expected by KDTreeSingleIndexAdaptor
     * @{ */

    const self_t& derived() const { return *this; }
    self_t& derived() { return *this; }

    // Must return the number of data points
    inline size_t kdtree_get_point_count() const
    {
        return m_data.vertexCount();
    }

    // Returns the dim'th component of the idx'th point in the class:
    inline float kdtree_get_pt(const size_t idx, const size_t dim) const
    {
        Vec3 v = m_data.getPositions()[idx];

        // cast to float array.
        float* floatArray = (float*)&v;

        return floatArray[dim];
    }

    // Optional bounding-box computation: return false to default to a standard bbox computation loop.
    //   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
    //   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
    template <class BBOX>
    bool kdtree_get_bbox(BBOX& /*bb*/) const { return false; }

    /** @} */

}; // end of KDTreeVectorOfVectorsAdaptor


typedef KDTreeVectorOfVectorsAdaptor<Geometry> KdTree;

struct Geometry::Impl {
public:
    std::unique_ptr<KdTree> _kdTree;

    std::unique_ptr<nanort::BVHAccel<float>> _rtAccel;

    ~Impl() = default;
};

Geometry::Geometry(const std::vector<Vec3>& positions,
                   const std::vector<Vec3>& normals,
                   const std::vector<Vec3>& colors,
                   const std::vector<Face3>& faces) :
    Geometry()
{
    pImpl = std::unique_ptr<Impl>(new Impl());

    // NB: This copies data!
    _positions = positions;
    _normals = normals;
    _colors = colors;
    _faces = faces;
}

Geometry::Geometry(Geometry const& other) {
    //printf("create new geo by copy constructor\n");
    this->copy(other);

    _id = serialIdCounter++;
    _allocatedIds.insert(_id);
}

bool operator==(const Geometry& lhs, const Geometry& rhs) {
    bool b =  
        lhs.getPositions() == rhs.getPositions() && 
        lhs.getNormals() == rhs.getNormals() &&
        lhs.getColors() == rhs.getColors() &&
        lhs.getTexCoords() == rhs.getTexCoords() &&
        lhs.getFaces() == rhs.getFaces() &&
        lhs.normalsEncodeSurfelRadius() == rhs.normalsEncodeSurfelRadius() && 
        lhs.hasTexture() == rhs.hasTexture();

    if(lhs.hasTexture()) {
        b = b && lhs.getTexture() == rhs.getTexture();
    }
    
    return b;
}

Geometry::Geometry(const std::vector<Vec3>& positions,
                   const std::vector<Face3>& faces) :
    Geometry()
{
    pImpl = std::unique_ptr<Impl>(new Impl());

    // NB: This copies data!    
    _positions = positions;
    _faces = faces;
}

Geometry::Geometry()
{
    //printf("create new geo\n");
    pImpl = std::unique_ptr<Impl>(new Impl());

    _id = serialIdCounter++;
    _allocatedIds.insert(_id);
}

void Geometry::resetIdCounter() { serialIdCounter = 0; }

std::set<int> Geometry::getAllocatedIds() { return _allocatedIds; }

int Geometry::getNumAllocatedIds()
{
    return (int)_allocatedIds.size();
}

void Geometry::resetAllocatedIds()
{
    _allocatedIds.clear();
}

Geometry::~Geometry()
{

    _allocatedIds.erase(_id);

    //printf("delete geo by destructor %d\n",_allocatedIds.size() );

}

// clang-format off
const std::vector<Vec3>&  Geometry::getPositions() const { return _positions; }
const std::vector<Vec3>&  Geometry::getNormals()   const { return _normals; }
const std::vector<Vec3>&  Geometry::getColors()    const { return _colors; }
const std::vector<Vec2>&  Geometry::getTexCoords() const { return _texCoords; }
const std::vector<Face3>& Geometry::getFaces()     const { return _faces; }

bool Geometry::hasPositions() const { return _positions.size() > 0; }
bool Geometry::hasNormals()   const { return _normals.size()   > 0; }
bool Geometry::hasColors()    const { return _colors.size()    > 0; }
bool Geometry::hasTexCoords() const { return _texCoords.size() > 0; }
bool Geometry::hasFaces()     const { return _faces.size()     > 0; }
// clang-format on

void Geometry::setColor(const Vec3& color, float alpha)
{
    int numVertices = vertexCount();
    for (int i = 0; i < numVertices; i++) {
        _colors[i] = alpha * color + (1.0 - alpha) * _colors[i];
    }
}

void Geometry::setColor(const Vec3& color, float alpha, const VertexSelection& vertexIndices)
{
    for (auto index : vertexIndices) {
        _colors[index] = alpha * color + (1.0 - alpha) * _colors[index];
    }
}

bool Geometry::setVertexData(const std::vector<Vec3>& positions,
                             const std::vector<Vec3>& normals,
                             const std::vector<Vec3>& colors)
{
    _isDirty = true;

    if (normals.size() != 0 && normals.size() != positions.size()) {
        return false;
    }

    if (colors.size() != 0 && colors.size() != positions.size()) {
        return false;
    }

    _positions = positions;
    _normals = normals;
    _colors = colors;

    return true;
}

void Geometry::didMutateExternally() const
{
    _isDirty = true;
}

bool Geometry::setPositions(const std::vector<Vec3>& positions)
{
    _isDirty = true;
    // If positions are empty, unset entirely
    if (positions.size() == 0) {
        _positions = positions;
        return true;
    }

    // Assert consistency with normals and colors
    if (_normals.size() != 0 && _normals.size() != positions.size()) {
        return false;
    }
    if (_colors.size() != 0 && _colors.size() != positions.size()) {
        return false;
    }
    if (_texCoords.size() != 0 && _texCoords.size() != positions.size()) {
        return false;
    }

    _positions = positions;

    return true;
}

bool Geometry::setNormals(const std::vector<Vec3>& normals)
{
    // If normals are empty, unset entirely
    if (normals.size() == 0) {
        _normals = normals;
        return true;
    }

    // Assert consistency with positions and colors
    if (_positions.size() != 0 && _positions.size() != normals.size()) {
        return false;
    }
    if (_colors.size() != 0 && _colors.size() != normals.size()) {
        return false;
    }
    if (_texCoords.size() != 0 && _texCoords.size() != normals.size()) {
        return false;
    }

    _normals = normals;

    return true;
}

bool Geometry::setColors(const std::vector<Vec3>& colors)
{
    // If colors are empty, unset entirely
    if (colors.size() == 0) {
        _colors = colors;
        return true;
    }

    // Assert consistency with normals and positions
    if (_normals.size() != 0 && _normals.size() != colors.size()) {
        return false;
    }

    if (_positions.size() != 0 && _positions.size() != colors.size()) {
        return false;
    }
    if (_texCoords.size() != 0 && _texCoords.size() != colors.size()) {
        return false;
    }
    _colors = colors;

    return true;
}

bool Geometry::setTexCoords(const std::vector<Vec2>& texCoords)
{
    // If colors are empty, unset entirely
    if (texCoords.size() == 0) {
        _texCoords = texCoords;
        return true;
    }

    // Assert consistency with normals and positions
    if (_normals.size() != 0 && _normals.size() != texCoords.size()) {
        return false;
    }

    if (_positions.size() != 0 && _positions.size() != texCoords.size()) {
        return false;
    }
    if (_colors.size() != 0 && _colors.size() != texCoords.size()) {
        return false;
    }
    _texCoords = texCoords;

    return true;
}

bool Geometry::setFaces(const std::vector<Face3>& faces)
{
    _isDirty = true;
    // We may want to iterate through the face data and ensure there are
    // no out-of-bounds indices.
    _faces = faces;

    return true;
}

bool Geometry::setTexture(const ColorImage& texture)
{
    _texture.reset(new ColorImage());
    _texture->copy(texture);

    return true;
}

bool Geometry::hasTexture() const
{
    return _texture != nullptr;
}

const ColorImage& Geometry::getTexture() const
{
    SCASSERT(hasTexture(), "geometry doesnt have a texture.");
    return *_texture;
}

#ifdef EMBIND_ONLY
ColorImage* Geometry::getTexturePtr()
{
    return _texture.get();
}
#endif

int Geometry::vertexCount() const
{
    return (int)_positions.size();
}

int Geometry::faceCount() const
{
    return (int)_faces.size();
}

void Geometry::deleteVertices(const VertexSelection& verticesToDelete)
{
    _isDirty = true;

    if (hasFaces()) {
        std::set<int> faceIndicesToDelete;

        for (int faceIndex = 0; faceIndex < _faces.size(); ++faceIndex) {
            Face3 face = _faces[faceIndex];
            // Delete if any vertex that the face touches is in the selection to delete
            if (verticesToDelete.contains(face[0]) || verticesToDelete.contains(face[1]) || verticesToDelete.contains(face[2])) {
                faceIndicesToDelete.insert(faceIndex);
            }
        }

        // Renumber the faces in-place by keeping a running counter of the compressed index.
        int faceRenumbering = -1;
        for (int originalFaceIndex = 0; originalFaceIndex < _faces.size(); ++originalFaceIndex) {
            if (faceIndicesToDelete.count(originalFaceIndex) == 1) continue;
            faceRenumbering++;
            _faces[faceRenumbering] = _faces[originalFaceIndex];
        }
        _faces.resize(faceRenumbering + 1);

        // Construct a temporary array of vertex renumberings since we don't know anything a priori
        // about the order in which they'll show up in the faces
        std::vector<int> vertexRenumbering(vertexCount());
        int cumulativeCount = -1;
        for (int vertexIndex = 0; vertexIndex < vertexCount(); vertexIndex++) {
            cumulativeCount += verticesToDelete.contains(vertexIndex) ? 0 : 1;
            vertexRenumbering[vertexIndex] = cumulativeCount;
        }

        // Finally, step through the compressed faces and update their vertex indices
        for (int faceIndex = 0; faceIndex < _faces.size(); faceIndex++) {
            Face3& face = _faces[faceIndex];
            face[0] = vertexRenumbering[face[0]];
            face[1] = vertexRenumbering[face[1]];
            face[2] = vertexRenumbering[face[2]];
        }
    }

    if (hasPositions()) {
        deleteEntriesFromVector(_positions, verticesToDelete);
    }

    if (hasNormals()) {
        deleteEntriesFromVector(_normals, verticesToDelete);
    }

    if (hasColors()) {
        deleteEntriesFromVector(_colors, verticesToDelete);
    }

    if (hasTexCoords()) {
        deleteEntriesFromVector(_texCoords, verticesToDelete);
    }
}

int Geometry::getClosestVertexIndex(const Vec3& queryPoint) const
{
    updateDataStructures();

    size_t retIndex;
    float retDistSquared;

    nanoflann::KNNResultSet<float> resultSet(1);

    float pt[3] = {queryPoint.x, queryPoint.y, queryPoint.z};

    resultSet.init(&retIndex, &retDistSquared);
    pImpl->_kdTree->index->findNeighbors(resultSet, pt);

    return (int)retIndex;
}

Vec3 Geometry::getClosestVertexPosition(const Vec3& queryPoint) const
{
    int index = getClosestVertexIndex(queryPoint);

    return _positions[index];
}

std::vector<int> Geometry::getNClosestVertexIndices(const Vec3& queryPosition, int n) const
{
    std::vector<int> results;

    updateDataStructures();

    {
        std::vector<float> retDistsSquared;

        for (int ii = 0; ii < n; ++ii) {
            results.push_back(0);
            retDistsSquared.push_back(0);
        }

        nanoflann::KNNResultSet<float, int> resultSet(n);

        float pt[3] = {queryPosition.x, queryPosition.y, queryPosition.z};

        resultSet.init(results.data(), retDistsSquared.data());
        pImpl->_kdTree->index->findNeighbors(resultSet, pt, nanoflann::SearchParameters(10));
    }

    return results;
}

std::vector<int> Geometry::getVertexIndicesInRadius(const Vec3& queryPosition, float radius) const
{
    updateDataStructures();

    float position[3] = {queryPosition.x, queryPosition.y, queryPosition.z};
    float squaredRadius = radius * radius;
    std::vector<nanoflann::ResultItem<unsigned long, float>> resultIndicesAndSquaredDistances;
    nanoflann::SearchParameters params;
    params.sorted = false;

    pImpl->_kdTree->index->radiusSearch(&position[0], squaredRadius, resultIndicesAndSquaredDistances, params);

    std::vector<int> results;
    for (auto result : resultIndicesAndSquaredDistances) {
        results.push_back((int)result.first);
    }

    return results;
}



RayTraceResult Geometry::rayTrace(Vec3 rayOrigin, Vec3 rayDirection, float rayMin, float rayMax) const
{
    if (_faces.size() == 0) {
        // for point clouds, we at the moment dont support ray tracing. So, always return -1.
        RayTraceResult result;
        result.t = 0.0f;
        result.index = -1;

        return result;
    }

    updateDataStructures();

    nanort::Ray<float> ray;
    ray.min_t = rayMin;
    ray.max_t = rayMax;

    ray.dir[0] = rayDirection.x;
    ray.dir[1] = rayDirection.y;
    ray.dir[2] = rayDirection.z;
    ray.org[0] = rayOrigin.x;
    ray.org[1] = rayOrigin.y;
    ray.org[2] = rayOrigin.z;

    nanort::TriangleIntersection<> intersection;

    nanort::TriangleIntersector<> triangle_intersector((float*)this->_positions.data(), (unsigned int*)_faces.data(), sizeof(Vec3));

    bool hit = pImpl->_rtAccel->Traverse(ray, triangle_intersector, &intersection);

    RayTraceResult result;
    if (hit) {
        result.t = intersection.t;
        result.index = (int)intersection.prim_id;
        result.hitPoint = rayOrigin + result.t * rayDirection;
    }

    return result;
}


void Geometry::updateDataStructures() const
{
    if ((!pImpl->_rtAccel || _isDirty) && hasFaces()) {
        nanort::BVHBuildOptions<float> options; // Use default option
        nanort::TriangleMesh<float> triangle_mesh((float*)this->_positions.data(), (unsigned int*)_faces.data(), sizeof(Vec3));
        nanort::TriangleSAHPred<float> triangle_pred((float*)this->_positions.data(), (unsigned int*)_faces.data(), sizeof(Vec3));

        pImpl->_rtAccel.reset(new nanort::BVHAccel<float>());

        bool ret = pImpl->_rtAccel->Build((int)_faces.size(), triangle_mesh, triangle_pred, options);
        assert(ret);
    }

    if (!pImpl->_kdTree || _isDirty) { // lazily build kd tree.
        pImpl->_kdTree.reset(new KdTree(*this, 10));
        pImpl->_kdTree->index->buildIndex();
    }

    _isDirty = false;
}

void Geometry::copy(const Geometry& that)
{
    _positions = that.getPositions();
    _normals = that.getNormals();
    _colors = that.getColors();
    _texCoords = that.getTexCoords();
    _faces = that.getFaces();
    
    this->_frame = that._frame;

    if (that.hasTexture()) {
        _texture = that._texture;
    }
}

void Geometry::transform(const math::Mat3x4& mat)
{
    _isDirty = true;

    const math::Mat3x4& m = mat;

    for (int ii = 0; ii < _positions.size(); ++ii) {
        Vec3 p = _positions[ii];

        // clang-format off
        _positions[ii] =
        Vec3(p.x * m.m00 + p.y * m.m01 + p.z * m.m02 + 1.0 * m.m03,
             p.x * m.m10 + p.y * m.m11 + p.z * m.m12 + 1.0 * m.m13,
             p.x * m.m20 + p.y * m.m21 + p.z * m.m22 + 1.0 * m.m23);
        
        if (_normals.size() != 0) {
            Vec3 n = _normals[ii];
            
            _normals[ii] =
            Vec3(n.x * m.m00 + n.y * m.m01 + n.z * m.m02,
                 n.x * m.m10 + n.y * m.m11 + n.z * m.m12,
                 n.x * m.m20 + n.y * m.m21 + n.z * m.m22);
        }
        // clang-format on
    }
}

void Geometry::normalizeNormals()
{
    int numVertices = vertexCount();
    for (int i = 0; i < numVertices; i++) {
        Vec3 normal = _normals[i];
        float l = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
        _normals[i].x /= l;
        _normals[i].y /= l;
        _normals[i].z /= l;
    }
}

void Geometry::mutatePositionsWithFunction(const std::function<Vec3(int index, Vec3 position, Vec3 normal, Vec3 color)>& mapFn)
{
    _isDirty = true;

    int numVertices = vertexCount();
    for (int index = 0; index < numVertices; index++) {
        _positions[index] = mapFn(index, _positions[index], _normals[index], _colors[index]);
    }
}

void Geometry::mutateNormalsWithFunction(const std::function<Vec3(int index, Vec3 position, Vec3 normal, Vec3 color)>& mapFn)
{
    int numVertices = vertexCount();
    for (int index = 0; index < numVertices; index++) {
        _normals[index] = mapFn(index, _positions[index], _normals[index], _colors[index]);
    }
}

void Geometry::mutateColorsWithFunction(const std::function<Vec3(int index, Vec3 position, Vec3 normal, Vec3 color)>& mapFn)
{
    int numVertices = vertexCount();
    for (int index = 0; index < numVertices; index++) {
        _colors[index] = mapFn(index, _positions[index], _normals[index], _colors[index]);
    }
}

void Geometry::mutatePositionsWithFunction(const std::function<Vec3(int index, Vec3 position, Vec3 normal, Vec3 color)>& mapFn, const VertexSelection& vertexIndices)
{
    _isDirty = true;

    for (auto index : vertexIndices) {
        _positions[index] = mapFn(index, _positions[index], _normals[index], _colors[index]);
    }
}

void Geometry::mutateNormalsWithFunction(const std::function<Vec3(int index, Vec3 position, Vec3 normal, Vec3 color)>& mapFn, const VertexSelection& vertexIndices)
{
    for (auto index : vertexIndices) {
        _normals[index] = mapFn(index, _positions[index], _normals[index], _colors[index]);
    }
}

void Geometry::mutateColorsWithFunction(const std::function<Vec3(int index, Vec3 position, Vec3 normal, Vec3 color)>& mapFn, const VertexSelection& vertexIndices)
{
    for (auto index : vertexIndices) {
        _colors[index] = mapFn(index, _positions[index], _normals[index], _colors[index]);
    }
}

Vec3 Geometry::getFaceCenter(int faceIndex) const
{
    if (faceIndex >= _faces.size()) return Vec3(NAN);

    Face3 face = _faces[faceIndex];
    return (_positions[face[0]] + _positions[face[1]] + _positions[face[2]]) * (1.0 / 3.0);
}

int Geometry::getSize()
{
    int totalSize = 0;

    totalSize += _positions.size() == 0 ? 0 : sizeof(_positions[0]) * _positions.size();
    totalSize += _normals.size() == 0 ? 0 : sizeof(_normals[0]) * _normals.size();
    totalSize += _colors.size() == 0 ? 0 : sizeof(_colors[0]) * _colors.size();
    totalSize += _texCoords.size() == 0 ? 0 : sizeof(_texCoords[0]) * _texCoords.size();
    totalSize += _faces.size() == 0 ? 0 : sizeof(_faces[0]) * _faces.size();

    updateDataStructures(); // the data structure are lazy initialized, so initialize them explicitly if necessary.

    if (hasFaces()) {
        const std::vector<nanort::BVHNode<float>>& nodes = pImpl->_rtAccel->GetNodes();
        totalSize += nodes.size() == 0 ? 0 : sizeof(nodes[0]) * nodes.size();

        const std::vector<unsigned int>& indices = pImpl->_rtAccel->GetIndices();
        totalSize += indices.size() == 0 ? 0 : sizeof(indices[0]) * indices.size();
    }

    totalSize += pImpl->_kdTree->index->usedMemory(*(pImpl->_kdTree->index));

    return totalSize;
}

bool Geometry::normalsEncodeSurfelRadius() const
{
    return _normalsEncodeSurfelRadius;
}

void Geometry::setNormalsEncodeSurfelRadius(bool normalsEncodeSurfelRadius)
{
    _normalsEncodeSurfelRadius = normalsEncodeSurfelRadius;
}


} // namespace sc3d
} // namespace standard_cyborg
