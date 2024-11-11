//
//  HeadGlobalRegistration.cpp
//  StandardCyborgFusion
//
//  Created by eric on 2019-08-06.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//


#import "MeshUvMap.hpp"

#include "standard_cyborg/algorithms/Centroid.hpp"
#include "standard_cyborg/algorithms/PointToPointAlignment.hpp"
#include "standard_cyborg/algorithms/EdgeLoopFinder.hpp"
#include "standard_cyborg/algorithms/MeshSplitter.hpp"
#include "standard_cyborg/sc3d/MeshTopology.hpp"
#include "standard_cyborg/sc3d/BoundingBox3.hpp"
#include "standard_cyborg/util/AssertHelper.hpp"

#import "maxrects.hpp"

#include <fstream>
#include <queue>
#include <map>
#include <stack>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#pragma clang diagnostic ignored "-Wuninitialized"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma clang diagnostic ignored "-Wconditional-uninitialized"
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#include <igl/boundary_loop.h>
#include <igl/lscm.h>
#include <igl/arap.h>
#include <igl/map_vertices_to_circle.h>
#include <igl/harmonic.h>
#pragma clang diagnostic pop

using standard_cyborg::sc3d::Geometry;
using standard_cyborg::sc3d::Face3;

using standard_cyborg::math::Vec3;
using standard_cyborg::math::Vec2;

namespace sc3d = standard_cyborg::sc3d;

