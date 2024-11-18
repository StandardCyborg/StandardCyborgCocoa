//
//  PoissonReconExecute.h
//  PoissonRecon
//
//  Created by Aaron Thompson on 5/8/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

/**
 Example parameters for execution of
 PoissonRecon --in Scan.ply --out PoissonRecon.ply --colors --normals --density --depth 12
 {
 ASCII: true,
 BaseDepth: 0,
 BaseVCycles: 1,
 BoundaryNames: ["free", "Dirichlet", "Neumann"],
 BType: 3,
 CGSolverAccuracy: 0.00100000005,
 Colors: true,
 Confidence: 0,
 ConfidenceBias: 0,
 DataX: 32,
 Degree: 1,
 Density: true,
 Depth 12,
 ExactInterpolation: false,
 FEMTreeRealNames: ["float", "double"],
 FullDepth: 5,
 Grid: NULL,
 In: "Scan.ply",
 InCore: false,
 Iters: 8,
 KernelDepth: 0,
 LinearFit: false,
 MaxMemoryGB: 0,
 messageWriter: {
 outputFile: NULL,
 echoSTDOUT: false,
 },
 NoComments: false,
 NonManifold: false,
 Normals: true,
 Out: "PoissonRecon.ply",
 Performance: false,
 PointWeight: 2,
 PolygonMesh: false,
 PrimalGrid: false,
 SamplesPerNode: 1.5,
 Scale: 1.10000002,
 ShowGlobalResidualNames: ["show none", "show last", "show all"],
 ShowResidual: false,
 TempDir: NULL,
 Threads: 1,
 Transform: NULL,
 Tree: NULL,
 Verbose: false,
 Width: 0
 }
 */

#include "MyMiscellany.h"
#include "FEMTree.h"
#include "Ply.h"
#include "PointStreamData.h"

#define DATA_DEGREE 0                            // The order of the B-Spline used to splat in data for color interpolation
#define WEIGHT_DEGREE 2                            // The order of the B-Spline used to splat in the weights for density estimation
#define NORMAL_DEGREE 2                            // The order of the B-Spline used to splat in the normals for constructing the Laplacian constraints
#define DEFAULT_FEM_DEGREE 1                    // The default finite-element degree
#define DEFAULT_FEM_BOUNDARY BOUNDARY_NEUMANN    // The default finite-element boundary type

XForm<float, 3 + 1> GetBoundingBoxXForm(Point<float, 3> min, Point<float, 3> max, float scaleFactor)
{
    Point<float, 3> center = (max + min) / 2;
    float scale = max[0] - min[0];
    for (int d = 1; d < 3; d++) {
        scale = std::max<float>(scale, max[d] - min[d]);
    }
    
    scale *= scaleFactor;
    
    for (int i = 0; i < 3; i++) {
        center[i] -= scale / 2;
    }
    
    auto tXForm = XForm<float, 3 + 1>::Identity();
    auto sXForm = XForm<float, 3 + 1>::Identity();
    
    for (int i = 0; i < 3; i++) {
        sXForm(i, i) = 1.0f / scale;
        tXForm(3, i) = -center[i];
    }
    
    return sXForm * tXForm;
}

XForm<float, 4> GetPointXForm(InputPointStream<float, 3>& stream, float scaleFactor)
{
    Point<float, 3> min, max;
    stream.boundingBox(min, max);
    return GetBoundingBoxXForm(min, max, scaleFactor);
}

struct ConstraintDual
{
    float target, weight;
    ConstraintDual(float t, float w) : target(t), weight(w) { }
    CumulativeDerivativeValues<float, 3, 0> operator()(const Point<float, 3>& p) const {
        return CumulativeDerivativeValues<float, 3, 0>(target * weight);
    };
};

struct SystemDual {
    float weight;
    SystemDual(float w)
    : weight(w)
    { }
    
    CumulativeDerivativeValues<float,  3, 0> operator()(const Point<float, 3>& p, const CumulativeDerivativeValues<float,  3, 0>& dValues) const {
        return dValues * weight;
    };
    
