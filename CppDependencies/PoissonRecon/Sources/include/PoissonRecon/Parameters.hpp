//
//  Parameters.hpp
//  PoissonRecon
//
//  Created by Aaron Thompson on 2/13/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#ifndef Parameters_hpp
#define Parameters_hpp

struct PoissonReconParameters {
    int BaseDepth = 0;
    int BaseVCycles = 1;
    float CGSolverAccuracy = 0.001;
    int Depth = 8;
    int FullDepth = 5;
    int Iters = 8;
    int PointWeight = 2;
    float SamplesPerNode = 1.5;
    float Scale = 1.1;
    int Threads = 1;
};

struct SurfaceTrimmerParameters {
    int Smooth = 5;
    int Trim = 5;
    float IslandAreaRatio = 0.001;
    bool PolygonMesh = false;
};

#endif /* Parameters_hpp */
