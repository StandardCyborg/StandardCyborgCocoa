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

#include "standard_cyborg/math/TransformRegistry.hpp"
#include "standard_cyborg/math/Mat3x4.hpp"

#include <algorithm>
#include <queue>

using ::standard_cyborg::Result;

namespace standard_cyborg {
namespace math {

TransformRegistry::TransformRegistry () {}

TransformRegistry::TransformRegistry (const std::vector<Transform>& transformList) {
    registerTransforms(transformList);
}

Result<bool> TransformRegistry::deregisterTransform(const Transform& transform) {
    if (edges.count(transform.srcFrame) == 0) {
        return {.error = "Invalid attempt to deregister transform with unknown source frame" + transform.srcFrame };
    }
    if (edges.count(transform.destFrame) == 0) {
        return {.error = "Invalid attempt to deregister transform with unknown destination frame \"{}\"" + transform.destFrame };
    }
    
    auto& srcEdges = edges.at(transform.srcFrame);
    auto& destEdges = edges.at(transform.destFrame);
    
    if (srcEdges.count(transform.destFrame) == 0 || destEdges.count(transform.srcFrame) == 0) {
        return {.error = "Invalid attempt to deregister transform between disconnected frames: source = " + transform.srcFrame + ", destination = " + transform.destFrame};
    }
    
    srcEdges.erase(transform.destFrame);
    destEdges.erase(transform.srcFrame);
    
    return {.value = true};
}

Result<bool> TransformRegistry::registerTransform(const Transform& transform) {
    bool areAdjacent = edges.count(transform.srcFrame) && edges.at(transform.srcFrame).count(transform.destFrame);
    
    if (!areAdjacent) {
        // If they're adjacent, we can just overwrite them. If not, then we check for an existing connection.
        // This could potentially get expensive, but we don't currently expect complicated graphs.
        Result<bool> connectedResult = areConnected(transform.srcFrame, transform.destFrame);
        if (connectedResult.IsOk() && *(connectedResult.value)) {
            return {.error = "Addition of transforms between frames " + transform.srcFrame + " and " + transform.destFrame + " would create a circular reference. Ignoring transform."};
        }
    }
    
    edges[transform.srcFrame][transform.destFrame] = transform;
    edges[transform.destFrame][transform.srcFrame] = transform.inverse();
    
    return {.value = true};
}

Result<bool> TransformRegistry::registerTransforms(const std::vector<Transform>& transformList) {
    for(const auto& transform : transformList) {
        Result<bool> registrationResult = registerTransform(transform);
        if (!registrationResult.IsOk()) {
            return {.error = registrationResult.error};
        }
    }
    return {.value = true};
}

Result<std::vector<std::string>> TransformRegistry::path(const std::string& srcFrame, const std::string& destFrame) const {
    // We perform breadth-first search (BFS). We start at the destination frame and trace outward until we
    // encounter the requested source frame. For each visited frame, we store the name of the frame from
    // which we arrived. As soon as we find the source frame, we trace parents back to the destination.
    
    // Early-out if the src/dest frames are not in the graph at all
    if (!edges.count(srcFrame)) return {.error = "Invalid access of unknown frame " + srcFrame};
    if (!edges.count(destFrame)) return {.error = "Invalid access of unknown frame " + destFrame};
    
    if (srcFrame == destFrame) return {.value = std::vector<std::string>({srcFrame})};
    
    // This yields our path and also serves as a visted marker
    std::unordered_map<std::string, std::string> parentFrame;
    
    std::queue<std::string> frameQueue;
    frameQueue.push(destFrame);
    std::string curFrame;
    
    while (!frameQueue.empty()) {
        curFrame = frameQueue.front();
        frameQueue.pop();
        
        // Something is internally inconsistent if we arrived here and can't even go back where we came from
        if (!edges.count(curFrame)) return {.error = "Invalid access of unknown frame, this is likely the result of an internal error." + curFrame };
        
        for (const auto& neighbor : edges.at(curFrame)) {
            const std::string& frame = neighbor.first;
            
            // Skip if this frame is visited or if it's where you started
            if (parentFrame.count(frame) || frame == destFrame) continue;
            
            parentFrame[frame] = curFrame;
            
            if (frame == srcFrame) {
                // We've located the frame; flush the queue and break out of the current iteration
                std::queue<std::string> empty;
                std::swap(frameQueue, empty);
                
                // Set this to indicate we've found the correct frame
                curFrame = frame;
                break;
            }
            
            frameQueue.push(frame);
        }
    }
    
    // In this case we've traversed as much as the whole graph but did not actually find our desired frame.
    // We could probably get this for free by reorganizing the fence-cases throughout this function, but
    // this seems a perfectly equivalent and robust check.
    if (curFrame != srcFrame) {
        return {.value = std::vector<std::string>({})};
    }
    
    // Build up a path from the source to the destination
    curFrame = srcFrame;
    std::vector path = {curFrame};
    while (parentFrame.count(curFrame)) {
        curFrame = parentFrame.at(curFrame);
        path.push_back(curFrame);
    }
    
    return {.value = path};
}

Result<Transform> TransformRegistry::getTransform(const std::string& srcFrame, const std::string& destFrame) const {
    const auto& pathResult = path(srcFrame, destFrame);
    if (!pathResult.IsOk()) return {.error = pathResult.error};
    
    const std::vector<std::string>& path = *(pathResult.value);
    Mat3x4 t;
    
    for (int i = path.size() - 2; i >= 0; i--) {
        if (!edges.count(path[i])) return {.error =
            "Invalid access of frame " + path[i] };
        const auto adj = edges.at(path[i]);
        
        if (!adj.count(path[i + 1])) return {.error =
            "Invalid access of frame " + path[i + 1] };
        const Mat3x4& tNext = Mat3x4::fromTransform(adj.at(path[i + 1]));
        
        t = tNext * t;
    }
    
    return {.value = Transform::fromMat3x4(t, srcFrame, destFrame)};
}

Result<bool> TransformRegistry::areConnected(const std::string& srcFrame, const std::string& destFrame) const {
    // This could be *slightly* optimized by not storing the path, but it's not worthwhile.
    const auto pathResult = path(srcFrame, destFrame);
    if (!pathResult.IsOk()) {
        return {.error = pathResult.error};
    }
    return {.value = pathResult.value->size() > 0};
}

std::vector<Transform> TransformRegistry::toList () const {
    std::vector<Transform> output;
    std::unordered_map<std::string, std::unordered_map<std::string, bool>> visited;
    for (const auto& pair : edges) {
        const std::string& srcFrame = pair.first;
        for (const auto& nestedPair : pair.second) {
            const std::string& destFrame = nestedPair.first;
            if (visited.count(srcFrame) && visited[srcFrame].count(destFrame)) continue;
            visited[srcFrame][destFrame] = true;
            visited[destFrame][srcFrame] = true;
            output.push_back(nestedPair.second);
        }
    }
    return output;
}

} // namespace math
} // namespace standard_cyborg
