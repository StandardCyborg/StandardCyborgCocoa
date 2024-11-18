//
//  ExecuteEntryFunctions.hpp
//  PoissonRecon
//
//  Created by Aaron Thompson on 2/13/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#ifndef ExecuteEntryFunctions_hpp
#define ExecuteEntryFunctions_hpp

#include "Parameters.hpp"
#include <functional>

/** @param progressHandler Reports progress, from 0-1. The handler should return false if progress should stop, i.e. it was canceled.
    @param closed If true, uses Dirichlet boundary parameters instead of Neumann.
 */
extern void PoissonReconExecute(const char *inputFilePath,
                                const char *outputFilePath,
                                bool closed,
                                PoissonReconParameters parameters,
                                std::function<bool (float)> progressHandler);

/** Returns 0 on success, nonzero on error */
extern int SurfaceTrimmerExecute(const char* inputFilePath,
                                 const char* outputFilePath,
                                 SurfaceTrimmerParameters parameters,
                                 std::function<bool (float)> progressHandler);

#endif /* ExecuteEntryFunctions_hpp */
