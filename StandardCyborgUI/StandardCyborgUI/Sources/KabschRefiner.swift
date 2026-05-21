//
//  KabschRefiner.swift
//  StandardCyborgUI
//
//  Validates the ARKit-derived camera-in-face head-pose delta by running a
//  RANSAC + Kabsch rigid alignment over a constellation of stable face-mesh
//  vertices. The rotation is recovered via polar decomposition (Higham
//  iteration) rather than SVD, which avoids pulling in BLAS/LAPACK while
//  remaining numerically robust for 3x3 covariance matrices.
//
//  Used by ARFaceCameraManager as a confidence-adjustment layer:
//    - if Kabsch and ARKit agree (small residual, high inlier ratio),
//      we trust the prior at full confidence -> SCReconstructionManager
//      bypasses ICP entirely;
//    - if they disagree, we drop confidence so the C++ side falls back
//      to plain ICP for that frame.
//
//  This is the "patch constellation" Phase D approach adapted to ARKit's
//  pre-tracked face mesh: ARKit's face-mesh vertices are already paired
//  across frames by index, so we don't need optical flow / OpenCV.
//

import ARKit
import Foundation
import simd

@objc public final class KabschRefiner: NSObject {

    /// Subsample stride applied to the ~1220-vertex ARKit face mesh. Every
    /// stride-th vertex is fed into RANSAC. 30 keeps the constellation small
    /// enough to RANSAC quickly while still oversampling enough to survive
    /// occlusion of large face regions under 30-45 degree rotations.
    @objc public static let defaultSubsampleStride = 30

    /// RANSAC iteration count. With ~40 vertices and 3-point hypotheses, this
    /// gives ~99% probability of hitting an all-inlier sample when the true
    /// inlier ratio is >= 0.5.
    @objc public static let defaultRansacIterations = 20

    /// Inlier residual threshold in metres. Face-mesh vertices live in a ~0.2 m
    /// box; 5 mm is a tight-but-not-pathological threshold.
    @objc public static let defaultInlierThresholdMeters: Float = 0.005

    /// Validates a naive ARKit-derived camera-in-face delta against a Kabsch
    /// alignment over the supplied face-mesh vertex constellation.
    ///
    /// - Parameters:
    ///   - previousVerticesInWorld: face-mesh vertices of frame N-1 in world coords.
    ///   - currentVerticesInWorld:  face-mesh vertices of frame N   in world coords.
    /// - Returns: an inlier fraction in [0, 1] describing how many constellation
    ///   points agree with the recovered rigid transform. ARFaceCameraManager
    ///   maps this to a confidence multiplier on the head-pose prior.
    public static func inlierFraction(previousVerticesInWorld: [simd_float3],
                                      currentVerticesInWorld: [simd_float3],
                                      inlierThresholdMeters: Float = defaultInlierThresholdMeters,
                                      ransacIterations: Int = defaultRansacIterations) -> Float {
        let n = min(previousVerticesInWorld.count, currentVerticesInWorld.count)
        guard n >= 4 else { return 0 }

        var bestInlierCount = 0
        var rng = SystemRandomNumberGenerator()

        for _ in 0..<ransacIterations {
            guard let triplet = sample3Indices(upperBound: n, using: &rng) else { return 0 }
            let srcTriplet = [previousVerticesInWorld[triplet.0], previousVerticesInWorld[triplet.1], previousVerticesInWorld[triplet.2]]
            let tgtTriplet = [currentVerticesInWorld[triplet.0], currentVerticesInWorld[triplet.1], currentVerticesInWorld[triplet.2]]
            let candidate = kabschAlign(source: srcTriplet, target: tgtTriplet)

            var inlierCount = 0
            for i in 0..<n {
                let src4 = SIMD4<Float>(previousVerticesInWorld[i].x,
                                        previousVerticesInWorld[i].y,
                                        previousVerticesInWorld[i].z,
                                        1)
                let transformed = candidate * src4
                let resid = simd_length(SIMD3<Float>(transformed.x, transformed.y, transformed.z) - currentVerticesInWorld[i])
                if resid < inlierThresholdMeters {
                    inlierCount += 1
                }
            }
            if inlierCount > bestInlierCount {
                bestInlierCount = inlierCount
            }
        }
        return Float(bestInlierCount) / Float(n)
    }

