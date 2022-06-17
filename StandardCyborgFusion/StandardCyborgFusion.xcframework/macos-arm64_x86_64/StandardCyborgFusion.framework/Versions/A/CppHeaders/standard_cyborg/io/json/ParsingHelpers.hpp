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

#include <nlohmann/json.hpp>
#include <vector>

namespace standard_cyborg {
namespace io {
namespace json {

template <typename T>
std::vector<T> toNumericVectorWithNullAsNAN(nlohmann::basic_json<>& input) {
    std::vector<T> result (input.size());
    for (int i = 0; i < input.size(); i++) {
        nlohmann::basic_json<> value = input[i];
        if (value.is_null()) {
            result[i] = NAN;
            continue;
        }
        result[i] = static_cast<T>(input[i]);
    }
    return result;
}

} // namespace json
} // namespace io
} // namespace standard_cyborg
