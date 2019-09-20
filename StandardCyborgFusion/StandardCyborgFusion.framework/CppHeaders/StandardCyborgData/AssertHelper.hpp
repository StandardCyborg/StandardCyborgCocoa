//
//  AssertHelper.hpp
//  StandardCyborgSDK
//
//  Created by Ricky Reusser on 9/17/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

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
        std::cerr << (messageForInternalSDKDeveloper) << std::endl;\
        assert(condition); \
    })

#else // is production

#define SCASSERT(condition, messageForSDKUser)\
    ({\
        if (!(condition)) { \
            std::cerr << (messageForSDKUser) << std::endl; \
            throw; \
        } \
    })

#endif