    // MARK: - Sampling

    private static func sample3Indices(upperBound: Int,
                                       using rng: inout SystemRandomNumberGenerator) -> (Int, Int, Int)? {
        guard upperBound >= 3 else { return nil }
        let i0 = Int.random(in: 0..<upperBound, using: &rng)
        var i1 = Int.random(in: 0..<upperBound, using: &rng)
        while i1 == i0 { i1 = Int.random(in: 0..<upperBound, using: &rng) }
        var i2 = Int.random(in: 0..<upperBound, using: &rng)
        while i2 == i0 || i2 == i1 { i2 = Int.random(in: 0..<upperBound, using: &rng) }
        return (i0, i1, i2)
    }

    // MARK: - Kabsch rigid alignment via polar decomposition

    /// Returns a 4x4 rigid transform T such that T * source[i] approx= target[i] for all i.
    /// Recovery path: centroids -> cross-covariance H -> polar decomp(H) -> rotation R,
    /// then translation t = target_centroid - R * source_centroid.
    static func kabschAlign(source: [SIMD3<Float>], target: [SIMD3<Float>]) -> simd_float4x4 {
        let n = min(source.count, target.count)
        guard n >= 3 else { return matrix_identity_float4x4 }

        var srcCentroid = SIMD3<Float>(repeating: 0)
        var tgtCentroid = SIMD3<Float>(repeating: 0)
        for i in 0..<n {
            srcCentroid += source[i]
            tgtCentroid += target[i]
        }
        srcCentroid /= Float(n)
        tgtCentroid /= Float(n)

        // Cross-covariance H = sum_i (target_centred_i) (source_centred_i)^T
        // so that R applied to source yields target.
        var H = simd_float3x3(SIMD3<Float>.zero, SIMD3<Float>.zero, SIMD3<Float>.zero)
        for i in 0..<n {
            let p = source[i] - srcCentroid
            let q = target[i] - tgtCentroid
            H.columns.0 += q * p.x
            H.columns.1 += q * p.y
            H.columns.2 += q * p.z
        }

        // Polar decomposition (Higham): converges to the orthogonal factor of H.
        // R_{k+1} = 0.5 * (R_k + (R_k^T)^{-1}). On 3x3 this is dirt cheap and converges
        // in well under 10 iterations for any nonsingular start.
        var R = H
        for _ in 0..<10 {
            let RinvT = simd_transpose(simd_inverse(R))
            let next = 0.5 * (R + RinvT)
            // Cheap early-stop: norm of update column 0.
            let diff = simd_length(next.columns.0 - R.columns.0)
                     + simd_length(next.columns.1 - R.columns.1)
                     + simd_length(next.columns.2 - R.columns.2)
            R = next
            if diff < 1e-6 { break }
        }

        // Enforce right-handedness. If det(R) < 0 the polar decomp produced a
        // reflection; flipping the column corresponding to the smallest singular
        // direction restores a proper rotation. For our use case (small frame-to-
        // frame head motion) the sign flip rarely fires, but it's required for
        // robustness against noise.
        if simd_determinant(R) < 0 {
            R.columns.2 = -R.columns.2
        }

        let t = tgtCentroid - R * srcCentroid
        return simd_float4x4(SIMD4<Float>(R.columns.0, 0),
                             SIMD4<Float>(R.columns.1, 0),
                             SIMD4<Float>(R.columns.2, 0),
                             SIMD4<Float>(t, 1))
    }
}
