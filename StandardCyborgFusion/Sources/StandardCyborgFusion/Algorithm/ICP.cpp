//
//  ICP.cpp
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 7/10/18.
//

#include <dispatch/dispatch.h>

#include <standard_cyborg/util/DataUtils.hpp>
#include <standard_cyborg/util/DebugHelpers.hpp>

#include "GeometryHelpers.hpp"
#include "ThreadPool.hpp"
#include "ICP.hpp"

#include "EigenHelpers.hpp"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "Eigen/Jacobi"
#pragma clang diagnostic pop

using namespace standard_cyborg;

struct _ICPCorrespondence {
    const std::vector<math::Vec3>& sourceVertices;
    
    std::vector<math::Vec3>& targetVertices;
    std::vector<math::Vec3>& targetNormals;

    Eigen::VectorXf* squaredErrors;
    Eigen::VectorXf* weights;
    
    _ICPCorrespondence(const std::vector<math::Vec3>& sourceVertices,
                       std::vector<math::Vec3>& targetVertices,
                       std::vector<math::Vec3>& targetNormals,
                       Eigen::VectorXf* squaredErrors,
                       Eigen::VectorXf* weights) :
        sourceVertices(sourceVertices),
        targetVertices(targetVertices),
        targetNormals(targetNormals),
        squaredErrors(squaredErrors),
        weights(weights)
    {}
    
    // Weighted RMS: only correspondences whose outlier weight is non-zero contribute.
    // This is the quantity ICP's convergence test watches.
    float computeRMSCorrespondenceError()
    {
        size_t sourceVerticesLength = sourceVertices.size();
        const Eigen::VectorXf& w = *weights;

        double sumSquaredError = 0.0;
        double weightSum = 0;
        
        for (size_t i = 0; i < sourceVerticesLength; ++i) {
            float weight = w(i);
            if (weight == 0.0f) { continue; }
            Vector3f correspondenceError = standard_cyborg::toVector3f(sourceVertices[i]) - standard_cyborg::toVector3f(targetVertices[i]);
            float errorSquared = correspondenceError.squaredNorm();
            sumSquaredError += errorSquared * weight;
            weightSum += weight;
        }

        if (weightSum == 0.0) { return 0.0f; }
        return (float)sqrt(sumSquaredError / weightSum);
    }
};

static inline void AddUpperTriangularOuterProductToMatrix6x6(Eigen::VectorXf& source, Eigen::MatrixXf& destination)
{
    destination(0, 0) += source[0] * source[0];
    destination(0, 1) += source[0] * source[1];
    destination(0, 2) += source[0] * source[2];
    destination(0, 3) += source[0] * source[3];
    destination(0, 4) += source[0] * source[4];
    destination(0, 5) += source[0] * source[5];
    
    destination(1, 1) += source[1] * source[1];
    destination(1, 2) += source[1] * source[2];
    destination(1, 3) += source[1] * source[3];
    destination(1, 4) += source[1] * source[4];
    destination(1, 5) += source[1] * source[5];
    
    destination(2, 2) += source[2] * source[2];
    destination(2, 3) += source[2] * source[3];
    destination(2, 4) += source[2] * source[4];
    destination(2, 5) += source[2] * source[5];
    
    destination(3, 3) += source[3] * source[3];
    destination(3, 4) += source[3] * source[4];
    destination(3, 5) += source[3] * source[5];
    
    destination(4, 4) += source[4] * source[4];
    destination(4, 5) += source[4] * source[5];
    
    destination(5, 5) += source[5] * source[5];
}

static inline void FillUpperTriangularMatrixIntoLower6x6(Eigen::MatrixXf& m)
{
    m(1, 0) = m(0, 1);
    
    m(2, 0) = m(0, 2);
    m(2, 1) = m(1, 2);

    m(3, 0) = m(0, 3);
    m(3, 1) = m(1, 3);
    m(3, 2) = m(2, 3);

    m(4, 0) = m(0, 4);
    m(4, 1) = m(1, 4);
    m(4, 2) = m(2, 4);
    m(4, 3) = m(3, 4);

    m(5, 0) = m(0, 5);
    m(5, 1) = m(1, 5);
    m(5, 2) = m(2, 5);
    m(5, 3) = m(3, 5);
    m(5, 4) = m(4, 5);
}

