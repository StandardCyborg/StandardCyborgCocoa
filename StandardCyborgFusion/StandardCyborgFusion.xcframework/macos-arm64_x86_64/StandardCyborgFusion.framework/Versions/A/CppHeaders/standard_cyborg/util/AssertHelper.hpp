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

// clang-format off
#if EMBIND_ONLY

#include <iostream>


#define SCASSERT(condition, webConsoleErrorMessage)\
    ({\
        if(!(condition)) { \
            std::cerr << __FILE__ << ":" << __LINE__ << std::endl; \
            std::cerr << (webConsoleErrorMessage) << std::endl; \
        }\
    })

#elif DEBUG

#include <cassert>
#include <iostream>

#define SCASSERT(condition, messageForInternalSDKDeveloper)\
    ({\
        if (!(condition)) std::cerr << (messageForInternalSDKDeveloper) << std::endl;\
        assert(condition); \
    })

#else // is production

#include <iostream>

#define SCASSERT(condition, messageForSDKUser)\
    ({\
        if (!(condition)) { \
            std::cerr << (messageForSDKUser) << std::endl; \
            throw; \
        } \
    })

#endif

// clang-format on
