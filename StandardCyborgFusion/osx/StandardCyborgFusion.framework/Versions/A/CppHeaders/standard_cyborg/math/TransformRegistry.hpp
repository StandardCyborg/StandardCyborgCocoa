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

#include <unordered_map>
#include <string>
#include <vector>
#include "standard_cyborg/math/Transform.hpp"
#include "standard_cyborg/util/Result.hpp"

namespace standard_cyborg {
namespace math {

struct Transform;

/* TransformRegistry stores a set of transforms, each connecting a source and destination frame
 * and allows answering questions about relationships between any two frames. Upon registering known
 * transforms (or inverse transforms; it accepts either), you may query the transform between two
 * frames.
 */
class TransformRegistry {

public:
  TransformRegistry ();
  TransformRegistry (const std::vector<Transform>& transformList);

  /** Register a transform, storing a copy linked by its src and dest frame names.
   * If a transform already exists between the specified frames, the existing transorm is overwritten.
   * If the transform would result in a cycle, returns an error. */
  Result<bool> registerTransform(const Transform& transform);

  /** Register a list of transforms.
   * If the list contains cycles, registration is immediately halted and an error returned. */
  Result<bool> registerTransforms(const std::vector<Transform>& transformList);

  /** Remove a transform from the registry */ 
  Result<bool> deregisterTransform(const Transform& transform);

  /** Compute the path from source frame to destination frame.
   * The path consists of a list of frame names connecting the two, from source to destination.
   * If no path exists, returns an empty path.
   * If either srcFrame or destFrame is unknown, returns an error. */
  Result<std::vector<std::string>> path(const std::string& srcFrame, const std::string& destFrame) const;

  /** Return a transform from the specified source frame to the destination frame.
   * If no path exists, returns an empty path.
   * If either srcFrame or destFrame is unknown, returns an error. */
  Result<Transform> getTransform(const std::string& srcFrame, const std::string& destFrame) const;

  /** Returns true if one src frame is accessible from the dest frame.
   * If either srcFrame or destFrame is unknown, returns an error. */
  Result<bool> areConnected(const std::string& srcFrame, const std::string& destFrame) const;
  /** Return all transforms as a list.
    * Transforms between all adjacent frames are returned, though without guarantees about direction.
    * That is, if provided a transform from frame1 to frame2, the returned list may contain the inverse
    * transform connecting frame2 to frame1. */
  std::vector<Transform> toList () const;

private:

  std::unordered_map<std::string, std::unordered_map<std::string, Transform> > edges;
};

} // namespace math
} // namespace standard_cyborg