    CumulativeDerivativeValues<double, 3, 0> operator()(const Point<float, 3>& p, const CumulativeDerivativeValues<double, 3, 0>& dValues) const {
        return dValues * weight;
    };
};

template<typename Vertex, unsigned int ... FEMSigs, typename ... SampleData>
void ExtractMesh(UIntPack<FEMSigs ...>,
                 std::tuple<SampleData ...>,
                 FEMTree<sizeof ...(FEMSigs), float>& tree,
                 const DenseNodeData<float, UIntPack<FEMSigs ...>>& solution,
                 float isoValue,
                 const std::vector<typename FEMTree<sizeof ...(FEMSigs), float>::PointSample>* samples,
                 std::vector<MultiPointStreamData<float, PointStreamNormal<float, 3>, MultiPointStreamData<float, SampleData ...>>>* sampleData,
                 const typename FEMTree<sizeof ...(FEMSigs), float>::template DensityEstimator<WEIGHT_DEGREE>* density,
                 std::function<void (Vertex&, Point<float, 3>, float, MultiPointStreamData<float, PointStreamNormal<float, 3>, MultiPointStreamData<float, SampleData ...>>)> SetVertex,
                 MessageWriter& messageWriter,
                 std::vector<std::string> &comments,
                 XForm<float, sizeof...(FEMSigs) + 1> iXForm,
                 const char *Out)
{
    const bool ASCII = true;
    const int DataX = 32;
    const bool LinearFit = false;
    const bool NoComments = true;
    const bool NonManifold = false;
    const bool PolygonMesh = false;
    
    typedef UIntPack<FEMSigs ...> Sigs;
    typedef PointStreamNormal<float, 3> NormalPointSampleData;
    typedef MultiPointStreamData<float, SampleData ...> AdditionalPointSampleData;
    typedef MultiPointStreamData<float, NormalPointSampleData, AdditionalPointSampleData> TotalPointSampleData;
    static const unsigned int DataSig = FEMDegreeAndBType<DATA_DEGREE, BOUNDARY_FREE>::Signature;
    typedef typename FEMTree<3, float>::template DensityEstimator<WEIGHT_DEGREE> DensityEstimator;
    
    char tempHeader[1024];
    {
        char tempPath[1024];
        tempPath[0] = 0;
        SetTempDirectory(tempPath, sizeof(tempPath));
        
        if (strlen(tempPath) == 0) sprintf(tempPath, ".%c", FileSeparator);
        if (tempPath[strlen(tempPath) - 1] == FileSeparator) sprintf(tempHeader, "%sPR_", tempPath);
        else                                                 sprintf(tempHeader, "%s%cPR_", tempPath, FileSeparator);
    }
    CoredMeshData<Vertex> *mesh = new CoredFileMeshData<Vertex>(tempHeader);
    
    typename IsoSurfaceExtractor<3, float, Vertex>::IsoStats isoStats;
    
    if (sampleData)
    {
        SparseNodeData<ProjectiveData<TotalPointSampleData, float>, IsotropicUIntPack<3, DataSig>> _sampleData = tree.template setDataField<DataSig, false>(*samples, *sampleData, (DensityEstimator*)NULL);
        for (const RegularTreeNode<3, FEMTreeNodeData>* n = tree.tree().nextNode(); n; n = tree.tree().nextNode(n))
        {
            ProjectiveData<TotalPointSampleData, float>* clr = _sampleData(n);
            if (clr) { (*clr) *= (float)pow(DataX, tree.depth(n)); }
        }
        
        isoStats = IsoSurfaceExtractor<3, float, Vertex>::template Extract<TotalPointSampleData>(Sigs(), UIntPack<WEIGHT_DEGREE>(), UIntPack<DataSig>(), tree, density, &_sampleData, solution, isoValue, *mesh, SetVertex, !LinearFit, !NonManifold, PolygonMesh, false);
    }
#if defined(__GNUC__) && __GNUC__ < 5
    // #warning "you've got me gcc version < 5"
    else isoStats = IsoSurfaceExtractor<3, float, Vertex>::template Extract<TotalPointSampleData>(Sigs(), UIntPack<WEIGHT_DEGREE>(), UIntPack<DataSig>(), tree, density, (SparseNodeData<ProjectiveData<TotalPointSampleData, float>, IsotropicUIntPack<3, DataSig>> *)NULL, solution, isoValue, *mesh, SetVertex, !LinearFit, !NonManifold, PolygonMesh, false);
#else // !__GNUC__ || __GNUC__ >= 5
    else isoStats = IsoSurfaceExtractor<3, float, Vertex>::template Extract<TotalPointSampleData>(Sigs(), UIntPack<WEIGHT_DEGREE>(), UIntPack<DataSig>(), tree, density, NULL, solution, isoValue, *mesh, SetVertex, !LinearFit, !NonManifold, PolygonMesh, false);
#endif // __GNUC__ || __GNUC__ < 4
    messageWriter("Vertices / Polygons: %d / %d\n", mesh->outOfCorePointCount() + mesh->inCorePoints.size(), mesh->polygonCount());
    std::string isoStatsString = isoStats.toString() + std::string("\n");
    messageWriter(isoStatsString.c_str());
    
    std::vector<std::string> noComments;
    if (!PlyWritePolygons<Vertex, float, 3>(Out, mesh, ASCII ? PLY_ASCII : PLY_BINARY_NATIVE, NoComments ? noComments : comments, iXForm)) {
        ERROR_OUT("Could not write mesh to: %s", Out);
    }
    
    delete mesh;
}

