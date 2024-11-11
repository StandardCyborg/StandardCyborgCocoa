//
//  DebugLog.h
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 5/1/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

// This preprocessor macro is #defined in the build settings via XCODE_ACTION_${ACTION}=1
#if XCODE_ACTION_install

#include <stdarg.h>

static inline void __unused_consumer(__attribute__((unused)) int x, ...) { }

#define DEBUG_LOG(...)  __unused_consumer(0, __VA_ARGS__);

#else /* XCODE_ACTION_install */

#define DEBUG_LOG(...)  do { fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n"); } while (0);

#endif /* XCODE_ACTION_install */