static void _computeCorrespondencePartial(off_t rangeStart, off_t rangeEnd,
                                          const std::vector<math::Vec3>& sourceVertices,
                                          const sc3d::Geometry& targetCloud,
                                          std::vector<math::Vec3>& targetVertices,
                                          std::vector<math::Vec3>& targetNormals,
                                          Eigen::VectorXf* squaredErrors,
                                          double* sumSquaredErrorOut,
                                          dispatch_group_t group)
{
    double sumSquaredError = 0;
    
    const std::vector<math::Vec3>& positions = targetCloud.getPositions();
    const std::vector<math::Vec3>& normals = targetCloud.getNormals();
    
    for (off_t i = rangeStart; i < rangeEnd; ++i) {
        size_t nearestRefVertexIndex = targetCloud.getClosestVertexIndex(sourceVertices[i]);
        
        // Use the nearest neighbor
        targetVertices[i] = positions[nearestRefVertexIndex];
        targetNormals[i] = normals[nearestRefVertexIndex];
        
        float nearestRefVertexDistanceSquared = math::Vec3::squaredDistanceBetween(sourceVertices[i], targetVertices[i]);
        
        (*squaredErrors)(i) = nearestRefVertexDistanceSquared;
        sumSquaredError += nearestRefVertexDistanceSquared;
    }
    
    *sumSquaredErrorOut = sumSquaredError;
    
    dispatch_group_leave(group);
}

static _ICPCorrespondence _computeCorrespondence(const std::vector<math::Vec3>& sourceVertices,
                                                 const sc3d::Geometry& targetCloud,
                                                 const ICPConfiguration& config,
                                                 std::vector<math::Vec3>& targetVertices,
                                                 std::vector<math::Vec3>& targetNormals,
                                                 Eigen::VectorXf* squaredErrors,
                                                 Eigen::VectorXf* weights)
{
    size_t vertexCount = sourceVertices.size();
    assert(vertexCount > 0);
    
    // make sure to initialize the KD-tree. since initializing it in the multithreaded code, is asking for trouble.
    targetCloud.getClosestVertexIndex(math::Vec3(0,0,0));
    
    // Just doing 2 threads for now, because iPhone X is 2 cores, one of which is mostly busy doing UI and other AR work anyway
#define MULTITHREAD 1
    static ThreadPool threadPool(config.threadCount, QOS_CLASS_USER_INTERACTIVE);
    static dispatch_group_t group = dispatch_group_create();
    double sumSquaredError = 0;
    
#if !MULTITHREAD
    dispatch_group_enter(group);
    _computeCorrespondencePartial(0, vertexCount,
                                  sourceVertices,
                                  targetCloud,
                                  
                                  targetVertices,
                                  targetNormals,
                                  squaredErrors,
                                  &sumSquaredError,
                                  group);
#else
    double sumSquaredErrors[config.threadCount];
    memset(&sumSquaredErrors, config.threadCount, sizeof(double));
    
    for (int i = 0; i < config.threadCount; ++i) {
        size_t rangeStart = vertexCount * (i) / config.threadCount;
        size_t rangeEnd = vertexCount * (i + 1) / config.threadCount;
        
        dispatch_group_enter(group);
        
        threadPool.addJob(std::bind(_computeCorrespondencePartial,
                                    rangeStart, rangeEnd,
                                    std::cref(sourceVertices),
                                    std::cref(targetCloud),
                                    std::ref(targetVertices),
                                    std::ref(targetNormals),
                                    squaredErrors,
                                    &sumSquaredErrors[i],
                                    group));
    }
    
    dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
    
    for (int i = 0; i < config.threadCount; ++i) {
        sumSquaredError += sumSquaredErrors[i];
    }
#endif
    
    // Outlier rejection based on the Euclidean distance distribution of correspondences
    Eigen::VectorXf distances(vertexCount);
    double sumDistance = 0.0;
    for (size_t i = 0; i < vertexCount; ++i) {
        float d = std::sqrt((*squaredErrors)(i));
        distances(i) = d;
        sumDistance += d;
    }
    double meanDistance = sumDistance / vertexCount;

    double varianceDistance = 0.0;
    for (size_t i = 0; i < vertexCount; ++i) {
        double delta = distances(i) - meanDistance;
        varianceDistance += delta * delta;
    }
    float stdDevDistance = (vertexCount > 1) ? (float)std::sqrt(varianceDistance / (vertexCount - 1)) : 0.0f;

    // Small floor so that a nearly-perfect fit doesn't reject its own noise floor
    const float kMinStdDev = 1e-4f;
    float effectiveStdDev = std::max(stdDevDistance, kMinStdDev);
    float rejectDistance = (float)meanDistance + config.outlierDeviationsThreshold * effectiveStdDev;

    for (size_t i = 0; i < vertexCount; i++) {
        float weight = (distances(i) > rejectDistance) ? 0.0f : 1.0f;
        (*weights)(i) = weight;
    }
    
    return _ICPCorrespondence(sourceVertices, targetVertices, targetNormals, squaredErrors, weights);
}

