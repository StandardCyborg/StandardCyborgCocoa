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

#include "standard_cyborg/sc3d/Point2D.hpp"
#include "standard_cyborg/sc3d/Size2D.hpp"

namespace standard_cyborg {
namespace sc3d {

struct Rect2D {
    Size2D size;
    Point2D origin;
};

} // namespace sc3d
} // namespace standard_cyborg