// Called templated as Execute<float, PointStreamColor<float>>(argc, argv, IsotropicUIntPack<3, FEMDegreeAndBType<1, BOUNDARY_NEUMANN>::Signature>())
template<typename ... SampleData, unsigned int ... FEMSigs>
void _PoissonReconExecute(const char *In,
                          const char *Out,
                          PoissonReconParameters params,
                          std::function<bool (float)> progressHandler,
                          UIntPack<FEMSigs ...>)
{
    typedef UIntPack<FEMSigs ...> Sigs;
    typedef UIntPack<FEMSignature<FEMSigs>::Degree ...> Degrees;
    typedef UIntPack<FEMDegreeAndBType<NORMAL_DEGREE, DerivativeBoundary<FEMSignature<FEMSigs>::BType, 1>::BType>::Signature ...> NormalSigs;
    typedef typename FEMTree<3, float>::template DensityEstimator<WEIGHT_DEGREE> DensityEstimator;
    typedef typename FEMTree<3, float>::template InterpolationInfo<float, 0> InterpolationInfo;
    typedef PointStreamNormal<float, 3> NormalPointSampleData;
    typedef MultiPointStreamData<float, SampleData ...> AdditionalPointSampleData;
    typedef MultiPointStreamData<float, NormalPointSampleData, AdditionalPointSampleData> TotalPointSampleData;
    typedef InputPointStreamWithData<float, 3, TotalPointSampleData> InputPointStream;
    typedef TransformedInputPointStreamWithData<float, 3, TotalPointSampleData> XInputPointStream;
    MessageWriter messageWriter;
    messageWriter.echoSTDOUT = false;
    std::vector<std::string> comments;
    
    messageWriter(comments, "*************************************************************\n");
    messageWriter(comments, "** Running Screened Poisson Reconstruction (Version %s) **\n", VERSION);
    messageWriter(comments, "*************************************************************\n");
    
    XForm<float, 3 + 1> xForm = XForm<float, 3 + 1>::Identity();
    
    float isoValue = 0;
    
    if (progressHandler(0) == false) { return; }
    
    FEMTree<3, float> tree(MEMORY_ALLOCATOR_BLOCK_SIZE);
    
    int pointCount;
    
    float pointWeightSum;
    std::vector<typename FEMTree<3, float>::PointSample>* samples = new std::vector<typename FEMTree<3, float>::PointSample>();
    std::vector<TotalPointSampleData>* sampleData = NULL;
    DensityEstimator* density = NULL;
    SparseNodeData<Point<float, 3>, NormalSigs>* normalInfo = NULL;
    float targetValue = (float)0.5;
    
    // Read in the samples (and color data)
    {
        sampleData = new std::vector<TotalPointSampleData>();
        std::vector<std::pair<Point<float, 3>, TotalPointSampleData>> inCorePoints;
        InputPointStream* pointStream = new PLYInputPointStreamWithData<float, 3, TotalPointSampleData>
        (
            In,
            TotalPointSampleData::PlyReadProperties(),
            TotalPointSampleData::PlyReadNum,
            TotalPointSampleData::ValidPlyReadProperties
        );
        
        typename TotalPointSampleData::Transform _xForm(xForm);
        XInputPointStream _pointStream([&](Point<float, 3>& p, TotalPointSampleData& d){ p = xForm * p, d = _xForm(d); }, *pointStream);
        xForm = params.Scale > 0 ? GetPointXForm(_pointStream, (float)params.Scale) * xForm : xForm;
        {
            typename TotalPointSampleData::Transform _xForm(xForm);
            XInputPointStream _pointStream([&](Point<float, 3>& p, TotalPointSampleData& d){ p = xForm*p, d = _xForm(d); }, *pointStream);
            auto ProcessData = [](const Point<float, 3>& p, TotalPointSampleData& d)
            {
                float l = (float)Length(std::get<0>(d.data).data);
                
                if (!l || l != l) { return -1.0f; }
                
                std::get<0>(d.data).data /= l;
                
                return 1.0f;
            };
            
            pointCount = FEMTreeInitializer<3, float>::template Initialize<TotalPointSampleData>(tree.spaceRoot(), _pointStream, params.Depth, *samples, *sampleData, true, tree.nodeAllocator, tree.initializer(), ProcessData);
        }
        
        delete pointStream;
        
        messageWriter("Input Points / Samples: %d / %d\n", pointCount, samples->size());
    }
    
    if (progressHandler(0.2) == false) { return; }
    
    int kernelDepth = params.Depth - 2;
    
    DenseNodeData<float, Sigs> solution;
    {
        DenseNodeData<float, Sigs> constraints;
        InterpolationInfo* iInfo = NULL;
        int solveDepth = params.Depth;
        
        tree.resetNodeIndices();
        
        // Get the kernel density estimator
        {
            density = tree.template setDensityEstimator<WEIGHT_DEGREE>(*samples, kernelDepth, params.SamplesPerNode, 1);
        }
        
        // Transform the Hermite samples into a vector field
        {
            normalInfo = new SparseNodeData<Point<float, 3>, NormalSigs>();
            *normalInfo = tree.setNormalField(NormalSigs(), *samples, *sampleData, density, pointWeightSum);
            
            #pragma omp parallel for
            for (int i = 0; i < normalInfo->size(); i++)
            {
                (*normalInfo)[i] *= -1.0f;
            }
            
            messageWriter("Point weight / Estimated Area: %g / %g\n", pointWeightSum, pointCount * pointWeightSum);
        }
        
        // Trim the tree and prepare for multigrid
        {
            constexpr int MAX_DEGREE = NORMAL_DEGREE> Degrees::Max() ? NORMAL_DEGREE : Degrees::Max();
            tree.template finalizeForMultigrid<MAX_DEGREE>(params.FullDepth, typename FEMTree<3, float>::template HasNormalDataFunctor<NormalSigs>(*normalInfo), normalInfo, density);
        }
        
        // Add the FEM constraints
        {
            constraints = tree.initDenseNodeData(Sigs());
            typename FEMIntegrator::template Constraint<Sigs, IsotropicUIntPack<3, 1>, NormalSigs, IsotropicUIntPack<3, 0>, 3> F;
            unsigned int derivatives2[3];
            for (int d = 0; d < 3; d++) { derivatives2[d] = 0; }
            
            typedef IsotropicUIntPack<3, 1> Derivatives1;
            typedef IsotropicUIntPack<3, 0> Derivatives2;
            
            for (int d = 0; d < 3; d++)
            {
                unsigned int derivatives1[3];
                for (int dd = 0; dd < 3; dd++) { derivatives1[dd] = dd == d ? 1 : 0; }
                
                F.weights[d][TensorDerivatives<Derivatives1>::Index(derivatives1)][TensorDerivatives<Derivatives2>::Index(derivatives2)] = 1;
            }
            
            tree.addFEMConstraints(F, *normalInfo, constraints, solveDepth);
        }
        
        // Free up the normal info
        delete normalInfo, normalInfo = NULL;
        
        if (progressHandler(0.3) == false) { return; }
        
        // Add the interpolation constraints
        if (params.PointWeight > 0)
        {
            iInfo = FEMTree<3, float>::template InitializeApproximatePointInterpolationInfo<float, 0>(
                                                                                                      tree,
                                                                                                      *samples,
                                                                                                      ConstraintDual(targetValue, (float)params.PointWeight * pointWeightSum),
                                                                                                      SystemDual((float)params.PointWeight * pointWeightSum),
                                                                                                      true,
                                                                                                      1);
            tree.addInterpolationConstraints(constraints, solveDepth, *iInfo);
        }
        
        messageWriter("Leaf Nodes / Active Nodes / Ghost Nodes: %d / %d / %d\n", (int)tree.leaves(), (int)tree.nodes(), (int)tree.ghostNodes());
        
        if (progressHandler(0.4) == false) { return; }
        
        // Solve the linear system
        {
            typename FEMTree<3, float>::SolverInfo sInfo;
            sInfo.cgDepth = 0;
            sInfo.cascadic = true;
            sInfo.vCycles = 1;
            sInfo.iters = params.Iters;
            sInfo.cgAccuracy = params.CGSolverAccuracy;
            sInfo.verbose = false;
            sInfo.showResidual = false;
            sInfo.showGlobalResidual = SHOW_GLOBAL_RESIDUAL_NONE;
            sInfo.sliceBlockSize = 1;
            sInfo.baseDepth = params.BaseDepth;
            sInfo.baseVCycles = params.BaseVCycles;
            
            typename FEMIntegrator::template System<Sigs, IsotropicUIntPack<3, 1>> F({ 0.0, 1.0 });
            solution = tree.solveSystem(Sigs(), F, constraints, solveDepth, sInfo, iInfo);
            if (iInfo) delete iInfo, iInfo = NULL;
        }
        
        if (progressHandler(0.5) == false) { return; }
    }
    
    {
        double valueSum = 0, weightSum = 0;
        typename FEMTree<3, float>::template MultiThreadedEvaluator<Sigs, 0> evaluator(&tree, solution);
        
        #pragma omp parallel for reduction(+ : valueSum, weightSum)
        for (int j = 0; j < samples->size(); j++)
        {
            ProjectiveData<Point<float, 3>, float>& sample = (*samples)[j].sample;
            float w = sample.weight;
            if (w > 0) {
                weightSum += w;
                valueSum += evaluator.values(sample.data / sample.weight, omp_get_thread_num(), (*samples)[j].node)[0] * w;
            }
        }
        
        isoValue = (float)(valueSum / weightSum);
        messageWriter("Iso-Value: %e = %g / %g\n", isoValue, valueSum, weightSum);
    }
    
    if (progressHandler(0.6) == false) { return; }
    
    typedef PlyVertexWithData<float, 3, MultiPointStreamData<float, PointStreamNormal<float, 3>, PointStreamValue<float>, AdditionalPointSampleData>> Vertex;
    std::function<void (Vertex&, Point<float, 3>, float, TotalPointSampleData)> SetVertex = [](Vertex& v, Point<float, 3> p, float w, TotalPointSampleData d) {
        v.point = p;
        std::get<0>(v.data.data) = std::get<0>(d.data);
        std::get<1>(v.data.data).data = w;
        std::get<2>(v.data.data) = std::get<1>(d.data);
    };
    
    ExtractMesh<Vertex>(UIntPack<FEMSigs ...>(),
                        std::tuple<SampleData ...>(),
                        tree,
                        solution,
                        isoValue,
                        samples,
                        sampleData,
                        density,
                        SetVertex,
                        messageWriter,
                        comments,
                        xForm.inverse(),
                        Out);
    
    if (sampleData) { delete sampleData; sampleData = NULL; }
    if (density) { delete density, density = NULL; }
    
    progressHandler(1);
}