static Eigen::Matrix4f _computePointToPlaneTransform(_ICPCorrespondence& correspondence, ICPConfiguration config)
{
    const std::vector<math::Vec3>& sourceVertices = correspondence.sourceVertices;
    std::vector<math::Vec3>& targetVertices = correspondence.targetVertices;
    std::vector<math::Vec3>& targetNormals = correspondence.targetNormals;
    const Eigen::VectorXf& weights = *(correspondence.weights);
    size_t sourceVerticesLength = sourceVertices.size();
    
    Eigen::Vector3f p; // The source point
    Eigen::Vector3f q; // The target point
    Eigen::Vector3f n; // The source normal
    Eigen::VectorXf cn(6); // A concatenation of c and n since it's so useful
    
    // Perform linearized point-to-plane ICP, as described in:
    //    https://www.cs.princeton.edu/~smr/papers/icpstability.pdf
    
    // A * x = b. We seek to solve for x.
    Eigen::MatrixXf A = Eigen::MatrixXf::Zero(6, 6);
    Eigen::VectorXf b = Eigen::VectorXf::Zero(6);
    Eigen::VectorXf x = Eigen::VectorXf::Zero(6);
    
    for (off_t i = 0; i < sourceVerticesLength; i++) {
        float w = weights(i);
        p = standard_cyborg::toVector3f(sourceVertices[i]);
        q = standard_cyborg::toVector3f(targetVertices[i]);
        n = standard_cyborg::toVector3f(targetNormals[i]);
        cn.head<3>() = p.cross(n) * w;
        cn.tail<3>() = n * w;
        
        // We used to multiply A & b by w below, but it's faster to do that multiplication above
        
        // Another optimization: adding .noalias()
        // A.noalias() += cn * cn.transpose();
        // http://eigen.tuxfamily.org/dox/TopicWritingEfficientProductExpression.html
        
        // A third optimization: manually add the result of only the upper triangular product of cn * transpose(cn)
        AddUpperTriangularOuterProductToMatrix6x6(cn, A);
        b.noalias() -= cn * (p - q).dot(n);
    }
    
    // Fill in the lower part of the triangular matrix, which is mathematically identical
    FillUpperTriangularMatrixIntoLower6x6(A);
    
    x = A.llt().solve(b);

    // Retract the linearized 6-vector back to SE(3) via Rodrigues (axis-angle exponential), not Euler XYZ.
    // The Jacobian above (cn.head<3> = p × n) corresponds to the skew-symmetric/axis-angle parameterization
    // p_new ≈ p + ω × p + t, so retracting with R = exp([ω]×) is the internally consistent choice.
    Eigen::Vector3f omega = x.head<3>();
    float theta = omega.norm();
    Eigen::Matrix3f R;
    if (theta < 1e-8f) {
        // Small-angle fallback: R ≈ I + [ω]× (first-order Rodrigues). Avoids the
        // division by theta without introducing a discontinuity.
        R = Eigen::Matrix3f::Identity();
        R(0,1) = -omega.z(); R(0,2) =  omega.y();
        R(1,0) =  omega.z(); R(1,2) = -omega.x();
        R(2,0) = -omega.y(); R(2,1) =  omega.x();
    } else {
        R = Eigen::AngleAxisf(theta, omega / theta).toRotationMatrix();
    }

    Eigen::Matrix4f sourceTransform = Eigen::Matrix4f::Identity();
    sourceTransform.topLeftCorner<3, 3>() = R;
    sourceTransform.topRightCorner<3, 1>() = x.tail<3>();

    return sourceTransform;
}

bool hasNaN(const math::Mat4x4& m) {
    return std::isnan(m.m00) || std::isnan(m.m01) || std::isnan(m.m02) || std::isnan(m.m03) ||
           std::isnan(m.m10) || std::isnan(m.m11) || std::isnan(m.m12) || std::isnan(m.m13) ||
           std::isnan(m.m20) || std::isnan(m.m21) || std::isnan(m.m22) || std::isnan(m.m23) ||
           std::isnan(m.m30) || std::isnan(m.m31) || std::isnan(m.m32) || std::isnan(m.m33);
}

// transform all the 3D vectors in  `target`, with the specified transformation matrix. And make sure to normalize homogenous coordinates, after transforming.
void transformVectors(const Eigen::Matrix4f& m, std::vector<math::Vec3>& target) {
    const float m00 = m(0, 0), m01 = m(0, 1), m02 = m(0, 2), m03 = m(0, 3),
                m10 = m(1, 0), m11 = m(1, 1), m12 = m(1, 2), m13 = m(1, 3),
                m20 = m(2, 0), m21 = m(2, 1), m22 = m(2, 2), m23 = m(2, 3);
    
    size_t count = target.size();
    for (size_t i = 0; i < count; ++i) {
        Vector3f a = standard_cyborg::toVector3f(target[i]);
        
        const float x = a.x(), y = a.y(), z = a.z();
        float w = m(3, 0) * x + m(3, 1) * y + m(3, 2) * z + m(3, 3);
        if (w == 0) { w = 1.0; }
        const float wInverse = 1.0 / w;
        
        target[i].x = (m00 * x + m01 * y + m02 * z + m03) * wInverse;
        target[i].y = (m10 * x + m11 * y + m12 * z + m13) * wInverse;
        target[i].z = (m20 * x + m21 * y + m22 * z + m23) * wInverse;
    }
}

