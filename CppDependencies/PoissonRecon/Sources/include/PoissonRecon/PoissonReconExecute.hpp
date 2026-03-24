//
//  PoissonReconExecute.hpp
//  PoissonRecon
//
//  Created by Aaron Thompson on 5/8/18.
//

#include "PreProcessor.h"
#include "Reconstructors.h"
#include "DataStream.imp.h"
#include "VertexFactory.h"

using namespace PoissonRecon;

// Adapter: reads a PLY file with position+normal+color and presents as InputOrientedSampleStream
struct PLYOrientedColorStream : public Reconstructor::InputOrientedSampleStream<float, 3, Point<float, 3>>
{
    using Factory = VertexFactory::Factory<float,
        VertexFactory::PositionFactory<float, 3>,
        VertexFactory::NormalFactory<float, 3>,
        VertexFactory::RGBColorFactory<float>>;
    using Vertex = typename Factory::VertexType;

    PLYOrientedColorStream(const char* fileName) : _stream(fileName, Factory()) {}
    void reset() { _stream.reset(); }

    bool read(Point<float, 3>& p, Point<float, 3>& n, Point<float, 3>& color)
    {
        Vertex v;
        if (!_stream.read(v)) return false;
        p = v.template get<0>();
        n = v.template get<1>();
        color = v.template get<2>();
        return true;
    }

    bool read(unsigned int, Point<float, 3>& p, Point<float, 3>& n, Point<float, 3>& color)
    {
        return read(p, n, color);
    }

private:
    PLYInputDataStream<Factory> _stream;
};

// Collects level-set vertices from extractLevelSet
struct VertexCollector : public Reconstructor::OutputLevelSetVertexStream<float, 3, Point<float, 3>>
{
    struct OutputVertex {
        Point<float, 3> position;
        Point<float, 3> gradient;
        float weight;
        Point<float, 3> color;
    };

    std::vector<OutputVertex> vertices;

    size_t write(const Point<float, 3>& p, const Point<float, 3>& g, const float& w, const Point<float, 3>& c)
    {
        vertices.push_back({p, g, w, c});
        return vertices.size() - 1;
    }

    size_t write(unsigned int, const Point<float, 3>& p, const Point<float, 3>& g, const float& w, const Point<float, 3>& c)
    {
        return write(p, g, w, c);
    }

    size_t size() const { return vertices.size(); }
};

// Collects faces from extractLevelSet
struct FaceCollector : public Reconstructor::OutputFaceStream<2>
{
    std::vector<std::vector<node_index_type>> faces;

    size_t write(const std::vector<node_index_type>& f)
    {
        faces.push_back(f);
        return faces.size() - 1;
    }

    size_t write(unsigned int, const std::vector<node_index_type>& f)
    {
        return write(f);
    }

    size_t size() const { return faces.size(); }
};

template<unsigned int ... FEMSigs>
void _PoissonReconExecute(const char *In,
                          const char *Out,
                          PoissonReconParameters params,
                          std::function<bool (float)> progressHandler,
                          UIntPack<FEMSigs ...>)
{
    using namespace Reconstructor;
    typedef Point<float, 3> ColorType;
    ColorType zeroColor;

    if (progressHandler(0) == false) { return; }

    // Create the input point stream from PLY
    PLYOrientedColorStream pointStream(In);

    // Set up solver parameters
    Poisson::SolutionParameters<float> solverParams;
    solverParams.depth = params.Depth;
    solverParams.baseDepth = params.BaseDepth;
    solverParams.baseVCycles = params.BaseVCycles;
    solverParams.cgSolverAccuracy = params.CGSolverAccuracy;
    solverParams.fullDepth = params.FullDepth;
    solverParams.iters = params.Iters;
    solverParams.pointWeight = params.PointWeight;
    solverParams.samplesPerNode = params.SamplesPerNode;
    solverParams.scale = params.Scale;
    solverParams.verbose = false;

    if (progressHandler(0.2) == false) { return; }

    // Solve the Poisson reconstruction
    auto *implicit = Poisson::Solver<float, 3, UIntPack<FEMSigs...>, ColorType>::Solve(
        pointStream, solverParams, zeroColor);

    if (!implicit) { return; }

    if (progressHandler(0.5) == false) { delete implicit; return; }

    // Extract the level set (mesh)
    LevelSetExtractionParameters extractionParams;
    extractionParams.linearFit = false;
    extractionParams.outputGradients = true;
    extractionParams.forceManifold = true;
    extractionParams.polygonMesh = false;
    extractionParams.outputDensity = true;

    VertexCollector vertexCollector;
    FaceCollector faceCollector;
    implicit->extractLevelSet(vertexCollector, faceCollector, extractionParams);

    if (progressHandler(0.8) == false) { delete implicit; return; }

    // Write the output PLY file
    // Output vertex format: position, value (density), normal, color
    using OutputFactory = VertexFactory::Factory<float,
        VertexFactory::PositionFactory<float, 3>,
        VertexFactory::ValueFactory<float>,
        VertexFactory::NormalFactory<float, 3>,
        VertexFactory::RGBColorFactory<float>>;
    using OutputVertex = typename OutputFactory::VertexType;

    std::vector<OutputVertex> plyVertices(vertexCollector.vertices.size());
    for (size_t i = 0; i < vertexCollector.vertices.size(); i++) {
        auto& v = vertexCollector.vertices[i];
        plyVertices[i].template get<0>() = v.position;
        plyVertices[i].template get<1>() = v.weight;
        plyVertices[i].template get<2>() = v.gradient;
        plyVertices[i].template get<3>() = v.color;
    }

    std::vector<std::vector<int>> plyFaces(faceCollector.faces.size());
    for (size_t i = 0; i < faceCollector.faces.size(); i++) {
        plyFaces[i].resize(faceCollector.faces[i].size());
        for (size_t j = 0; j < faceCollector.faces[i].size(); j++) {
            plyFaces[i][j] = (int)faceCollector.faces[i][j];
        }
    }

    std::vector<std::string> comments;
    PLY::WritePolygons<OutputFactory, int>(Out, OutputFactory(), plyVertices, plyFaces, PLY::DefaultFileType(), comments);

    delete implicit;

    progressHandler(1);
}
