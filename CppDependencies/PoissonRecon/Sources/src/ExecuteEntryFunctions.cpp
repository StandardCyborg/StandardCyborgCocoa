//
//  ExecuteEntryFunctions.cpp
//  PoissonRecon
//
//  Created by Aaron Thompson on 2/13/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#include <PoissonRecon/ExecuteEntryFunctions.hpp>
#include <PoissonRecon/PoissonReconExecute.hpp>
#include <PoissonRecon/SurfaceTrimmerExecute.hpp>

/**
 Why have these together instead of moving them into their respective *Execute source files?
 - There's a conflict between the OS-provided Point struct type and this library's Point,
   so you can't include both together.
 - If the compiler doesn't specialize both at the same time, it ends up with duplicate symbols
   when linking because it didn't know to name them differently (I think?)
 */

void PoissonReconExecute(const char *inputFilePath,
                         const char *outputFilePath,
                         bool closed,
                         PoissonReconParameters parameters,
                         std::function<bool (float)> progressHandler)
{
    typedef IsotropicUIntPack<3, FEMDegreeAndBType<1, BOUNDARY_NEUMANN>::Signature> FEMSignatureNeumann;
    typedef IsotropicUIntPack<3, FEMDegreeAndBType<1, BOUNDARY_DIRICHLET>::Signature> FEMSignatureDirichlet;
    typedef PointStreamColor<float> ColorType;
    
    if (closed) {
        _PoissonReconExecute<ColorType>(inputFilePath, outputFilePath, parameters, progressHandler, FEMSignatureDirichlet());
    } else {
        _PoissonReconExecute<ColorType>(inputFilePath, outputFilePath, parameters, progressHandler, FEMSignatureNeumann());
    }
}

int SurfaceTrimmerExecute(const char* inputFilePath,
                          const char* outputFilePath,
                          SurfaceTrimmerParameters parameters,
                          std::function<bool (float)> progressHandler)
{
    typedef PointStreamNormal<float, 3> NormalType;
    typedef PointStreamColor<float> ColorType;
    return _SurfaceTrimmerExecute<NormalType, ColorType>(inputFilePath, outputFilePath, parameters, progressHandler);
}
