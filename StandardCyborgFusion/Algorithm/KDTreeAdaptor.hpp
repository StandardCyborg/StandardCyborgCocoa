//
//  KdTreeAdaptor.hpp
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 8/7/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#pragma once

#include <standard_cyborg/math/Vec3.hpp>

#include <nanoflann.hpp>

/** Squared Euclidean (L2) distance functor (suitable for low-dimensionality
 * datasets, like 2D or 3D point clouds) Corresponding distance traits:
 * nanoflann::metric_L2_Simple \tparam T Type of the elements (e.g. double,
 * float, uint8_t) \tparam _DistanceType Type of distance variables (must be
 * signed) (e.g. float, double, int64_t)
 */
template <class T, class DataSource, typename _DistanceType = T>
struct L2_Simple_Adaptor_DIM3 {
    typedef T ElementType;
    typedef _DistanceType DistanceType;
    
    const DataSource& data_source;
    
    L2_Simple_Adaptor_DIM3(const DataSource& _data_source) :
        data_source(_data_source)
    {}
    
    inline DistanceType evalMetric(const T* a, const size_t b_idx, size_t size) const
    {
        const DistanceType diff0 = a[0] - data_source.kdtree_get_pt(b_idx, 0);
        const DistanceType diff1 = a[1] - data_source.kdtree_get_pt(b_idx, 1);
        const DistanceType diff2 = a[2] - data_source.kdtree_get_pt(b_idx, 2);
        return diff0 * diff0 + diff1 * diff1 + diff2 * diff2;
    }
    
    template <typename U, typename V>
    inline DistanceType accum_dist(const U a, const V b, const size_t) const
    {
        return (a - b) * (a - b);
    }
};

/** Metaprogramming helper traits class for the L2_simple (Euclidean) metric */
struct metric_L2_Simple_DIM3 : public nanoflann::Metric {
    template <class T, class DataSource>
    struct traits {
        typedef L2_Simple_Adaptor_DIM3<T, DataSource> distance_t;
    };
};

class SingleNNResultSet {
private:
    size_t* _index;
    float* _dist;
    size_t _count;
    
public:
    inline SingleNNResultSet() :
        _index(0), _dist(0), _count(0)
    {}
    
    inline void init(size_t* index, float* dist)
    {
        _index = index;
        _dist = dist;
        _count = 0;
        
        *_dist = (std::numeric_limits<float>::max)();
    }
    
    inline size_t size() const { return _count; }
    
    inline bool full() const { return _count == 1; }
    
    /**
     * Called during search to add an element matching the criteria.
     * @return true if the search should be continued, false if the results are sufficient
     */
    inline bool addPoint(float dist, size_t index)
    {
        if (*_dist > dist) {
            *_dist = dist;
            *_index = index;
            _count = 1;
        }
        
        // tell caller that the search shall continue
        return true;
    }
    
    inline float worstDist() const { return *_dist; }
};

// Annoyingly, the nanoflann-provided KDTreeEigenMatrixAdaptor class works with X by 3 matrices, not our 3 by X ones
// This means we have to implement our own
template <class MatrixType, int DIM = -1, class Distance = metric_L2_Simple_DIM3, typename IndexType = size_t>
struct KdTreeAdaptor {
    typedef KdTreeAdaptor<MatrixType, DIM, Distance> self_t;
    typedef float num_t;
    typedef typename Distance::template traits<num_t, self_t>::distance_t metric_t;
    typedef nanoflann::KDTreeSingleIndexAdaptor<metric_t, self_t, DIM, IndexType> index_t;
    index_t* index;
    
    /// Constructor: takes a const ref to the matrix object with the data points
    KdTreeAdaptor(const MatrixType& mat, const size_t leafMaxSize = 32) :
        m_data_matrix(mat)
    {
        const int rowCount = 3;
        // The last param is # threads, and 0 means auto-determine
        auto adaptorParams = nanoflann::KDTreeSingleIndexAdaptorParams(leafMaxSize, KDTreeSingleIndexAdaptorFlags::None, 2);
        index = new index_t(rowCount, *this, adaptorParams);
        index->buildIndex();
    }
    
    ~KdTreeAdaptor() { delete index; }
    
    const MatrixType& m_data_matrix;
    
    /// Query for the closest 3D point to a given point
    inline void query(const num_t* query_point, const size_t num_closest, IndexType* out_indices, num_t* out_distances_sq) const
    {
        SingleNNResultSet resultSet;
        resultSet.init(out_indices, out_distances_sq);
        index->findNeighbors(resultSet, query_point, nanoflann::SearchParameters());
    }
    
    const self_t& derived() const { return *this; }
    
    self_t& derived() { return *this; }
    
    inline size_t kdtree_get_point_count() const { return m_data_matrix.size(); }
    
    /// Returns the dim'th component of the idx'th point in the class:
    inline num_t kdtree_get_pt(const size_t idx, int dim) const
    {
        return m_data_matrix[idx][dim];
    }
    
    /// Optional bounding-box computation: return false to default to a standard bbox computation loop.
    template <class BBOX>
    bool kdtree_get_bbox(BBOX&) const { return false; }
};

template <class MatrixType>
class KdTreeAdaptor<const MatrixType, 3, metric_L2_Simple_DIM3, size_t>
{};

typedef KdTreeAdaptor<std::vector<standard_cyborg::math::Vec3>> SCVec3KdTreeAdaptor;
