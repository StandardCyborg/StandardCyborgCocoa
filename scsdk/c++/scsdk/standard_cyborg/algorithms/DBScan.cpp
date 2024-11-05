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
#include "standard_cyborg/algorithms/DBScan.hpp"
#include "standard_cyborg/math/Vec3.hpp"

#include <algorithm>
#include <random>
#include <nanoflann.hpp>


namespace standard_cyborg {

namespace algorithms {

template <class VectorOfVectorsType, typename num_t = double, int DIM = -1, class Distance = nanoflann::metric_L2, typename IndexType = size_t>
struct KDTreeVec3VectorAdaptor {
    typedef KDTreeVec3VectorAdaptor<VectorOfVectorsType, num_t, DIM, Distance> self_t;
    typedef typename Distance::template traits<num_t, self_t>::distance_t metric_t;
    typedef nanoflann::KDTreeSingleIndexAdaptor<metric_t, self_t, DIM, IndexType> index_t;
    
    index_t* index; //! The kd-tree index for the user to call its methods as usual with any other FLANN index.
    
    /// Constructor: takes a const ref to the vector of vectors object with the data points
    KDTreeVec3VectorAdaptor(const VectorOfVectorsType& mat, const int leaf_max_size = 10) :
        m_data(mat)
    {
        assert(mat.size() != 0);
        const size_t dims = 3;
        index = new index_t(static_cast<int>(dims), *this /* adaptor */, nanoflann::KDTreeSingleIndexAdaptorParams(leaf_max_size, nanoflann::KDTreeSingleIndexAdaptorFlags::None, 0));
        index->buildIndex();
    }
    
    KDTreeVec3VectorAdaptor()
    {
        delete index;
    }
    
    const VectorOfVectorsType& m_data;
    
    /** Query for the \a num_closest closest points to a given point (entered as query_point[0:dim-1]).
     *  Note that this is a short-cut method for index->findNeighbors().
     *  The user can also call index->... methods as desired.
     * \note nChecks_IGNORED is ignored but kept for compatibility with the original FLANN interface.
     */
    inline void query(const num_t* query_point, const size_t num_closest, IndexType* out_indices, num_t* out_distances_sq, const int nChecks_IGNORED = 10) const
    {
        nanoflann::KNNResultSet<num_t, IndexType> resultSet(num_closest);
        resultSet.init(out_indices, out_distances_sq);
    index->findNeighbors(resultSet, query_point, nanoflann::SearchParameters());
    }
    
    /** @name Interface expected by KDTreeSingleIndexAdaptor
     * @{ */
    
    const self_t& derived() const { return *this; }
    self_t& derived() { return *this; }
    
    // Must return the number of data points
    inline size_t kdtree_get_point_count() const
    {
        return m_data.size();
    }
    
    // Returns the dim'th component of the idx'th point in the class:
    inline num_t kdtree_get_pt(const size_t idx, const size_t dim) const
    {
        math::Vec3 v = m_data[idx];
        
        // cast to float array.
        float* floatArray = (float*)&v;
        
        return floatArray[dim];
    }
    
    // Optional bounding-box computation: return false to default to a standard bbox computation loop.
    //   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
    //   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
    template <class BBOX>
    bool kdtree_get_bbox(BBOX& /*bb*/) const
    {
        return false;
    }
    
    /** @} */
    
}; // end of KDTreeVectorOfVectorsAdaptor
    
struct Point {
public:
    math::Vec3 p;
    int ptsCnt = 0, cluster;
    double getDis(const Point& ot)
    {
        return (p - ot.p).norm();
    }
};

static std::vector<std::vector<int>> adjPoints;
static std::vector<Point> points;

static int minPts;

static bool isCoreObject(int idx)
{
    return points[idx].ptsCnt >= minPts;
}

static const int NOISE = -2;
static const int NOT_CLASSIFIED = -1;

static void dfs(int now, int c)
{
    points[now].cluster = c;
    if (!isCoreObject(now)) return;
    
    for (auto& next : adjPoints[now]) {
        if (points[next].cluster != NOT_CLASSIFIED) continue;
        dfs(next, c);
    }
}

std::vector<int> DBScan::compute(const std::vector<math::Vec3>& argpoints, size_t minPoints, float epsilon)
{
    typedef KDTreeVec3VectorAdaptor<std::vector<math::Vec3>, float> KdTree;
    
    minPts = (int)minPoints;
    
    adjPoints.clear();
    points.clear();
    
    int clusterIdx = -1;
    
    for (int ii = 0; ii < argpoints.size(); ++ii) {
        Point p;
        p.p = argpoints[ii];
        p.cluster = NOT_CLASSIFIED;
        p.ptsCnt = 0;
        
        points.push_back(p);
    }
    
    adjPoints.resize(points.size());
    
    // Check near Points (sped up by Kd tree)
    KdTree kdTree(argpoints, 10);
    kdTree.index->buildIndex();
    
    for (int i = 0; i < points.size(); ++i) {
        std::vector<nanoflann::ResultItem<size_t, float>> matches;
        
        float point[3] = {points[i].p.x, points[i].p.y, points[i].p.z};
        kdTree.index->radiusSearch(&point[0], epsilon, matches);
        
        for (int j = 0; j < matches.size(); ++j) {
            auto iter = matches[j];
            adjPoints[i].push_back((int)iter.first);
            points[i].ptsCnt++;
        }
    }
    
    // Classify as core or noise, through depth first search
    for (int i = 0; i < points.size(); i++) {
        if (points[i].cluster != NOT_CLASSIFIED) continue;
        
        if (isCoreObject(i)) {
            dfs(i, ++clusterIdx);
        } else {
            points[i].cluster = NOISE;
        }
    }
    
    // finally, asign points to clusters.
    std::vector<int> result;
    result.clear();
    for (int i = 0; i < points.size(); i++) {
        if (points[i].cluster != NOISE) {
            //std::cout << "points[i].cluster: " << std::to_string(points[i].cluster) << std::endl;
            result.push_back(points[i].cluster);
        } else {
            result.push_back(-1);
        }
    }
    
    return result;
}

}

}