namespace standard_cyborg {

namespace algorithms {

std::string errorMessage = "";

static void assertHandler(const char* x, const char* prettyFunction, const char* file, int line)
{
    char buf[1000];
    snprintf(buf, sizeof(buf), "My custom assertion failed: (%s), function %s, file %s, line %d.\n",
             x, prettyFunction, file, line);
    printf("lol %s", buf);
    errorMessage = buf;
}

#define STR(x) #x

#define MY_ASSERT(x) (!(x)) ? assertHandler(STR(x), __PRETTY_FUNCTION__, __FILE__, __LINE__), true : false

//#define MY_ASSERT(x) false

//#define UVMAP_MESH_LOG

std::string getUvmapMeshErrorMessage()
{
    return errorMessage;
}

template <class T, class S, class C>
static S& getPriorityQueueContainer(std::priority_queue<T, S, C>& q)
{
    struct HackedQueue : private std::priority_queue<T, S, C> {
        static S& Container(std::priority_queue<T, S, C>& q)
        {
            return q.*&HackedQueue::c;
        }
    };
    return HackedQueue::Container(q);
}

// all faces, directly adjacent to the three edges of the face.
std::vector<int> getDirectlyAdjacentFaces(int iCurFace,
                                          const std::vector<sc3d::MeshTopology::Edge>& topologyEdges,
                                          const std::vector<sc3d::MeshTopology::FaceEdges>& topologyFaceEdges)
{
    const sc3d::MeshTopology::FaceEdges& faceEdges = topologyFaceEdges[iCurFace];

    std::vector<int> adjacentFaces;

    for (int iEdge = 0; iEdge < 3; ++iEdge) {

        sc3d::MeshTopology::Edge edge = topologyEdges[faceEdges[iEdge]];

        int iFace0 = edge.face0;
        int iFace1 = edge.face1;

        if (MY_ASSERT(iFace0 == iCurFace || iFace1 == iCurFace)) {
            return std::vector<int>();
        }

        if ((iFace0 == iCurFace && iFace1 == -1) || (iFace0 == -1 && iFace1 == iCurFace)) {
            continue; // no adjacent edge here.
        }

        int iAdjacentFace;

        if (iFace0 == iCurFace) {
            iAdjacentFace = iFace1;
        } else {
            iAdjacentFace = iFace0;
        }

        if (MY_ASSERT(iAdjacentFace != iCurFace && iAdjacentFace != -1)) {
            return std::vector<int>();
        }

        adjacentFaces.push_back(iAdjacentFace);
    }

    return adjacentFaces;
}


/*
     Segment mesh. Each std::set<int> contains all the face
     indices of a segment.
     
     Based on the cutting algorithm from the paper
     "Evaluating 3D Thumbnails for Virtual Object Galleries"
     
     Also using some parts from:
     "Least Squares Conformal Maps for Automatic Texture Atlas Generation"
     */
std::vector<std::set<int>> segmentMeshNew(Geometry& geometry)
{
    const std::vector<Vec3>& positions = geometry.getPositions();
    const std::vector<Face3>& faces = geometry.getFaces();

    sc3d::MeshTopology::MeshTopology topology(geometry.getFaces());
    const std::vector<sc3d::MeshTopology::Edge>& topologyEdges = topology.getEdges();
    const std::vector<sc3d::MeshTopology::FaceEdges>& topologyFaceEdges = topology.getFaceEdges();
    const std::vector<sc3d::MeshTopology::VertexEdges>& vertexEdges = topology.getVertexEdges();

    srand(0); // make sure its deterministic.

    auto calcNormal = [&positions](Face3 face) {
        Vec3 p0 = positions[face[0]];
        Vec3 p1 = positions[face[1]];
        Vec3 p2 = positions[face[2]];

        Vec3 n = Vec3::normalize(Vec3::cross(p1 - p0, p2 - p0));

        return n;
    };

    auto calcCentroid = [&positions](Face3 face) {
        Vec3 p0 = positions[face[0]];
        Vec3 p1 = positions[face[1]];
        Vec3 p2 = positions[face[2]];

        Vec3 n = (1.0 / 3.0f) * (p0 + p1 + p2);

        return n;
    };

    std::vector<float> dist(faces.size(), std::numeric_limits<float>::infinity());
    std::vector<int> prev(faces.size(), -1); // -1 is UNDEFINED vertex.

    {
        std::set<int> borderAndFeatureFaces;

        struct EdgeEntry {
            int iEdge;
            float dihedralDotProduct;
        };

        std::vector<EdgeEntry> edgeAngles;

        // calculate sharpness of all edges.
        for (int iEdge = 0; iEdge < topologyEdges.size(); ++iEdge) {
            sc3d::MeshTopology::Edge edge = topologyEdges[iEdge];

            const int iFace0 = edge.face0;
            const int iFace1 = edge.face1;

            if (MY_ASSERT(!(iFace0 == -1 && iFace1 == -1))) {
                return std::vector<std::set<int>>();
            }

            // handle borders.
            if (iFace0 == -1 || iFace1 == -1) {

                int iBorderFace;
                if (iFace0 == -1) {
                    iBorderFace = iFace1;
                } else {
                    iBorderFace = iFace0;
                }

                if (MY_ASSERT(iBorderFace != -1)) {
                    return std::vector<std::set<int>>();
                }

                if (borderAndFeatureFaces.count(iBorderFace) != 0) {
                } else {
                    borderAndFeatureFaces.insert(iBorderFace);
                }

                continue;
            }

            if (MY_ASSERT((iFace0 != -1 && iFace1 != -1))) {
                return std::vector<std::set<int>>();
            }

            Face3 face0 = faces[iFace0];
            Face3 face1 = faces[iFace1];

            if (MY_ASSERT(!(face0[0] == face0[1] && face0[1] == face0[2]))) {
                return std::vector<std::set<int>>();
            }

            if (MY_ASSERT(!(face1[0] == face1[1] && face1[1] == face1[2]))) {
                return std::vector<std::set<int>>();
            }

            Vec3 faceNormal0 = Vec3::normalize(calcNormal(face0));
            Vec3 faceNormal1 = Vec3::normalize(calcNormal(face1));

            if (MY_ASSERT(!std::isnan(faceNormal0.x))) {
                return std::vector<std::set<int>>();
            }

            if (MY_ASSERT(!std::isnan(faceNormal1.x))) {
                return std::vector<std::set<int>>();
            }

            EdgeEntry entry;

            entry.dihedralDotProduct = Vec3::dot(faceNormal0, faceNormal1);
            entry.iEdge = iEdge;

            float eps = 0.00001f;


            if (MY_ASSERT(!(entry.dihedralDotProduct < -(1.0 + eps) || entry.dihedralDotProduct > (+1.0 + eps)))) {
                return std::vector<std::set<int>>();
            }

            edgeAngles.push_back(entry);
        }


        auto comparator = [](const EdgeEntry& e0, const EdgeEntry& e1) {
            return e0.dihedralDotProduct < e1.dihedralDotProduct;
        };

        std::sort(edgeAngles.begin(), edgeAngles.end(), comparator);

        // top 5% sharpest of all edges, are the feature edges.
        int numFeatures = edgeAngles.size() * 0.005;

        if (numFeatures == 0) {
            numFeatures = 2;
        }

        if (MY_ASSERT(edgeAngles.size() > 2)) {
            return std::vector<std::set<int>>();
        }

        // get all features edges, and border edges into one set.
        for (int iEdge = 0; iEdge < numFeatures; ++iEdge) {
            sc3d::MeshTopology::Edge edge = topologyEdges[edgeAngles[iEdge].iEdge];

            if (borderAndFeatureFaces.count(edge.face0) == 0) {
                borderAndFeatureFaces.insert(edge.face0);
            } else if ((borderAndFeatureFaces.count(edge.face1) == 0)) {

                borderAndFeatureFaces.insert(edge.face1);
            }
        }

        struct QueueEntry {
            int iFace;
            float dist;
        };

        auto queueComparator = [](const QueueEntry& e1, const QueueEntry& e2) {
            return e1.dist > e2.dist;
        };

        std::priority_queue<QueueEntry, std::vector<QueueEntry>, decltype(queueComparator)> Q(queueComparator);

        {
            std::set<int> sourceFaces = borderAndFeatureFaces; // we have several source vertices.

            for (int iSourceFace : sourceFaces) {
                dist[iSourceFace] = 0.0f;

                QueueEntry entry{iSourceFace, dist[iSourceFace]};
                Q.push(entry);
            }
        }

        std::set<int> visitedFaces;


        // run dijsktras algortihm, to get the closest distance to each feature/border face
        // for every face in the mesh.
        while (!Q.empty()) {
            QueueEntry entry = Q.top();
            Q.pop();

            {

                int iCurFace = entry.iFace;
                std::vector<int> adjacentFaces = getDirectlyAdjacentFaces(iCurFace, topologyEdges, topologyFaceEdges);

                if (MY_ASSERT(adjacentFaces.size() != 0)) {
                    return std::vector<std::set<int>>();
                }

                if (MY_ASSERT(adjacentFaces.size() >= 0 && adjacentFaces.size() <= 3)) {
                    return std::vector<std::set<int>>();
                }

                for (int iAdjacentFace : adjacentFaces) {

                    if (visitedFaces.count(iAdjacentFace) != 0) {
                        continue;
                    }
                    visitedFaces.insert(iAdjacentFace);

                    Face3 curFace = faces[iCurFace];

                    Face3 adjacentFace = faces[iAdjacentFace];


                    Vec3 curFaceCentroid = calcCentroid(curFace);
                    Vec3 adjacentFaceCentroid = calcCentroid(adjacentFace);

                    if (MY_ASSERT(!std::isnan(curFaceCentroid.x))) {
                        return std::vector<std::set<int>>();
                    }
                    if (MY_ASSERT(!std::isnan(adjacentFaceCentroid.x))) {
                        return std::vector<std::set<int>>();
                    }

                    const float len = (curFaceCentroid - adjacentFaceCentroid).norm();

                    float newDist = dist[iCurFace] + len;

                    if (newDist < dist[iAdjacentFace]) {
                        dist[iAdjacentFace] = newDist;
                        prev[iAdjacentFace] = iCurFace;
                    }

                    QueueEntry entry{iAdjacentFace, dist[iAdjacentFace]};
                    Q.push(entry);
                }
            }
        }

        if (MY_ASSERT(visitedFaces.size() == faces.size())) { // have to visit all faces.
            return std::vector<std::set<int>>();
        }
    }

    std::set<int> adjacentFacesSetTemp;

    auto getVertexAdjacentFaces = [&adjacentFacesSetTemp](
                                      int iCurFace,
                                      const std::vector<sc3d::MeshTopology::Edge>& topologyEdges,
                                      const std::vector<sc3d::MeshTopology::FaceEdges>& topologyFaceEdges,
                                      const std::vector<sc3d::MeshTopology::VertexEdges>& vertexEdges,
                                      const std::vector<Face3>& faces) {
        adjacentFacesSetTemp.clear();
        for (int iVertex = 0; iVertex < 3; ++iVertex) {

            sc3d::MeshTopology::VertexEdges edges;
            edges = vertexEdges[faces[iCurFace][iVertex]];

            for (int iEdge : edges) {

                sc3d::MeshTopology::Edge edge = topologyEdges[iEdge];

                int iFace0 = edge.face0;
                int iFace1 = edge.face1;


                if (iFace0 != iCurFace && iFace0 != -1) {


                    adjacentFacesSetTemp.insert(iFace0);
                }

                if (iFace1 != iCurFace && iFace1 != -1) {


                    adjacentFacesSetTemp.insert(iFace1);
                }
            }
        }

        if (MY_ASSERT(adjacentFacesSetTemp.count(iCurFace) == 0)) {
            return std::vector<int>();
        }

        std::vector<int> adjacentFaces;

        for (int iFace : adjacentFacesSetTemp) {
            adjacentFaces.push_back(iFace);
        }

        return adjacentFaces;
    };

    std::set<int> seedFaces;

    // the local maximas of dist[](gotten from the dijsktras algorithm previously),
    // is used as seeds for the region/segment growing.
    for (int iCurFace = 0; iCurFace < faces.size(); ++iCurFace) {

        if (dist[iCurFace] == 0.0f) { // border edge. skip.
            continue;
        }

        std::vector<int> adjacentFaces = getVertexAdjacentFaces(iCurFace, topologyEdges, topologyFaceEdges, vertexEdges, faces);

        if (MY_ASSERT(adjacentFaces.size() != 0)) {
            return std::vector<std::set<int>>();
        }

        float curDist = dist[iCurFace];

        bool biggerThanAll = true;

        int count = 0;
        for (int iFace : adjacentFaces) {

            if (MY_ASSERT(iFace >= 0 && iFace < faces.size())) {
                return std::vector<std::set<int>>();
            }


            float dist0 = dist[iFace];

            if (dist0 == 0.0f) {
                biggerThanAll = false;
                break;
            }

            if (curDist < dist0) {
            } else {
                count++;
            }
        }

        if (count != adjacentFaces.size()) {
            biggerThanAll = false;
        }


        if (biggerThanAll) {
            if (MY_ASSERT(seedFaces.count(iCurFace) == 0)) {
                return std::vector<std::set<int>>();
            }

            seedFaces.insert(iCurFace);
        }
    }

    if (seedFaces.size() <= 1) { // zero or one seeds is of course not an acceptable segmentation.
        seedFaces.clear();
        // welp. couldnt find any good local maximas. this often seen to happen for very small meshes,
        // (like with 8 triangle or something). just grab two faces then:

        if (MY_ASSERT(faces.size() > 2)) {
            return std::vector<std::set<int>>();
        }

        seedFaces.insert(0); // just pick face 0.

        // now pick any other face, that is not face 0, and not adjacent to it:
        std::vector<int> adjacents = getDirectlyAdjacentFaces(0, topologyEdges, topologyFaceEdges);
        std::set<int> adjacentsSet(adjacents.begin(), adjacents.end());
        for (int iFace = 1; iFace < faces.size(); ++iFace) {
            if (adjacentsSet.count(iFace) == 0) {
                seedFaces.insert(iFace);


                break;
            }
        }

        if (seedFaces.size() == 1 && faces.size() == 4) {
            // tetrahedron case:
            return std::vector<std::set<int>>{{0}, {1, 2, 3}};
        } else if (MY_ASSERT(seedFaces.size() == 2)) {
            return std::vector<std::set<int>>();
        }
    }

    std::vector<std::set<int>> segmentations;
    // region growing algorithm.
    // this inspired from algorithm 2 of
    // "Least Squares Conformal Maps for Automatic Texture Atlas Generation"
    {
        // filter out seeds that are adjacent to another seed face.
        {
            std::set<int> newSeedFaces;

            for (int iSeedFace : seedFaces) {


                std::vector<int> adjacents = getDirectlyAdjacentFaces(iSeedFace, topologyEdges, topologyFaceEdges);

                bool keep = true;

                for (int iAdjacent : adjacents) {

                    if (newSeedFaces.count(iAdjacent) != 0) {
                        // adjacent to already added one. dont keep.
                        keep = false;
                        break;
                    }
                }

                newSeedFaces.insert(iSeedFace);
            }
            seedFaces = newSeedFaces;
        }

        float mergeEpsilon = (float)(*std::max_element(dist.begin(), dist.end())) / 6.0f;

        struct QueueEntry {
            int iFace;
            float dist;
            int iRegion;
        };

        std::set<int> visitedFaces;

        auto queueComparator = [](const QueueEntry& e1, const QueueEntry& e2) {
            return e1.dist < e2.dist;
        };

        std::priority_queue<QueueEntry, std::vector<QueueEntry>, decltype(queueComparator)> Q(queueComparator);

        // face index -> assigned region.
        std::vector<int> faceToRegion(faces.size(), -1);

        // init Q.
        std::vector<float> maxDist(seedFaces.size(), 0.0f);
        {
            for (int iSeedFace : seedFaces) {
                int iRegion = (int)segmentations.size();

                segmentations.push_back(std::set<int>{iSeedFace});

                faceToRegion[iSeedFace] = iRegion;
                maxDist[iRegion] = dist[iSeedFace];


                for (const int iAdjacentFace : getDirectlyAdjacentFaces(iSeedFace, topologyEdges, topologyFaceEdges)) {
                    QueueEntry entry{iAdjacentFace, dist[iAdjacentFace], iRegion};
                    Q.push(entry);
                }

                visitedFaces.insert(iSeedFace);
            }

            if (MY_ASSERT(seedFaces.size() == segmentations.size())) {
                return std::vector<std::set<int>>();
            }
        }

        int numRegions = (int)seedFaces.size();


        while (!Q.empty()) {
            QueueEntry entry = Q.top();
            Q.pop();

            if (visitedFaces.count(entry.iFace) != 0) {
                continue;
            }
            visitedFaces.insert(entry.iFace);

            segmentations[entry.iRegion].insert(entry.iFace);
            maxDist[entry.iRegion] = std::max(maxDist[entry.iRegion], entry.dist);
            faceToRegion[entry.iFace] = entry.iRegion;


            for (int iAdjacentFace : getDirectlyAdjacentFaces(entry.iFace, topologyEdges, topologyFaceEdges)) {

                if (faceToRegion[iAdjacentFace] != -1 && faceToRegion[iAdjacentFace] != entry.iRegion) {
                    float distF = dist[entry.iFace];

                    float maxDistChartF = maxDist[entry.iRegion];
                    float maxDistChartFOpp = maxDist[faceToRegion[iAdjacentFace]];

                    if (numRegions > 20 && abs(maxDistChartF - distF) < mergeEpsilon && abs(maxDistChartFOpp - distF) < mergeEpsilon) {


                        int replaceRegion = faceToRegion[iAdjacentFace];
                        int sourceRegion = entry.iRegion;

#ifdef UVMAP_MESH_LOG
                        printf("can merge %d %d!\n", replaceRegion, sourceRegion);
#endif
                        
                        int expectedSize = (int)segmentations[replaceRegion].size() + (int)segmentations[sourceRegion].size();

                        // update segmentation.
                        for (int iFace : segmentations[replaceRegion]) {
                            segmentations[sourceRegion].insert(iFace);

                            faceToRegion[iFace] = replaceRegion;
                        }

                        if (MY_ASSERT(segmentations[sourceRegion].size() == expectedSize)) {
                            return std::vector<std::set<int>>();
                        }

                        // update queue.
                        {
                            std::vector<QueueEntry>& QContainer = getPriorityQueueContainer(Q);
                            for (QueueEntry& targetEntry : QContainer) {
                                if (targetEntry.iRegion == replaceRegion) {
                                    targetEntry.iRegion = sourceRegion;
                                }
                            }
                        }

                        // update maxDists.
                        maxDist[sourceRegion] = std::max(maxDist[sourceRegion], maxDist[replaceRegion]);
                        maxDist[replaceRegion] = std::nan("lol");

                        segmentations[replaceRegion].clear();

                        --numRegions;
                    }
                } else {
                    QueueEntry newEntry{iAdjacentFace, dist[iAdjacentFace], entry.iRegion};
                    Q.push(newEntry);
                }
            }
        }

        if (MY_ASSERT(visitedFaces.size() == faces.size())) {
            return std::vector<std::set<int>>();
        }

        // merge small segments, that are completely contained within ONE larger segment.
        {
            for (int iSegment = 0; iSegment < segmentations.size(); ++iSegment) {

                if (numRegions == 2) {
                    // we must not merge beyond this point.
                    break;
                }

                if (segmentations[iSegment].size() == 0) {
                    continue; // already merged this segment.
                }

                int iSurroundRegion = -1;
                bool isSurrounded = true;

                for (const int iFace : segmentations[iSegment]) {

                    for (const int iAdjacentFace : getDirectlyAdjacentFaces(iFace, topologyEdges, topologyFaceEdges)) {

                        const int iAdjacentRegion = faceToRegion[iAdjacentFace];

                        if (iAdjacentRegion == iSegment) {
                            // we only care about the neighbour faces of the region, that
                            // are in another, different region.
                            continue;
                        }

                        if (iSurroundRegion == -1) {
                            // ok. lets make the guess that this region surrounds iRegion
                            iSurroundRegion = iAdjacentRegion;
                        } else if (iSurroundRegion != iAdjacentRegion) {
                            // welp. guess failed. this region is not entirely surronded by one region.
                            isSurrounded = false;
                            break;
                        }
                    }

                    if (isSurrounded == false) {
                        break;
                    }
                }

                if (isSurrounded && iSurroundRegion != -1) {
#ifdef UVMAP_MESH_LOG
                    printf("can merge surronded %d %d\n", iSegment, iSurroundRegion);
#endif
                    
                    int replaceRegion = iSegment;
                    int sourceRegion = iSurroundRegion;

                    if (MY_ASSERT(segmentations[replaceRegion].size() != 0)) {
                        return std::vector<std::set<int>>();
                    }

                    if (MY_ASSERT(segmentations[sourceRegion].size() != 0)) {
                        return std::vector<std::set<int>>();
                    }

                    int expectedSize = (int)segmentations[replaceRegion].size() + (int)segmentations[sourceRegion].size();

                    // update segmentation.
                    for (int iFace : segmentations[replaceRegion]) {
                        segmentations[sourceRegion].insert(iFace);

                        faceToRegion[iFace] = replaceRegion;
                    }

                    if (MY_ASSERT(segmentations[sourceRegion].size() == expectedSize)) {
                        return std::vector<std::set<int>>();
                    }

                    segmentations[replaceRegion].clear();

                    --numRegions;
                }
            }
        }

        // do sanity check, to verify that all faces are part of the segmentation
        // and that no faces were accidentally never added to a segment.
        {
            int totalSegmentedFaces = 0;
            std::set<int> all;
            for (int isegment = 0; isegment < segmentations.size(); ++isegment) {

                totalSegmentedFaces += segmentations[isegment].size();

                for (int index : segmentations[isegment]) {
                    all.insert(index);
                }
            }

            if (MY_ASSERT((totalSegmentedFaces == geometry.faceCount()) && (all.size() == geometry.faceCount()))) {
                return std::vector<std::set<int>>();
            }
        }
    }

    return segmentations;
}


/*
     Just dijsktras algorithm, for finding shortest path from sourceSet to targetSet.
     */
std::vector<int> findCutEdges(const std::set<int>& sourceSet, const std::set<int>& targetSet, const std::vector<Face3>& geoFaces, const int numVertices, bool& foundAPath)
{
    foundAPath = true;

    sc3d::MeshTopology::MeshTopology topology(geoFaces);
    const std::vector<sc3d::MeshTopology::Edge>& meshTopologyEdges = topology.getEdges();
    const std::vector<sc3d::MeshTopology::VertexEdges>& meshTopologyVertexEdges = topology.getVertexEdges();

    struct QueueEntry {
        int iVertex;
        float dist;
    };

    auto queueComparator = [](const QueueEntry& e1, const QueueEntry& e2) {
        return e1.dist > e2.dist;
    };

    std::vector<float> dist(numVertices, std::numeric_limits<float>::infinity());
    std::vector<int> prev(numVertices, -1); // -1 is UNDEFINED vertex.

    std::priority_queue<QueueEntry, std::vector<QueueEntry>, decltype(queueComparator)> Q(queueComparator);

    for (const int iSourceVertex : sourceSet) {
        dist[iSourceVertex] = 0.0f;
        QueueEntry entry{iSourceVertex, dist[iSourceVertex]};
        Q.push(entry);
    }

    std::set<int> visitedVertices;

    while (!Q.empty()) {
        QueueEntry entry = Q.top();
        Q.pop();

        {
            const int iCurVertex = entry.iVertex;

            std::vector<int> adjacentVertices;
            {
                const std::set<int>& adjacentVertexEdges = meshTopologyVertexEdges[iCurVertex];

                for (const int iAdjacentVertexEdge : adjacentVertexEdges) {
                    sc3d::MeshTopology::Edge adjacentEdge = meshTopologyEdges[iAdjacentVertexEdge];

                    if (MY_ASSERT(adjacentEdge.vertex0 == iCurVertex || adjacentEdge.vertex1 == iCurVertex)) {
                        return std::vector<int>();
                    }

                    if (adjacentEdge.vertex0 == iCurVertex) {
                        adjacentVertices.push_back(adjacentEdge.vertex1);
                    } else {
                        adjacentVertices.push_back(adjacentEdge.vertex0);
                    }
                }
            }

            if (MY_ASSERT(adjacentVertices.size() > 0)) {
                return std::vector<int>();
            }

            for (const int iAdjacentVertex : adjacentVertices) {

                if (visitedVertices.count(iAdjacentVertex) == 1) {
                    continue;
                }
                visitedVertices.insert(iAdjacentVertex);

                float newDist = dist[iCurVertex] + 1.0f;

                if (newDist < dist[iAdjacentVertex]) {
                    dist[iAdjacentVertex] = newDist;
                    prev[iAdjacentVertex] = iCurVertex;
                }

                QueueEntry entry{iAdjacentVertex, dist[iAdjacentVertex]};
                Q.push(entry);
            }
        }
    }

    int iMinTarget = -1;
    float minDist = std::numeric_limits<float>::infinity();

    {

        for (int iTarget : targetSet) {

            if (dist[iTarget] < minDist) {
                iMinTarget = iTarget;
                minDist = dist[iTarget];
            }
        }
    }

    if (iMinTarget == -1) {
        foundAPath = false;
        return std::vector<int>();
    } else {
    }

    if (MY_ASSERT(prev[iMinTarget] != -1)) {
        return std::vector<int>();
    }

    if (MY_ASSERT(sourceSet.count(iMinTarget) == 0)) {
        return std::vector<int>();
    }

    // now reconstruct the shortest path
    std::vector<int> reconstructedPath;
    {
        int u = iMinTarget;
        do {
            reconstructedPath.push_back(u);
            u = prev[u];
        } while (u != -1);
    }


    if (MY_ASSERT(targetSet.count(reconstructedPath[0]) == 1)) {
        return std::vector<int>();
    }

    if (MY_ASSERT(sourceSet.count(reconstructedPath[reconstructedPath.size() - 1]) == 1)) {
        return std::vector<int>();
    }

    foundAPath = true;


    return reconstructedPath;
}

bool splitSharedVertexIntoTwo(Geometry& geometry, int iSharedVertex)
{
    const std::vector<Face3>& geoFaces = geometry.getFaces();
    const std::vector<Vec3>& geoPositions = geometry.getPositions();
    const std::vector<Vec3>& geoColors = geometry.getColors();
    const std::vector<Vec3>& geoNormals = geometry.getNormals();

    const sc3d::MeshTopology::MeshTopology topology(geoFaces);
    const std::vector<sc3d::MeshTopology::Edge>& topologyEdges = topology.getEdges();
    const std::vector<sc3d::MeshTopology::FaceEdges>& topologyFaceEdges = topology.getFaceEdges();
    const std::vector<sc3d::MeshTopology::VertexEdges>& vertexEdges = topology.getVertexEdges();

    // these faces are on one half of the seamCut
    std::set<int> firstHalfFaceSet;
    // these faces are on the other seamCut.
    std::set<int> secondHalfFaceSet;

    // all faces, adjacent to specified vertex.
    auto getVertexAdjacentFaces = [](
                                      int iVertex,
                                      const std::vector<sc3d::MeshTopology::Edge>& topologyEdges,
                                      const std::vector<sc3d::MeshTopology::VertexEdges>& vertexEdges) {
        std::set<int> adjacentFacesSet;

        sc3d::MeshTopology::VertexEdges edges;
        edges = vertexEdges[iVertex];

        for (int iEdge : edges) {
            sc3d::MeshTopology::Edge edge = topologyEdges[iEdge];

            int iFace0 = edge.face0;
            int iFace1 = edge.face1;

            if (iFace0 != -1)
                adjacentFacesSet.insert(iFace0);

            if (iFace1 != -1)
                adjacentFacesSet.insert(iFace1);
        }

        return adjacentFacesSet;
    };

    // all faces, which will be divided into two sets.
    std::set<int> allFaces = getVertexAdjacentFaces(iSharedVertex, topologyEdges, vertexEdges);

    std::set<int> visitedFaces;

    std::queue<int> Q;

    if (MY_ASSERT(allFaces.size() != 0)) {
        return false;
    }

    Q.push(*allFaces.begin());

    while (!Q.empty()) {
        int iCurFace = Q.front();
        Q.pop();

        {
            firstHalfFaceSet.insert(iCurFace);

            std::vector<int> adjacentFaces = getDirectlyAdjacentFaces(iCurFace, topologyEdges, topologyFaceEdges);

            if (MY_ASSERT(adjacentFaces.size() != 0)) {
                return false;
            }


            if (MY_ASSERT(adjacentFaces.size() >= 0 && adjacentFaces.size() <= 3)) {
                return false;
            }

            for (int iAdjacentFace : adjacentFaces) {

                if (
                    visitedFaces.count(iAdjacentFace) == 1 || // already visited
                    allFaces.count(iAdjacentFace) == 0) { // or not considered.
                    continue;
                }
                visitedFaces.insert(iAdjacentFace);


                Q.push(iAdjacentFace);
            }
        }
    }

    // starting from iStartVertexEdge, iterate around iCenterVertex
    // classifying faces into one of the two halves.
    for (int iFace : allFaces) {
        if (firstHalfFaceSet.count(iFace) == 0) {
            secondHalfFaceSet.insert(iFace);
        }
    }

    if (MY_ASSERT(firstHalfFaceSet.size() + secondHalfFaceSet.size() == allFaces.size())) {
        return false;
    }

    // good. Now we've divided the faces into two sets.
    // now we can make the cut.

    std::vector<Vec3> newPositions = geoPositions;
    std::vector<Vec3> newNormals = geoNormals;
    std::vector<Vec3> newColors = geoColors;
    std::vector<Face3> newFaces = geoFaces;

    std::map<int, int> indexRemapping;

    // for one half, create new vertices.
    {
        int iCutVertex = iSharedVertex;

        Vec3 sourcePosition = newPositions[iCutVertex];
        int iOld = iCutVertex;
        int iNew = (int)newPositions.size();
        indexRemapping[iOld] = iNew;

        newPositions.push_back(sourcePosition);

        if (geometry.hasNormals()) {
            Vec3 sourceNormal = newNormals[iCutVertex];
            newNormals.push_back(sourceNormal);
        }

        if (geometry.hasColors()) {
            Vec3 sourceColor = newColors[iCutVertex];
            newColors.push_back(sourceColor);
        }
    }

    for (int iFace = 0; iFace < newFaces.size(); ++iFace) {
        if (secondHalfFaceSet.count(iFace) != 1) {
            // this half uses the old vertices. so skip
            continue;
        }

        Face3& newFace = newFaces[iFace];

        for (int iCorner = 0; iCorner < 3; ++iCorner) {

            if (newFace[iCorner] != iSharedVertex) {
                continue;
            }

            if (MY_ASSERT(indexRemapping.count(newFace[iCorner]) != 0)) {
                return false;
            }

            newFace[iCorner] = indexRemapping[newFace[iCorner]];
        }
    }

    Geometry newGeo(newPositions, newNormals, newColors, newFaces);
    geometry.copy(newGeo);

    return true;
}

bool seamCutMesh(Geometry& geometry, const std::vector<int>& seamCut)
{
    const std::vector<Face3>& geoFaces = geometry.getFaces();
    const std::vector<Vec3>& geoPositions = geometry.getPositions();
    const std::vector<Vec3>& geoColors = geometry.getColors();
    const std::vector<Vec3>& geoNormals = geometry.getNormals();

    const sc3d::MeshTopology::MeshTopology topology(geoFaces);
    const std::vector<sc3d::MeshTopology::Edge>& topologyEdges = topology.getEdges();
    const std::vector<sc3d::MeshTopology::FaceEdges>& topologyFaceEdges = topology.getFaceEdges();
    const std::vector<sc3d::MeshTopology::VertexEdges>& vertexEdges = topology.getVertexEdges();

    const std::set<int> seamCutSet{seamCut.begin(), seamCut.end()};

    std::pair<int, int> previousCutEdge;

    // these faces are on one half of the seamCut
    std::set<int> firstHalfFaceSet;
    // these faces are on the other seamCut.
    std::set<int> secondHalfFaceSet;

    for (int iCurVertex = 0; iCurVertex < seamCut.size(); ++iCurVertex) {
        const int iCenterVertex = seamCut[iCurVertex];

        std::pair<int, int> currentCutEdge;

        if (iCurVertex != (seamCut.size() - 1)) {
            currentCutEdge = std::pair<int, int>(seamCut[iCurVertex + 0], seamCut[iCurVertex + 1]);
        } else {
            currentCutEdge = std::pair<int, int>(seamCut[iCurVertex - 1], seamCut[iCurVertex + 0]);
        }

        if (MY_ASSERT(vertexEdges[iCenterVertex].size() > 0)) {
            return false;
        }

        int iStartVertexEdge;

        // find start edge to start iterating edges from.
        if (iCurVertex == 0) {
            iStartVertexEdge = -1;
            for (int iIterVertexEdge : vertexEdges[iCenterVertex]) {
                sc3d::MeshTopology::Edge vertexEdge = topologyEdges[iIterVertexEdge];

                if (vertexEdge.face0 == -1 || vertexEdge.face1 == -1) {
                    iStartVertexEdge = iIterVertexEdge;
                    break;
                }
            }

            if (MY_ASSERT(iStartVertexEdge != -1)) {
                return false;
            }

        } else if (iCurVertex == seamCut.size() - 1) {
            iStartVertexEdge = -1;
            for (int iIterVertexEdge : vertexEdges[iCenterVertex]) {
                sc3d::MeshTopology::Edge vertexEdge = topologyEdges[iIterVertexEdge];

                if (vertexEdge.face0 == -1 || vertexEdge.face1 == -1) {
                    iStartVertexEdge = iIterVertexEdge;
                    break;
                }
            }

            if (MY_ASSERT(iStartVertexEdge != -1)) {
                return false;
            }

        } else {

            iStartVertexEdge = -1;
            for (int iIterVertexEdge : vertexEdges[iCenterVertex]) {
                sc3d::MeshTopology::Edge vertexEdge = topologyEdges[iIterVertexEdge];

                if (
                    (vertexEdge.vertex0 == previousCutEdge.first && vertexEdge.vertex1 == previousCutEdge.second) || (vertexEdge.vertex1 == previousCutEdge.first && vertexEdge.vertex0 == previousCutEdge.second)

                ) {
                    iStartVertexEdge = iIterVertexEdge;
                    break;
                }
            }

            if (MY_ASSERT(iStartVertexEdge != -1)) {
                return false;
            }
        }

        int iIterVertexEdge = iStartVertexEdge;
        sc3d::MeshTopology::Edge iterVertexEdge = topologyEdges[iIterVertexEdge];

        std::set<int> iteratedFaces;

        std::vector<int> firstHalfFaceVector;
        std::vector<int> secondHalfFaceVector;

        bool isFirstHalf = true;
        bool shouldFlipHalves = false;

        // starting from iStartVertexEdge, iterate around iCenterVertex
        // classifying faces into one of the two halves.
        while (true) {
            if (MY_ASSERT(
                    (iteratedFaces.count(iterVertexEdge.face0) == 0 || iteratedFaces.count(iterVertexEdge.face1) == 0))) {
                return false;
            }

            int iFace = -1;

            if (iteratedFaces.count(iterVertexEdge.face0) == 1) {
                iFace = iterVertexEdge.face1;
            } else {
                iFace = iterVertexEdge.face0;

                if (MY_ASSERT(iteratedFaces.size() == 0 || iteratedFaces.count(iterVertexEdge.face1) == 1)) {
                    return false;
                }
            }

            iteratedFaces.insert(iFace);

            if (MY_ASSERT(

                    geoFaces[iFace][0] == iCenterVertex || geoFaces[iFace][1] == iCenterVertex || geoFaces[iFace][2] == iCenterVertex)) {
                return false;
            }

            if (isFirstHalf) {
                firstHalfFaceVector.push_back(iFace);
            } else {
                secondHalfFaceVector.push_back(iFace);
            }

            if (isFirstHalf && secondHalfFaceSet.count(iFace) != 0 && iCurVertex > 0) {
                shouldFlipHalves = true;
            }

            bool foundNewIter = false;

            for (int jj = 0; jj < 3; ++jj) {
                const int iFaceEdge = topologyFaceEdges[iFace][jj];

                // we've already iterated over this one.
                if (iIterVertexEdge == iFaceEdge) {
                    continue;
                }

                const sc3d::MeshTopology::Edge faceEdge = topologyEdges[iFaceEdge];

                if (iCenterVertex == faceEdge.vertex0 || iCenterVertex == faceEdge.vertex1) {
                    iIterVertexEdge = iFaceEdge;
                    iterVertexEdge = faceEdge;
                    foundNewIter = true;
                    break;
                }
            }

            if (MY_ASSERT(foundNewIter)) {
                return false;
            }

            if (iIterVertexEdge == iStartVertexEdge) {
                break;
                // completed full loop.
            }

            if (iterVertexEdge.face0 == -1 || iterVertexEdge.face1 == -1) {
                break;
                // completed full loop.
            }

            if (
                (iterVertexEdge.vertex0 == currentCutEdge.first && iterVertexEdge.vertex1 == currentCutEdge.second) || (iterVertexEdge.vertex1 == currentCutEdge.first && iterVertexEdge.vertex0 == currentCutEdge.second)) {
                isFirstHalf = !isFirstHalf;
            }
        }

        if (shouldFlipHalves) {

            for (int i : firstHalfFaceVector) {
                secondHalfFaceSet.insert(i);
            }
            for (int i : secondHalfFaceVector) {
                firstHalfFaceSet.insert(i);
            }
        } else {
            for (int i : firstHalfFaceVector) {
                firstHalfFaceSet.insert(i);
            }
            for (int i : secondHalfFaceVector) {
                secondHalfFaceSet.insert(i);
            }
        }

        previousCutEdge = currentCutEdge;
    }

    // good. Now we've divided the faces into two sets.
    // now we can make the cut.

    std::vector<Vec3> newPositions = geoPositions;
    std::vector<Vec3> newNormals = geoNormals;
    std::vector<Vec3> newColors = geoColors;
    std::vector<Face3> newFaces = geoFaces;

    std::map<int, int> indexRemapping;

    // for one half, create new vertices.
    for (int iSeamCutVertex : seamCut) {
        Vec3 sourcePosition = newPositions[iSeamCutVertex];
        int iOld = iSeamCutVertex;
        int iNew = (int)newPositions.size();
        indexRemapping[iOld] = iNew;

        newPositions.push_back(sourcePosition);

        if (geometry.hasNormals()) {
            Vec3 sourceNormal = newNormals[iSeamCutVertex];
            newNormals.push_back(sourceNormal);
        }

        if (geometry.hasColors()) {
            Vec3 sourceColor = newColors[iSeamCutVertex];
            newColors.push_back(sourceColor);
        }
    }

    for (int iFace = 0; iFace < newFaces.size(); ++iFace) {
        if (secondHalfFaceSet.count(iFace) != 1) {
            // this half uses the old vertices. so skip
            continue;
        }

        Face3& newFace = newFaces[iFace];

        for (int iCorner = 0; iCorner < 3; ++iCorner) {

            if (seamCutSet.count(newFace[iCorner]) == 0) {
                continue; // not part of the team.
            }

            if (MY_ASSERT(indexRemapping.count(newFace[iCorner]) != 0)) {
                return false;
            }

            newFace[iCorner] = indexRemapping[newFace[iCorner]];
        }
    }

    Geometry newGeo(newPositions, newNormals, newColors, newFaces);
    geometry.copy(newGeo);

    return true;
}

/*
     
     std::vector<std::shared_ptr<Geometry>> cutNonDisk(
     
     const Geometry& geometry,
     
     std::vector<int> cutEdge)
     {
     struct Entry {
     int iSegment;
     int iFace;
     int iv;
     };
     
     std::vector<Face3> geoFaces = geometry.getFaces();
     std::vector<Vec3> geoPositions = geometry.getPositions();
     std::vector<Vec3> geoColors = geometry.getColors();
     std::vector<Vec3> geoNormals = geometry.getNormals();
     
     // map from vertex index, to list of all faces and segments that touch it.
     std::map<int, std::vector<Entry>> map;
     
     for (int iFace = 0; iFace < geometry.getNumFaces(); ++iFace) {
     
     Face3 face = geoFaces[iFace];
     
     for (int iv = 0; iv < 3; ++iv) {
     int vertexIndex = face[iv];
     
     for (int isegment = 0; isegment < segmentation.size(); ++isegment) {
     
     if (segmentation[isegment].count(iFace) == 1) {
     map[vertexIndex].push_back(Entry{isegment, iFace, iv});
     }
     }
     }
     }
     
     std::vector<int> segmentVertexIndexCache; // length is number of segments.
     
     for (int iSegment = 0; iSegment < segmentation.size(); ++iSegment) {
     segmentVertexIndexCache.push_back(-1);
     }
     
     for (auto pair : map) {
     if (pair.second.size() > 1) {
     
     int vertexIndex = pair.first;
     
     std::vector<int> newVertexIndices;
     
     for (int iSegment = 0; iSegment < segmentation.size(); ++iSegment) {
     segmentVertexIndexCache[iSegment] = -1;
     }
     
     // create new vertex for every segment.
     for (int ii = 0; ii < pair.second.size(); ++ii) {
     if (segmentVertexIndexCache[pair.second[ii].iSegment] != -1) {
     // already created this vertex for this segment.
     // used cached index.
     
     Face3& f = geoFaces[pair.second[ii].iFace];
     
     f[pair.second[ii].iv] = segmentVertexIndexCache[pair.second[ii].iSegment];
     continue;
     }
     
     if (MY_ASSERT(geoPositions.size() > 0)) {
     return std::vector<std::shared_ptr<Geometry>>();
     }
     
     int newVertexIndex = (int)geoPositions.size();
     
     if (geoPositions.size() > 0) {
     geoPositions.push_back(geoPositions[vertexIndex]);
     }
     
     if (geoNormals.size() > 0) {
     geoNormals.push_back(geoNormals[vertexIndex]);
     }
     
     if (geoColors.size() > 0) {
     geoColors.push_back(geoColors[vertexIndex]);
     }
     
     Face3& f = geoFaces[pair.second[ii].iFace];
     
     f[pair.second[ii].iv] = newVertexIndex;
     
     segmentVertexIndexCache[pair.second[ii].iSegment] = newVertexIndex;
     }
     }
     }
     
     geometry.setVertexData(geoPositions, geoNormals, geoColors);
     geometry.setFaces(geoFaces);
     
     removeUnusedVertices(geometry);
     
     // now that the mesh has been split across the segment borders,
     // we can easily separate it into separate meshes.
     std::vector<std::shared_ptr<Geometry>> ret = splitMeshIntoPieces(geometry);
     }
     }
     */

std::vector<std::shared_ptr<Geometry>> seamCutMeshBySegmentation(
    Geometry& geometry,
    std::vector<std::set<int>> segmentation)
{
    if (geometry.vertexCount() == 0 || !geometry.hasFaces()) {
        return std::vector<std::shared_ptr<Geometry>>();
    }

    std::vector<int> faceIndexToSegmentMapping(geometry.faceCount(), -1);

    for (int ii = 0; ii < geometry.vertexCount(); ++ii) {
        faceIndexToSegmentMapping.push_back(-1);
    }

    for (int iSegment = 0; iSegment < segmentation.size(); ++iSegment) {
        const std::set<int>& segment = segmentation[iSegment];

        for (int iFace : segment) {

            if (MY_ASSERT(faceIndexToSegmentMapping[iFace] == -1)) {
                return std::vector<std::shared_ptr<Geometry>>();
            }

            faceIndexToSegmentMapping[iFace] = iSegment;
        }
    }

    std::vector<std::vector<Vec3>> subPositions;
    std::vector<std::vector<Vec3>> subNormals;
    std::vector<std::vector<Vec3>> subColors;
    std::vector<std::vector<Face3>> subFaces;
    std::vector<int> freeIndex;
    std::vector<std::map<int, int>> subIndexMapping;

    for (int ii = 0; ii < segmentation.size(); ++ii) {
        // std::shared_ptr<Geometry> geo(new Geometry());
        subPositions.push_back(std::vector<Vec3>());
        subNormals.push_back(std::vector<Vec3>());
        subColors.push_back(std::vector<Vec3>());
        subFaces.push_back(std::vector<Face3>());
        freeIndex.push_back(0);
        subIndexMapping.push_back(std::map<int, int>());
    }

    const std::vector<Face3>& geoFaces = geometry.getFaces();

    const std::vector<Vec3>& geoPositions = geometry.getPositions();
    const std::vector<Vec3>& geoColors = geometry.getColors();
    const std::vector<Vec3>& geoNormals = geometry.getNormals();

    for (int iFace = 0; iFace < geoFaces.size(); ++iFace) {
        Face3 face = geoFaces[iFace];

        int iSegment = faceIndexToSegmentMapping[iFace];

        std::map<int, int>& indexMapping = subIndexMapping[iSegment];

        if (MY_ASSERT(iSegment >= 0 && iSegment < segmentation.size())) {
            return std::vector<std::shared_ptr<Geometry>>();
        }

        Face3 newFace;

        for (int iCorner = 0; iCorner < 3; ++iCorner) {

            if (indexMapping.count(face[iCorner]) != 0) {
                newFace[iCorner] = indexMapping[face[iCorner]];
            } else {
                // if an index is unencoutered, then add to list of positions in mesh.

                int newIndex = freeIndex[iSegment]++;

                if (geometry.hasPositions()) {
                    Vec3 position = geoPositions[face[iCorner]];
                    subPositions[iSegment].push_back(position);
                }
                if (geometry.hasNormals()) {
                    Vec3 normal = geoNormals[face[iCorner]];
                    subNormals[iSegment].push_back(normal);
                }

                if (geometry.hasColors()) {
                    Vec3 color = geoColors[face[iCorner]];
                    subColors[iSegment].push_back(color);
                }

                newFace[iCorner] = newIndex;

                indexMapping[face[iCorner]] = newIndex;
            }
        }

        subFaces[iSegment].push_back(newFace);
    }

    std::vector<std::shared_ptr<Geometry>> splittedMeshes;

    for (int iSubmesh = 0; iSubmesh < segmentation.size(); ++iSubmesh) {
        if (subFaces[iSubmesh].size() > 0) {
            std::shared_ptr<Geometry> geo(new Geometry(subPositions[iSubmesh],
                                                       subNormals[iSubmesh],
                                                       subColors[iSubmesh],
                                                       subFaces[iSubmesh]));

            splittedMeshes.push_back(geo);
        }
    }

    return splittedMeshes;

    // actually, this doesnt seem that important, you look at the final textured meshes
    // and its hard to get this robust
    // so skip this topological disk cutting stuff for now.
    /*
    // Unfortunately, our segmentation algorithm cant guarantee that all
    // charts are topological disks. We will cut them up even more,
    // so that they all become topological disks.
    std::vector<std::shared_ptr<Geometry>> topologicalDiskMeshPieces;
    while (!meshStack.empty()) {

        std::shared_ptr<Geometry> geo = meshStack.top();
        meshStack.pop();
        
        int loopCount = 0;
        while (true) {
            std::vector<std::vector<std::pair<int, int>>> edgeLoops = findEdgeLoops(*geo);

            // if there's more than one edge loop, there must be a hole in the chart.
            // so we must cut it up.
            if (edgeLoops.size() > 1) {
                printf("edgeLoops.size(): %d\n", (int)edgeLoops.size());
                if (MY_ASSERT(edgeLoops.size() >= 2)) {
                    return std::vector<std::shared_ptr<Geometry>>();
                }

                std::set<int> sourceSet;
                for (const std::pair<int, int>& edge : edgeLoops[0]) {
                    sourceSet.insert(edge.first);
                    sourceSet.insert(edge.second);
                }

                std::set<int> targetSet;
                for (const std::pair<int, int>& edge : edgeLoops[1]) {
                    targetSet.insert(edge.first);
                    targetSet.insert(edge.second);
                }
                
                
                std::set<int> setIntersection;
                
                for (int iSource : sourceSet) {
                    if (targetSet.count(iSource) == 1) {
                        setIntersection.insert(iSource);
                    }
                }
                
                if (setIntersection.size() != 0) {
                    // if source and target have intersecting vertices, then
                    // seamCutMesh() wont work. must split the shared vertex
                    // into two vertices to make it work.
                    
                    if (MY_ASSERT(splitSharedVertexIntoTwo(*geo, *setIntersection.begin()))) {
                        
                        {
                            std::vector<Vec3> points;
                            for(int s : sourceSet) {
                                points.push_back(geo->getPositions()[s]);
                            }
                            
                            Geometry mygeo(points);
                            WriteGeometryToPLYFile("/Users/eric/Downloads/out_source.ply", mygeo);
                        }
                        
                        {
                            std::vector<Vec3> points;
                            for(int s : targetSet) {
                                points.push_back(geo->getPositions()[s]);
                            }
                            
                            Geometry mygeo(points);
                            WriteGeometryToPLYFile("/Users/eric/Downloads/out_target.ply", mygeo);
                        }
                        return std::vector<std::shared_ptr<Geometry>>();
                    }
                    
                } else {
                    
                    
                    // the edge loops are basically the borders of the chart
                    // find the shortest path from one border to the other,
                    // using dijsktras algorith. that tends to give good cuts.
                     
                    bool foundAPath;
                    std::vector<int> cutEdges = findCutEdges(
                                                             sourceSet, targetSet,
                                                             geo->getFaces(), (int)geo->getPositions().size(), foundAPath);
                    
                    if (!foundAPath){

                        // if no path was found, its because source and target has gotten separated
                        // into two separate geometries during the splitting.
                        // add these geometries to the stack, and attempt processing them again.
                           
                        std::vector<std::shared_ptr<Geometry>> meshPieces = splitMeshIntoPieces(*geo);
                        
                        if (MY_ASSERT(meshPieces.size() >= 2)) {
                            return std::vector<std::shared_ptr<Geometry>>();
                        }
                        
                        for (auto meshPiece : meshPieces) {
                            printf("add a mesh piece!\n");
                            meshStack.push(meshPiece);
                        }
                        
                        break;
                    }
                    
                    if (MY_ASSERT(cutEdges.size() != 0)) {
   
                        
                        
                        
                    
                        WriteGeometryToPLYFile("/Users/eric/Downloads/out.ply", *geo);
                        
                        {
                            std::vector<Vec3> points;
                            for(int s : sourceSet) {
                                points.push_back(geo->getPositions()[s]);
                            }
                        
                            Geometry mygeo(points);
                            WriteGeometryToPLYFile("/Users/eric/Downloads/out_source.ply", mygeo);
                        }
                        
                        {
                            std::vector<Vec3> points;
                            for(int s : targetSet) {
                                points.push_back(geo->getPositions()[s]);
                            }
                        
                            Geometry mygeo(points);
                            WriteGeometryToPLYFile("/Users/eric/Downloads/out_target.ply", mygeo);
                        }
                        
                        exit(1);
                        
                        return std::vector<std::shared_ptr<Geometry>>();
                    }
                    
                    if (MY_ASSERT(seamCutMesh(*geo, cutEdges))) {
                        return std::vector<std::shared_ptr<Geometry>>();
                    }
                    
                }
                

            } else {
                
                topologicalDiskMeshPieces.push_back(geo);
                break;
            }

            loopCount++;

            if (MY_ASSERT(loopCount < 30)) {
                // make sure we dont get stuck in eternal loop.
                // so just crash if this runs for too long.
                return std::vector<std::shared_ptr<Geometry>>();
            }
        }

    }

    return topologicalDiskMeshPieces;
    */
}

/*
     Parameterize and find the UV-coords of some geometry, using LSCM.
     */
std::vector<Vec2> parameterize(const Geometry& geometry)
{

    Eigen::MatrixXd V(geometry.vertexCount(), 3);
    Eigen::MatrixXi F(geometry.faceCount(), 3);

    const std::vector<Vec3>& geoPositions = geometry.getPositions();
    const std::vector<Face3>& geoFaces = geometry.getFaces();

    for (int iv = 0; iv < geoPositions.size(); ++iv) {
        V.row(iv) << geoPositions[iv].x, geoPositions[iv].y, geoPositions[iv].z;
    }

    for (int iFace = 0; iFace < geoFaces.size(); ++iFace) {
        F.row(iFace) << geoFaces[iFace][0], geoFaces[iFace][1], geoFaces[iFace][2];
    }

    Eigen::MatrixXd V_uv;

    Eigen::VectorXi bnd;
    igl::boundary_loop(F, bnd);

    if (MY_ASSERT(bnd.rows() != 0)) {
        return std::vector<Vec2>();
    }

    // we need to fix at least two vertices for the LSCM method,
    // so we pick choose two on the boundary.
    Eigen::VectorXi b(2, 1);
    b(0) = bnd(0);
    b(1) = bnd((int)round(bnd.size() / 2));
    Eigen::MatrixXd bc(2, 2);
    bc << 0, 0, 1, 0;

    // LSCM parametrization
    bool lscmResult = igl::lscm(V, F, b, bc, V_uv);
    if (MY_ASSERT(lscmResult)) {
        return std::vector<Vec2>();
    }

    std::vector<Vec2> vertexUvs;


    auto isCoordValid = [=](float coord) {
        if (std::isnan(coord) || std::isinf(coord))
            return false;
        return true;
    };

    
#ifdef UVMAP_MESH_LOG
    printf("parameterize geo %d %d\n", geometry.faceCount(), geometry.vertexCount());
#endif
    
    for (int i = 0; i < geoPositions.size(); i++) {
        Vec2 coord;
        coord.x = V_uv.row(i)[0];
        coord.y = V_uv.row(i)[1];

        if (MY_ASSERT(isCoordValid(coord.x) && isCoordValid(coord.y))) {
            return std::vector<Vec2>();
        }

        vertexUvs.push_back(coord);
    }

    return vertexUvs;
}

/**
 Removes all degenerate triangles from the geometry.
 Degenerate triangles are simply triangles with an area of zero.
 */
bool removeDegenerateTriangles(Geometry& geometry)
{

    int numRemoved = 0;
    int totalIterations = 0;
    const int MAX_ITERATIONS = 100;
    
    do {
        numRemoved = 0;
        const std::vector<Vec3>& positions = geometry.getPositions();

        std::vector<Face3> newFaces;

        // first merge vertices, in triangles where two vertices are exactly equal.
        {
            constexpr float eps = 0.0000000000000000000000000001f;

            std::vector<Face3> newFacesTemp;

            std::set<std::pair<int, int>> duplicatesToMerge;

            // discard zero-area triangle faces:
            {
                const std::vector<Face3>& faces = geometry.getFaces();

                for (int iFace = 0; iFace < faces.size(); ++iFace) {
                    Face3 face = faces[iFace];

                    Vec3 a = positions[face[0]];
                    Vec3 b = positions[face[1]];
                    Vec3 c = positions[face[2]];

                    float area = (Vec3::cross(b - a, c - a)).norm() * 0.5f;

                    if (area < eps) {
                        // discard this face.
                        int duplicateVertexA = -1;
                        int duplicateVertexB = -1;

                        if (Vec3::almostEqual(a, b, eps)) {
                            duplicateVertexA = face[0];
                            duplicateVertexB = face[1];
                        } else if (Vec3::almostEqual(a, c, eps)) {
                            duplicateVertexA = face[0];
                            duplicateVertexB = face[2];
                        } else if (Vec3::almostEqual(b, c, eps)) {
                            duplicateVertexA = face[1];
                            duplicateVertexB = face[2];
                        } else if (Vec3::almostEqual(a, b, eps) && Vec3::almostEqual(b, c, eps)) {
                            MY_ASSERT("Unhandled zero area triangle");
                            return false;
                        } else {

                            // if (MY_ASSERT("Unhandled zero area triangle lol" == "")) {
                            // return false;
                            // }


                            // we'll handle colinear vertices in a second pass.
                            newFacesTemp.push_back(face);
                            continue;
                        }

                        // these two vertices must be merged later:

                        if (duplicatesToMerge.count({duplicateVertexB, duplicateVertexA}) == 0 && duplicatesToMerge.count({duplicateVertexA, duplicateVertexB}) == 0) {

                            duplicatesToMerge.insert({duplicateVertexA, duplicateVertexB});
                        }
                    } else {
                        newFacesTemp.push_back(face);
                    }
                }
            }


            numRemoved += duplicatesToMerge.size();


            {
                std::map<int, std::set<int>> vertexIndexToTriMap = std::map<int, std::set<int>>();

                for (int iFace = 0; iFace < newFacesTemp.size(); ++iFace) {
                    Face3 face = newFacesTemp[iFace];

                    vertexIndexToTriMap[face[0]].insert(iFace);
                    vertexIndexToTriMap[face[1]].insert(iFace);
                    vertexIndexToTriMap[face[2]].insert(iFace);
                }

                // due to removing degenerate triangles, duplicate vertices that should
                // be merged are created.
                // so merge all duplicate vertices.
                for (auto duplicate : duplicatesToMerge) {
                    // given vertex index, find all triangles that use that vertex.

                    int duplicateVertexA = duplicate.first;
                    int duplicateVertexB = duplicate.second;

#ifdef UVMAP_MESH_LOG
                    printf("vertex merge %d %d\n", duplicateVertexA, duplicateVertexB);
#endif
                    // replace all instances of B with A.
                    std::set<int> targetFaces = vertexIndexToTriMap[duplicateVertexB];


                    for (int iTargetFace : targetFaces) {
                        Face3& targetFace = newFacesTemp
                            [iTargetFace];
                        for (int iv = 0; iv < 3; ++iv) {
                            if (targetFace[iv] == duplicateVertexB) {
                                targetFace[iv] = duplicateVertexA;
                                vertexIndexToTriMap[duplicateVertexA].insert(iTargetFace);
                            }
                        }
                    }

                    vertexIndexToTriMap[duplicateVertexB].clear();
                }
            }


            newFaces = newFacesTemp;
        }

        // now merge vertices, in triangles where the vertices are colinear.
        {
            std::set<int> facesToProcess;

            // discard zero-area triangle faces:
            {
                for (int iFace = 0; iFace < newFaces.size(); ++iFace) {
                    Face3 face = newFaces[iFace];

                    Vec3 a = positions[face[0]];
                    Vec3 b = positions[face[1]];
                    Vec3 c = positions[face[2]];

                    float area = (Vec3::cross(b - a, c - a)).norm() * 0.5f;


                    constexpr float eps = 0.0000000000000000000000000001f;

                    if (area < eps) {
                        // discard this face.

                        if (Vec3::almostEqual(a, b, eps)) {
                            MY_ASSERT("Unhandled Vec3::almostEqual(a, b, eps)");
                            return false;
                        } else if (Vec3::almostEqual(a, c, eps)) {
                            MY_ASSERT("Unhandled Vec3::almostEqual(a, c, eps)");
                            return false;
                        } else if (Vec3::almostEqual(b, c, eps)) {
                            MY_ASSERT("Unhandled Vec3::almostEqual(b, c, eps)");
                            return false;
                        } else {
                            facesToProcess.insert(iFace);
                        }
                    } else {
                    }
                    //newFacesTemp.push_back(face);
                }
            }

            std::vector<Face3> newlyCreatedFaces;
            std::set<int> facesToRemove;

            numRemoved += facesToProcess.size();

            for (int iFaceToProcess : facesToProcess) {
                Face3 faceToProcess = newFaces[iFaceToProcess];

                std::array<Vec3, 3> p;

                p[0] = positions[faceToProcess[0]];
                p[1] = positions[faceToProcess[1]];
                p[2] = positions[faceToProcess[2]];

                Vec3 o = p[0];
                Vec3 d = (p[1] - p[0]).normalize();

                constexpr float eps = 0.000001f;

                int iNonZeroD = -1;
                for (int i = 0; i < 3; ++i) {

                    if (fabs(d[i]) > eps) {
                        iNonZeroD = i;
                        break;
                    }
                }

                if (MY_ASSERT(iNonZeroD != -1)) {
                    return false;
                }

                std::array<float, 3> t; // t-values for all points.

                for (int iPoint = 0; iPoint < 3; ++iPoint) {
                    t[iPoint] = (p[iPoint][iNonZeroD] - o[iNonZeroD]) / d[iNonZeroD];
                }

                int iTargetVertex;
                std::array<int, 2> iOppositeEdge;

                if ((t[0] < t[1] && t[1] < t[2]) || (t[2] < t[1] && t[1] < t[0])) {
                    // 1 is the target.
                    iTargetVertex = faceToProcess[1];
                    iOppositeEdge = {faceToProcess[0], faceToProcess[2]};
                } else if ((t[2] < t[0] && t[0] < t[1]) || (t[1] < t[0] && t[0] < t[2])) {
                    // 0 is the target.
                    iTargetVertex = faceToProcess[0];
                    iOppositeEdge = {faceToProcess[1], faceToProcess[2]};
                } else if ((t[0] < t[2] && t[2] < t[1]) || (t[1] < t[2] && t[2] < t[0])) {
                    // 2 is the target.
                    iTargetVertex = faceToProcess[2];
                    iOppositeEdge = {faceToProcess[0], faceToProcess[1]};
                } else {
                    iTargetVertex = -1;
                    MY_ASSERT("Unhandled t[0] t[1] t[2] sorting case");
                    return false;
                }


                int iSplittedFace = -1;

                for (int iFace = 0; iFace < newFaces.size(); ++iFace) {
                    if (iFace == iFaceToProcess) {
                        continue;
                    }

                    Face3 face = newFaces[iFace];
                    for (int iEdge = 0; iEdge < 3; ++iEdge) {
                        int vertexA = face[(iEdge + 0) % 3];
                        int vertexB = face[(iEdge + 1) % 3];

                        if ((iOppositeEdge[0] == vertexA && iOppositeEdge[1] == vertexB) || (iOppositeEdge[1] == vertexA && iOppositeEdge[0] == vertexB)) {

                            if (MY_ASSERT(iSplittedFace == -1)) {
                                return false;
                            }

                            newlyCreatedFaces.push_back(Face3{face[(iEdge + 2) % 3], face[(iEdge + 3) % 3], iTargetVertex});

                            newlyCreatedFaces.push_back(Face3{iTargetVertex, face[(iEdge + 4) % 3], face[(iEdge + 5) % 3]});

                            iSplittedFace = iFace;
                        }
                    }
                }

                if (MY_ASSERT(iSplittedFace != -1)) {
                    return false;
                }

                facesToRemove.insert(iSplittedFace);
            }

            std::vector<Face3> newFacesTemp;

            for (int iFace = 0; iFace < newFaces.size(); ++iFace) {
                if (facesToRemove.count(iFace) == 1) {
                    continue;
                }
                if (facesToProcess.count(iFace) == 1) {
                    continue;
                }

                newFacesTemp.push_back(newFaces[iFace]);
            }

            // newlyCreatedFaces
            for (Face3 face : newlyCreatedFaces) {
                newFacesTemp.push_back(face);
            }

            newFaces = newFacesTemp;
        }

        geometry.setFaces(newFaces);
        // finally, remove all vertices that have now become unused.
        removeUnusedVertices(geometry);
        totalIterations++;
    } while (numRemoved != 0 && totalIterations != MAX_ITERATIONS);

    if (MY_ASSERT(totalIterations != MAX_ITERATIONS)) {
        return false;
    }

    return true;
}

/// Removes all vertices that are not referenced by any faces
void removeUnusedVertices(Geometry& geometry)
{
    int freeIndex = 0;


    const std::vector<Vec3>& geoPositions = geometry.getPositions();
    const std::vector<Vec3>& geoNormals = geometry.getNormals();
    const std::vector<Vec3>& geoColors = geometry.getColors();
    const std::vector<Vec2>& geoTexCoords = geometry.getTexCoords();
    std::vector<Face3> geoFaces = geometry.getFaces();

    // if vertexUsage[iv]==false, vertex iv is no longer used anymore.
    std::vector<bool> vertexUsage;
    for (int iv = 0; iv < geometry.vertexCount(); ++iv) {
        vertexUsage.push_back(false);
    }

    for (int iFace = 0; iFace < geoFaces.size(); ++iFace) {
        Face3& face = geoFaces[iFace];

        vertexUsage[face[0]] = true;
        vertexUsage[face[1]] = true;
        vertexUsage[face[2]] = true;
    }

    std::map<int, int> newIndexMapping;

    // below, add all still-being-used vertices into our new vertex list
    std::vector<Vec3> newPositions;
    std::vector<Vec3> newNormals;
    std::vector<Vec3> newColors;
    std::vector<Vec2> newTexCoords;
    for (int iv = 0; iv < geometry.vertexCount(); ++iv) {
        if (vertexUsage[iv]) {
            int newIndex = freeIndex++; // the new index of this vertex.
            newIndexMapping[iv] = newIndex;

            if (geometry.hasPositions()) {
                Vec3 position = geoPositions[iv];
                newPositions.push_back(position);
            }
            if (geometry.hasNormals()) {
                Vec3 normal = geoNormals[iv];
                newNormals.push_back(normal);
            }

            if (geometry.hasColors()) {
                Vec3 color = geoColors[iv];
                newColors.push_back(color);
            }

            if (geometry.hasTexCoords()) {
                Vec2 texCoord = geoTexCoords[iv];
                newTexCoords.push_back(texCoord);
            }
        }
    }

    // update face indices.
    for (int iFace = 0; iFace < geoFaces.size(); ++iFace) {
        Face3& face = geoFaces[iFace];

        face[0] = newIndexMapping[face[0]];
        face[1] = newIndexMapping[face[1]];
        face[2] = newIndexMapping[face[2]];
    }

    geometry.copy(Geometry(newPositions, geoFaces));
    geometry.setTexCoords(newTexCoords);
    geometry.setNormals(newNormals);
    geometry.setColors(newColors);
}

bool tryPack(
    float textureSize,
    const std::vector<std::pair<float, float>>& chartSizes,
    std::vector<std::tuple<float, float, float, float, bool>>& m_result)
{
    // typedef struct maxRectsPosition {
    //     float left;
    //     float top;
    //     int rotated : 1;
    // } maxRectsPosition;

    std::vector<rbp::RectSize> rects;

    float width = textureSize;
    float height = width;

    //float paddingSize = 0.000000001 * width; paddingSize = 0;
    float paddingSize = 0; // 0 for now.

    float paddingSize2 = paddingSize + paddingSize;
    for (const auto& chartSize : chartSizes) {
        rbp::RectSize r;
        r.width = chartSize.first + paddingSize2;
        r.height = chartSize.second + paddingSize2;

        rects.push_back(r);
    }

    const rbp::MaxRectsBinPack::FreeRectChoiceHeuristic methods[] = {
        rbp::MaxRectsBinPack::RectBestShortSideFit,
        rbp::MaxRectsBinPack::RectBestLongSideFit,
        rbp::MaxRectsBinPack::RectBestAreaFit,
        rbp::MaxRectsBinPack::RectBottomLeftRule,
        rbp::MaxRectsBinPack::RectContactPointRule};
    float occupancy = 0;
    float bestOccupancy = 0;
    std::vector<rbp::Rect> bestResult;
    for (size_t i = 0; i < sizeof(methods) / sizeof(methods[0]); ++i) {
        rbp::MaxRectsBinPack binPack(width, height, false);

        std::vector<rbp::Rect> result;

        std::vector<rbp::RectSize> rectsCopy(rects);

        if (binPack.Insert(rectsCopy, result, methods[i]) == false)
            continue;

        occupancy = binPack.Occupancy();

        if (occupancy > bestOccupancy) {
            bestResult = result;
            bestOccupancy = occupancy;
        }
    }
    if (bestResult.size() != rects.size())
        return false;

    m_result.clear();
    m_result.resize(bestResult.size());
    for (decltype(bestResult.size()) i = 0; i < bestResult.size(); ++i) {
        const auto& result = bestResult[i];
        auto& dest = m_result[i];

        std::get<0>(dest) = (float)(result.x + paddingSize) / width;
        std::get<1>(dest) = (float)(result.y + paddingSize) / height;
        std::get<2>(dest) = (float)(result.width - paddingSize2) / width;
        std::get<3>(dest) = (float)(result.height - paddingSize2) / height;
        std::get<4>(dest) = false;
    }
    return true;
}

/**
    Given a list of chart sizes, finds an efficient packing.

    Chart packing algorithm was based on this MIT licensed code:
    https://github.com/huxingyi/simpleuv/blob/master/simpleuv/chartpacker.cpp
*/
std::vector<std::tuple<float, float, float, float, bool>> packCharts(const std::vector<std::pair<float, float>>& chartSizes, float& textureSize)
{
    {
        float totalChartSize = 0;

        for (const auto& chartSize : chartSizes) {
            totalChartSize += std::get<0>(chartSize) * std::get<1>(chartSize);
        }

        textureSize = sqrt(totalChartSize);
    }

    int tryNum = 0;

    std::vector<std::tuple<float, float, float, float, bool>> result;

    while (true) {
        ++tryNum;
        result.clear();

#ifdef UVMAP_MESH_LOG
        printf("texture size %f\n", textureSize);
#endif
        
        if (tryPack(textureSize, chartSizes, result))
            break;

        if (MY_ASSERT(tryNum < 100)) {
            return std::vector<std::tuple<float, float, float, float, bool>>();
        }


        textureSize = 1.1f * textureSize;
    }

    return result;
}


/**
 Concatenates a bunch of geometries into a single geometry.
 Note that this method doesnt do anything like merge identical vertices with each other
 It just does straight up concatenation.
*/
std::unique_ptr<Geometry> combineGeometries(const std::vector<std::shared_ptr<Geometry>>& geometries)
{
    std::vector<Vec3> combinedPositions;
    std::vector<Vec3> combinedNormals;
    std::vector<Vec3> combinedColors;
    std::vector<Vec2> combinedTexCoords;

    int globalIndex = 0;

    std::vector<std::map<int, int>> indexRemappings;

    for (int iGeo = 0; iGeo < geometries.size(); ++iGeo) {
        const Geometry& geo = *geometries[iGeo];

        const std::vector<Vec3>& geoPositions = geo.getPositions();
        const std::vector<Vec3>& geoNormals = geo.getNormals();
        const std::vector<Vec3>& geoColors = geo.getColors();
        const std::vector<Vec2>& geoTexCoords = geo.getTexCoords();

        // remap index in this submesh, to the new mesh that contiains all combined vertices
        std::map<int, int> indexRemapping;

        for (int iv = 0; iv < geo.vertexCount(); ++iv) {
            indexRemapping[iv] = globalIndex++;

            combinedPositions.push_back(geoPositions[iv]);

            if (geoNormals.size() > 0) {
                combinedNormals.push_back(geoNormals[iv]);
            }

            if (geoColors.size() > 0) {
                combinedColors.push_back(geoColors[iv]);
            }

            if (geoTexCoords.size() > 0) {
                combinedTexCoords.push_back(geoTexCoords[iv]);
            }
        }

        indexRemappings.push_back(indexRemapping);
    }

    std::vector<Face3> combinedFaces;

    for (int iGeo = 0; iGeo < geometries.size(); ++iGeo) {
        const Geometry& geo = *geometries[iGeo];

        const std::vector<Face3>& geoFaces = geo.getFaces();

        std::map<int, int>& indexRemapping = indexRemappings[iGeo];

        for (int iFace = 0; iFace < geo.faceCount(); ++iFace) {
            Face3 oldFace = geoFaces[iFace];

            Face3 newFace;

            newFace[0] = indexRemapping[oldFace[0]];
            newFace[1] = indexRemapping[oldFace[1]];
            newFace[2] = indexRemapping[oldFace[2]];

            combinedFaces.push_back(newFace);
        }
    }

    std::unique_ptr<Geometry> combinedGeometry(new Geometry());

    combinedGeometry->setPositions(combinedPositions);
    combinedGeometry->setFaces(combinedFaces);

    if (combinedNormals.size() > 0) {
        combinedGeometry->setNormals(combinedNormals);
    }

    if (combinedColors.size() > 0) {
        combinedGeometry->setColors(combinedColors);
    }

    if (combinedTexCoords.size() > 0) {
        combinedGeometry->setTexCoords(combinedTexCoords);
    }

    return combinedGeometry;
}

bool uvmapMesh(Geometry& geo)
{
    sc3d::MeshTopology::MeshTopology topology(geo.getFaces());

    // remove degenerate triangles, otherwise LSCM will fail to UV-unwrap.
    if (!removeDegenerateTriangles(geo)) {
        return false;
    }

    sc3d::MeshTopology::MeshTopology topology2(geo.getFaces());

    // split mesh into charts.
    // each chart will be UV-unwrapped with LSCM.
    // then each chart is packed using a packing algorithm.
    // then, at the end, we simply concatenate all the charts together
    // to create our final mesh.
    std::vector<std::shared_ptr<Geometry>> charts;
    {
        // find all disconnected meshes in the geometry.
        std::vector<std::shared_ptr<Geometry>> meshPieces = splitMeshIntoPieces(geo);


        if (MY_ASSERT(meshPieces.size() > 0)) {
            return false;
        }

        int throwAwayThreshold;

        {
            int maxFaceCount = 0;
            for (auto meshPiece : meshPieces) {
                maxFaceCount = std::max(meshPiece->faceCount(), maxFaceCount);
            }
            throwAwayThreshold = 0.001 * maxFaceCount;
            //0.001
        }

        // split each mesh into charts.
        for (int iMeshPiece = 0; iMeshPiece < meshPieces.size(); ++iMeshPiece) {


            if (meshPieces[iMeshPiece]->faceCount() < throwAwayThreshold) {
                continue;
            }

#ifdef UVMAP_MESH_LOG
            printf("mesh piece %d\n", iMeshPiece);
            printf("mesh piece %d %d\n",
                   meshPieces[iMeshPiece]->vertexCount(),
                   meshPieces[iMeshPiece]->faceCount());
#endif

            std::vector<std::set<int>> segmentation;

            segmentation = segmentMeshNew(*meshPieces[iMeshPiece]);

            if (segmentation.size() == 0) {
                return false; // some assert failed.
            }

            std::vector<std::shared_ptr<Geometry>> cuttedCharts = seamCutMeshBySegmentation(*meshPieces[iMeshPiece], segmentation);

            if (cuttedCharts.size() == 0) {
                return false; // some assert failed.
            }

            for (int iChart = 0; iChart < cuttedCharts.size(); ++iChart) {
                charts.push_back(cuttedCharts[iChart]);
            }
        }
    }

    if (MY_ASSERT(charts.size() > 0)) {
        return false;
    }

    std::vector<std::pair<float, float>> chartSizes;
    std::vector<std::tuple<float, float, float, float>> minMaxTexCoords;

    // parameterize each chart.
    for (int iChart = 0; iChart < charts.size(); ++iChart) {
        Geometry& geometry = *charts[iChart];

        std::vector<Vec2> geoUvs = parameterize(geometry);
        if (geoUvs.size() == 0) {
            return false; // some assert failed.
        }

        const std::vector<Face3>& geoFaces = geometry.getFaces();
        const std::vector<Vec3>& geoPositions = geometry.getPositions();

        float totalGeoArea = 0.0f;
        for (Face3 face : geoFaces) {
            Vec3 a = geoPositions[face[0]];
            Vec3 b = geoPositions[face[1]];
            Vec3 c = geoPositions[face[2]];

            float triArea = (Vec3::cross(b - a, c - a)).norm() * 0.5f;

            assert(triArea >= 0.0f);

            totalGeoArea += triArea;
        }

        float totalUvArea = 0.0f;
        for (Face3 face : geoFaces) {
            Vec2 a = geoUvs[face[0]];
            Vec2 b = geoUvs[face[1]];
            Vec2 c = geoUvs[face[2]];

            Vec3 a3d = Vec3(a.x, a.y, 0.0);
            Vec3 b3d = Vec3(b.x, b.y, 0.0);
            Vec3 c3d = Vec3(c.x, c.y, 0.0);

            float triArea = (Vec3::cross(b3d - a3d, c3d - a3d)).norm() * 0.5f;

            assert(triArea >= 0.0f);


            totalUvArea += triArea;
        }

        float scale = sqrt(totalGeoArea / totalUvArea);

        for (Vec2& uv : geoUvs) {

            uv.x *= scale;
            uv.y *= scale;
        }

        float umin = +std::numeric_limits<float>::max();
        float vmin = +std::numeric_limits<float>::max();

        float umax = -std::numeric_limits<float>::max();
        float vmax = -std::numeric_limits<float>::max();

        for (int iv = 0; iv < geoUvs.size(); ++iv) {
            Vec2 uv = geoUvs[iv];

            umin = std::min(umin, uv.x);
            vmin = std::min(vmin, uv.y);

            umax = std::max(umax, uv.x);
            vmax = std::max(vmax, uv.y);
        }

        chartSizes.push_back({umax - umin, vmax - vmin});

        minMaxTexCoords.push_back({umin, vmin, umax, vmax});

        geometry.setTexCoords(geoUvs);
    }

#ifdef UVMAP_MESH_LOG
    printf("charts: %d\n", (int)chartSizes.size());
#endif
    
    // pack all charts.
    float textureSize;
    std::vector<std::tuple<float, float, float, float, bool>> packedCharts = packCharts(chartSizes, textureSize);

    if (packedCharts.size() == 0) {
        return false; // some assert failed.
    }

    if (MY_ASSERT(charts.size() > 0)) {
        return false;
    }

    if (MY_ASSERT(packedCharts.size() == charts.size())) {
        return false;
    }


    // use the packing results, to adjust the UV coordinates of the charts.
    for (int iChart = 0; iChart < charts.size(); ++iChart) {
        Geometry& geometry = *charts[iChart];

        if (MY_ASSERT(geometry.hasTexCoords())) {
            return false;
        }

        auto t = minMaxTexCoords[iChart];

        float umin = std::get<0>(t);
        float vmin = std::get<1>(t);
        float umax = std::get<2>(t);
        float vmax = std::get<3>(t);

        auto packedChart = packedCharts[iChart];

        float packedLeft = std::get<0>(packedChart);
        float packedTop = std::get<1>(packedChart);
        float packedWidth = std::get<2>(packedChart);
        float packedHeight = std::get<3>(packedChart);

        float packedUmin = packedLeft;
        float packedUmax = packedLeft + packedWidth;
        float packedVmin = packedTop;
        float packedVmax = packedTop + packedHeight;

        std::vector<Vec2> texCoords = geometry.getTexCoords();

        if (MY_ASSERT(texCoords.size() == geometry.vertexCount())) {
            return false;
        }

        for (int itc = 0; itc < texCoords.size(); ++itc) {
            Vec2 texCoord = texCoords[itc];

            Vec2 newTexCoord;

            newTexCoord.x = ((texCoord.x - umin) / (umax - umin)) * (packedUmax - packedUmin) + packedUmin;


            newTexCoord.y = ((texCoord.y - vmin) / (vmax - vmin)) * (packedVmax - packedVmin) + packedVmin;


            if (MY_ASSERT(newTexCoord.x <= 1.0f)) {
                return false;
            }

            if (MY_ASSERT(newTexCoord.y <= 1.0f)) {
                return false;
            }

            texCoords[itc] = newTexCoord;
        }

        bool success = geometry.setTexCoords(texCoords);
        if (MY_ASSERT(success == true)) {
            return false;
        }
    }

    geo.copy(*combineGeometries(charts));
    return true;
}

} // namespace algorithms

} // namespace standard_cyborg
