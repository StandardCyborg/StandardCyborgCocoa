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

#include <optional>
#include <string>

namespace standard_cyborg {

// Why wrap things in Result?
//  * in offline pipelines, it can be OK to throw an exception or do an assert fail
// on error.  the developer will probably dig after it.
//  * in consumer products, and production cloud pipelines, you often want a 
// more graceful failure, and to log the error & traceback to some error logging
// service. in some extreme cases, you might have a C++ build w/out exceptions.
// for assertions, there's also the NDEBUG problem with assert.h .  while Result
// isn't perfect as-is, it at least encapsulates "error codes" for the MVP of 
// this product

// A hacky std::expected<> while the committee seeks consensus
template <typename T>
struct Result {
  std::optional<T> value;
  std::string error;

  bool IsOk() const { return value.has_value(); }

  // Or use "{.value = v}"
  static Result<T> Ok(T &&v) {
    return {.value = std::move(v)};
  }

  // Or use "{.error = s}"
  static Result<T> Err(const std::string &s) {
    return {.error = s};
  }
};

using OkOrErr = Result<bool>;
static const OkOrErr kOK = {.value = true};

} // namespace standard_cyborg
