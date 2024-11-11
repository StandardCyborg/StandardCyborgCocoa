//
//  SparseSurfelLandmarksIndex.hpp
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 4/22/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <standard_cyborg/util/IncludeEigen.hpp>
#include <map>
#include <unordered_map>
#include <vector>

#include "Surfel.hpp"

class SparseSurfelLandmarksIndex {
public:
    void addHit(int surfelIndex, int landmarkIndex);
    bool removeHit(int surfelIndex, int landmarkIndex);
    void removeAllHits();
    int getHitCount(int surfelIndex, int landmarkIndex) const;
    int size() const;
    
    // Given a list of surfel indices which were deleted, it deletes those from this
    //  index (the easy part) and shifts over all the remaining indices accordingly
    // (done by stepping through the index in parallel with the deleted vertices and
    //  tracking the cumulative shift)
    //
    // This method presumes the vertices it recieves are pre-sorted. This shouldn't
    // be a problem since we sweep through the vertices in order when deleting low-
    // confidence surfels.
    void deleteSurfelLandmarksAndRenumber(const std::vector<int>& sortedVertexIndicesToDelete);
    
    typedef std::map<int, std::unordered_map<int, int>>::const_iterator const_iterator;
    const_iterator begin() const;
    const_iterator end() const;
    
    void iterateHits(const std::function<void(int surfelIndex, int landmarkIndex, int hitCount)> callback) const;
    
    std::unordered_map<int, Eigen::Vector3f> computeCentroids(const Surfel* surfels) const;
    
private:
    // This is our primary storage of counts, first by surfel index, then by
    // landmark index. We store wrt landmark index by unordered map because we
    // don’t care about the ordering. We store wrt surfel index by map because
    // renumbering the map as surfels are culled requires some tricks. In
    // particular, we iterate through the map (wrt surfel index) and a list of deleted surfels
    // together. The fact that both are ordered and remain ordered means
    // we can do some low-level const_casting to modify the map content
    // directly via the iterator rather than explicitly deleting and inserting
    // elements which would be much more expensive.
    
    // tl;dr: keys are surfel indices, values are hit counts per landmark index
    std::map<int, std::unordered_map<int, int>> _landmarkHitCountsBySurfelIndex;
};
