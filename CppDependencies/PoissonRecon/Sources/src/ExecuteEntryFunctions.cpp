//
//  ExecuteEntryFunctions.cpp
//  PoissonRecon
//
//  Created by Aaron Thompson on 2/13/19.
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

    if (closed) {
        _PoissonReconExecute(inputFilePath, outputFilePath, parameters, progressHandler, FEMSignatureDirichlet());
    } else {
        _PoissonReconExecute(inputFilePath, outputFilePath, parameters, progressHandler, FEMSignatureNeumann());
    }
}

int SurfaceTrimmerExecute(const char* inputFilePath,
                          const char* outputFilePath,
                          SurfaceTrimmerParameters parameters,
                          std::function<bool (float)> progressHandler)
{
    return _SurfaceTrimmerExecute(inputFilePath, outputFilePath, parameters, progressHandler);
}
