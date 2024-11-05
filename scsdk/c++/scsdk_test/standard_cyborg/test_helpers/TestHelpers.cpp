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

#include "standard_cyborg/test_helpers/TestHelpers.hpp"

#ifndef SC_TEST_DEFAULT_FIXTURES_DIR
#define SC_TEST_DEFAULT_FIXTURES_DIR "/opt/scsdk/scsdk/c++/test_fixture_data"
#endif

#include <cstdlib>

namespace standard_cyborg {

std::string getTestCasesPath() {
    auto p = std::getenv("SC_TEST_FIXTURES_DIR");
    if (p) {
        return p;
    } else {
        return SC_TEST_DEFAULT_FIXTURES_DIR;
    }
}

}
