//
//  SparseSurfelLandmarksIndex.cpp
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 4/22/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#include "SparseSurfelLandmarksIndex.hpp"

SparseSurfelLandmarksIndex::const_iterator SparseSurfelLandmarksIndex::begin() const
{
    return _landmarkHitCountsBySurfelIndex.begin();
}

SparseSurfelLandmarksIndex::const_iterator SparseSurfelLandmarksIndex::end() const
{
    return _landmarkHitCountsBySurfelIndex.end();
}

void SparseSurfelLandmarksIndex::addHit(int surfelIndex, int landmarkIndex)
{
    // Add a map for this surfel if one doesn't exist
    if (!_landmarkHitCountsBySurfelIndex.count(surfelIndex)) {
        _landmarkHitCountsBySurfelIndex[surfelIndex] = std::unordered_map<int, int>();
    }
    
    std::unordered_map<int, int>& countsForSurfel = _landmarkHitCountsBySurfelIndex[surfelIndex];
    
    // Get the count or assume zero
    //int currentCount = countsForSurfel.at(landmarkIndex);
    int currentCount = countsForSurfel.count(landmarkIndex) ? countsForSurfel.at(landmarkIndex) : 0;
    
    // Set incremented count
    countsForSurfel[landmarkIndex] = currentCount + 1;
}

int SparseSurfelLandmarksIndex::size() const
{
    return (int)_landmarkHitCountsBySurfelIndex.size();
}

int SparseSurfelLandmarksIndex::getHitCount(int surfelIndex, int landmarkIndex) const
{
    // Return zero if this surfel is not registered at all
    if (!_landmarkHitCountsBySurfelIndex.count(surfelIndex)) {
        return 0;
    }
    
    const std::unordered_map<int, int>& countsForSurfel = _landmarkHitCountsBySurfelIndex.at(surfelIndex);
    
    // Return an existing count or assume zero
    return countsForSurfel.count(landmarkIndex) ? countsForSurfel.at(landmarkIndex) : 0;
}

bool SparseSurfelLandmarksIndex::removeHit(int surfelIndex, int landmarkIndex)
{
    // No-op if nothing registered for this surfel
    if (!_landmarkHitCountsBySurfelIndex.count(surfelIndex)) {
        return false;
    }
    
    std::unordered_map<int, int>& countsForSurfel = _landmarkHitCountsBySurfelIndex[surfelIndex];
    
    // Get the count or assume zero
    int currentCount = countsForSurfel.count(landmarkIndex) ? countsForSurfel.at(landmarkIndex) : 0;
    
    // This should never equal zero (it should have been deleted if that's the case), but it's
    // trivially easy not worth failing to make it handle that case just fine anyway
    if (currentCount <= 1) {
        countsForSurfel.erase(landmarkIndex);
    } else {
        countsForSurfel[landmarkIndex] = currentCount - 1;
    }
    
    // If there are no landmarks left for this surfel, delete the list
    if (!countsForSurfel.size()) {
        _landmarkHitCountsBySurfelIndex.erase(surfelIndex);
    }
    
    return true;
}

void SparseSurfelLandmarksIndex::removeAllHits()
{
    _landmarkHitCountsBySurfelIndex.clear();
}

void SparseSurfelLandmarksIndex::deleteSurfelLandmarksAndRenumber(const std::vector<int>& sortedSurfelIndicesToDelete)
{
    // Track how far we're shifting
    int shift = 0;
    
    // Iterate through currently-tagged surfel landmarks
    for (auto it = _landmarkHitCountsBySurfelIndex.begin(); it != _landmarkHitCountsBySurfelIndex.end();) {
        int surfelIndex = it->first;
        
        // Step through the sorted indices to keep them in line with the current surfel index
        while (shift < sortedSurfelIndicesToDelete.size() && surfelIndex > sortedSurfelIndicesToDelete[shift]) {
            shift++;
        }
        
        if (shift < sortedSurfelIndicesToDelete.size() && sortedSurfelIndicesToDelete[shift] == surfelIndex) {
            // If this surfel is deleted, simply delete it
            
            // A standard c++ idiom for deleting as you iterate:
            // https://stackoverflow.com/questions/8234779/how-to-remove-from-a-map-while-iterating-it
            
            it =_landmarkHitCountsBySurfelIndex.erase(it++);
        } else {
            // Otherwse, shift the surfel index over by the number of deleted surfels by modifying
            // the sorted map iterator *directly*
            if (shift) {
                // This is normally dangerous and forbidden, but as long as we keep everything strictly
                // ordered, then it starts and sorted map and ends a sorted map and we've only modified
                // the particular values a bit.
                const_cast<int&> (it->first) = it->first - shift;
            }
            
            ++it;
        }
    }
}

void SparseSurfelLandmarksIndex::iterateHits(const std::function<void(int surfelIndex, int landmarkIndex, int hitCount)> callback) const
{
    for (auto& [surfelIndex, hitCountsByLandmarkIndex] : _landmarkHitCountsBySurfelIndex)
    {
        for (auto [landmarkIndex, hitCount] : hitCountsByLandmarkIndex)
        {
            callback(surfelIndex, landmarkIndex, hitCount);
        }
    }
}

std::unordered_map<int, Eigen::Vector3f> SparseSurfelLandmarksIndex::computeCentroids(const Surfel* surfels) const
{
    std::unordered_map<int, std::pair<Eigen::Vector3f, float>> centroidsAndWeightsByLandmarkIndex;
    
    iterateHits([&centroidsAndWeightsByLandmarkIndex](int surfelIndex, int landmarkIndex, int weight) {
        centroidsAndWeightsByLandmarkIndex[landmarkIndex] = std::pair<Eigen::Vector3f, float>(Eigen::Vector3f::Zero(), 0);
    });
    
    iterateHits([surfels, &centroidsAndWeightsByLandmarkIndex](int surfelIndex, int landmarkIndex, int hitCount) {
        const Surfel& surfel = surfels[surfelIndex];
        auto& centroidAndWeight = centroidsAndWeightsByLandmarkIndex[landmarkIndex];
        float weight = hitCount; // Using the hit count as a weight
        
        centroidAndWeight.first += weight * surfel.position;
        centroidAndWeight.second += weight;
    });
    
    
    std::unordered_map<int, Eigen::Vector3f> result;
    
    for (auto& [landmarkIndex, centroidAndWeight] : centroidsAndWeightsByLandmarkIndex) {
        auto [centroid, weight] = centroidAndWeight;
        
        result[landmarkIndex] = centroid / weight;
    }
    
    return result;
}