ICPResult ICP::run(ICPConfiguration config,
                   sc3d::Geometry& sourceCloud,
                   sc3d::Geometry& targetCloud,
                   std::function<void(ICPResult)> callback)
{
    ICPResult result;
    result.succeeded = true;
    
    if (sourceCloud.getPositions().size() == 0 || targetCloud.getPositions().size() == 0) {
        result.rmsCorrespondenceError = 0;

        /*
        DEBUG_LOG("WARNING: sourceCloud or targetCloud empty. sourceCloud size %ld.\ntargetCloud size %ld\n",
                  sourceCloud.getVertices().size(),
                  targetCloud.getVertices().size());
        */
        
        return result;
    }
    
    // Although architecturally bad, we're going to abuse the knowledge that our caller
    // is operating on a sourceCloud that will only be used here and then discarded
    // A slower but cleaner approach is to copy the vertices to our own local version
    std::vector<math::Vec3>& sourceVertices = const_cast< std::vector<math::Vec3>&>(sourceCloud.getPositions());

//    const SCVec3KdTreeAdaptor& targetCloudKdTree = targetCloud.getKdTree();
    
    // We run multiple passes, so we aggregate the successive transforms here. *This is the main output*
    Eigen::Matrix4f sourceTransform = Eigen::Matrix4f::Identity();
    
    // Track relative and absolute error from step to step
    float relativeError = config.tolerance * 10;
    
    // Reuse these buffers between iterations
    size_t vertexCount = sourceVertices.size();
    
    std::vector<math::Vec3> targetVertices(vertexCount, math::Vec3());
    std::vector<math::Vec3> targetNormals(vertexCount, math::Vec3());
    std::unique_ptr<Eigen::VectorXf> squaredErrors(new Eigen::VectorXf(vertexCount));
    std::unique_ptr<Eigen::VectorXf> weights(new Eigen::VectorXf(vertexCount));
    
    static const float kTranslationLimit = 0.2;
    float squaredTranslationLimit = kTranslationLimit * kTranslationLimit;
    int iteration = 0;
    
    float previousError = 1e10;
    
    while (iteration++ < config.maxIterations && relativeError > config.tolerance) {
        // Compute the correspondence between the points being source and the reference cloud
        _ICPCorrespondence correspondence = _computeCorrespondence(sourceVertices,
                                                                   targetCloud,
                                                                   config,
                                                                   targetVertices,
                                                                   targetNormals,
                                                                   squaredErrors.get(),
                                                                   weights.get());
        
        // Compute the transform mapping these correspondences from the source to the target vertices
        Eigen::Matrix4f sourceTransformAdjustment = _computePointToPlaneTransform(correspondence, config);
        
        // This check doesn't enforce overall camera movement limits, but is instead an early bailout
        // for when ICP simply diverges to infinity
        if (sourceTransformAdjustment.col(3).head<3>().squaredNorm() > squaredTranslationLimit) {
            result.succeeded = false;
            break;
        }

        transformVectors(sourceTransformAdjustment, sourceVertices);
        
        // Calculate the correspondence error
        float rmsError = correspondence.computeRMSCorrespondenceError();
        
        relativeError = fabsf(rmsError - previousError) / rmsError;
        previousError = rmsError;
        sourceTransform = sourceTransformAdjustment * sourceTransform;

        if (sourceTransform.col(3).head<3>().squaredNorm() > squaredTranslationLimit) {
            result.succeeded = false;
            break;
        }
        
        result.sourceTransform = standard_cyborg::toMat4x4(sourceTransform);
        result.rmsCorrespondenceError = rmsError;
        result.iterationCount = iteration;
        
#if DEBUG && TARGET_OS_MAC
        result.sourceVertices = std::make_shared<std::vector<math::Vec3>>(correspondence.sourceVertices);
        result.targetVertices = std::make_shared<std::vector<math::Vec3>>(correspondence.targetVertices);
#endif
        
        if (callback != nullptr) { callback(result); }
    }
    
    if (isnan(result.rmsCorrespondenceError) || isinf(result.rmsCorrespondenceError) || hasNaN(result.sourceTransform)) {
        result.succeeded = false;
    }
    
    return result;
}
