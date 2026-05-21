//
//  ARFaceCameraManager.swift
//  StandardCyborgUI
//
//  Drop-in replacement for CameraManager that drives capture from an ARSession
//  running ARFaceTrackingConfiguration. Per frame it supplies:
//    - the AR-captured RGB image (converted YUV -> BGRA),
//    - the depth pixel buffer from the front TrueDepth camera,
//    - the per-frame AVCameraCalibrationData,
//    - and a frame-to-frame "camera-in-face" pose delta derived from the
//      ARFaceAnchor + ARCamera transforms, which the C++ fusion layer trusts
//      as a high-confidence ICP bypass when face tracking is healthy.
//
//  The face-frame delta is the key idea: by feeding deltas measured in the
//  face anchor's local coordinates, the accumulated _extrinsicMatrix in
//  PBFModel naturally lives in face-frame (Phase C), so surfels stay
//  registered to the head even as the head rotates in world.
//

import ARKit
import AVFoundation
import CoreImage
import Foundation

@objc public protocol ARFaceCameraManagerDelegate: AnyObject {
    /// Mirrors CameraManagerDelegate.cameraDidOutput plus a head-pose delta.
    /// headPoseDelta is camera-in-face_N * inverse(camera-in-face_{N-1}); on the
    /// very first valid frame the delta is identity.
    /// confidence is in [0, 1]; >= 0.9 triggers the ICP-bypass path inside
    /// SCReconstructionManager.
    func arFaceCameraDidOutput(colorBuffer: CVPixelBuffer,
                               depthBuffer: CVPixelBuffer,
                               depthCalibrationData: AVCameraCalibrationData,
                               headPoseDelta: simd_float4x4,
                               headPoseConfidence: Float)

    /// Optional UI-feedback channel. Fires every face-tracking frame with the
    /// face anchor's pose expressed in camera coordinates (inv(camera) * face).
    /// Powers the Face-ID-style scan overlay (Phase E) that fills cardinal arcs
    /// as the user rotates their head.
    @objc optional func arFaceCameraDidObserveFacePose(_ facePoseInCamera: simd_float4x4,
                                                       isTracked: Bool)

    @objc optional func arFaceCameraManagerDidStartSession(_ manager: ARFaceCameraManager)
    @objc optional func arFaceCameraManagerDidFailToStart(_ manager: ARFaceCameraManager, reason: String)
}

/// UserDefaults key gating the Kabsch refiner pass (Phase D). When true and the
/// AR face pipeline is engaged, each frame runs a RANSAC + Kabsch validation
/// against ARKit's face-mesh vertices and downgrades head-pose confidence when
/// the constellation disagrees with ARKit's anchor transform.
public let kScanUseKabschRefinerDefaultsKey = "scan.use_kabsch_refiner"

@objc public final class ARFaceCameraManager: NSObject {

    @objc public weak var delegate: ARFaceCameraManagerDelegate?

    @objc public class var isSupported: Bool {
        ARFaceTrackingConfiguration.isSupported
    }

    /// Public ARSession so callers (e.g. the scanning view controller) can
    /// embed an ARSCNView or read tracking state if they want to.
    public let session = ARSession()

    @objc public private(set) var isSessionRunning = false

    /// When true (driven by kScanUseKabschRefinerDefaultsKey), each frame
    /// validates the ARKit-derived head-pose delta against a Kabsch alignment
    /// over a face-mesh vertex constellation; low agreement -> confidence drops.
    private lazy var _useKabschRefiner: Bool = {
        UserDefaults.standard.bool(forKey: kScanUseKabschRefinerDefaultsKey)
    }()

    // MARK: - Lifecycle

    @objc public override init() {
        super.init()
        session.delegate = self
        session.delegateQueue = _delegateQueue
    }

    @objc public func startSession() {
        guard ARFaceCameraManager.isSupported else {
            delegate?.arFaceCameraManagerDidFailToStart?(self, reason: "Face tracking not supported on this device")
            return
        }
        let config = ARFaceTrackingConfiguration()
        config.maximumNumberOfTrackedFaces = 1
        config.isLightEstimationEnabled = true
        // Reset accumulated state so a new scan starts from identity.
        _previousCameraInFace = nil
        _previousFaceVerticesInWorld = nil
        session.run(config, options: [.resetTracking, .removeExistingAnchors])
        isSessionRunning = true
        delegate?.arFaceCameraManagerDidStartSession?(self)
    }

    @objc public func stopSession() {
        session.pause()
        isSessionRunning = false
    }

    // MARK: - Private state

    private let _delegateQueue = DispatchQueue(label: "ARFaceCameraManager.delegate",
                                               qos: .userInitiated)

    // Reused per-frame to convert YpCbCr -> BGRA, the format the fusion layer expects.
    private lazy var _ciContext: CIContext = {
        if let device = MTLCreateSystemDefaultDevice() {
            return CIContext(mtlDevice: device, options: nil)
        }
        return CIContext()
    }()

    // The BGRA conversion target; allocated on demand and reused while its size matches.
    private var _bgraConversionBuffer: CVPixelBuffer?
    private var _bgraConversionSize: CGSize = .zero

    // Most-recently-emitted camera-in-face transform, used to compute the next delta.
    private var _previousCameraInFace: simd_float4x4?

    // Subsampled face-mesh vertices of the previous frame, expressed in world
    // coordinates. Used by the Kabsch refiner to validate the head-pose delta.
    private var _previousFaceVerticesInWorld: [SIMD3<Float>]?
}

// MARK: - ARSessionDelegate

extension ARFaceCameraManager: ARSessionDelegate {

    public func session(_ session: ARSession, didUpdate frame: ARFrame) {
        // We need a tracked face anchor. capturedDepthData is required for the
        // scan path but the UI face-pose observer fires on every face-tracking
        // frame (60Hz) regardless of depth availability, so we report it first.
        let faceAnchor = frame.anchors.compactMap({ $0 as? ARFaceAnchor }).first
        if let faceAnchor = faceAnchor {
            let facePoseInCamera = simd_inverse(frame.camera.transform) * faceAnchor.transform
            DispatchQueue.main.async { [weak self] in
                guard let self = self else { return }
                self.delegate?.arFaceCameraDidObserveFacePose?(facePoseInCamera, isTracked: faceAnchor.isTracked)
            }
        }

        // We need synchronized depth + a tracked face anchor. ARKit drops
        // capturedDepthData on frames where the depth sensor didn't fire,
        // which is roughly half of them at 60Hz RGB vs. 15Hz depth.
        guard let depthData = frame.capturedDepthData else { return }
        guard let faceAnchor = faceAnchor else { return }

        let depthBuffer = depthData.depthDataMap
        guard let calibration = depthData.cameraCalibrationData else { return }

        // Convert YUV captured image to BGRA. The fusion layer's _fillDepthVector
        // reads 4 bytes/pixel from this buffer.
        guard let colorBuffer = _convertedBGRABuffer(from: frame.capturedImage) else { return }

        // Compute the camera-in-face transform for this frame, then the delta.
        let faceWorld = faceAnchor.transform
        let cameraWorld = frame.camera.transform
        let cameraInFace = simd_inverse(faceWorld) * cameraWorld

        let delta: simd_float4x4
        if let prev = _previousCameraInFace {
            delta = cameraInFace * simd_inverse(prev)
        } else {
            delta = matrix_identity_float4x4
        }
        _previousCameraInFace = cameraInFace

        // Base confidence: ARFaceAnchor.isTracked is binary, ARCamera.trackingState
        // is finer. Collapse to: 1.0 on healthy, 0.5 on limited, 0.0 on lost.
        var confidence: Float
        if !faceAnchor.isTracked {
            confidence = 0.0
        } else {
            switch frame.camera.trackingState {
            case .normal: confidence = 1.0
            case .limited: confidence = 0.5
            case .notAvailable: confidence = 0.0
            }
        }

        // Phase D: Kabsch refiner. Build a subsampled constellation of face-mesh
        // vertices in world coordinates and validate ARKit's reported delta
        // against a rigid alignment recovered from those vertices. Disagreement
        // (low inlier fraction) drops the confidence so the C++ side falls back
        // to plain ICP for this frame.
        let currentVerticesInWorld = _subsampledFaceVerticesInWorld(faceAnchor: faceAnchor)
        if _useKabschRefiner, confidence > 0, let prevVerts = _previousFaceVerticesInWorld, prevVerts.count == currentVerticesInWorld.count {
            let inlierFraction = KabschRefiner.inlierFraction(previousVerticesInWorld: prevVerts,
                                                              currentVerticesInWorld: currentVerticesInWorld)
            // Linearly scale confidence by inlier agreement. inlierFraction >= 0.9
            // leaves confidence ~unchanged; 0.5 halves it; below 0.3 effectively
            // disables the prior for this frame.
            confidence *= max(0, min(1, (inlierFraction - 0.3) / 0.6))
        }
        _previousFaceVerticesInWorld = currentVerticesInWorld

        delegate?.arFaceCameraDidOutput(colorBuffer: colorBuffer,
                                        depthBuffer: depthBuffer,
                                        depthCalibrationData: calibration,
                                        headPoseDelta: delta,
                                        headPoseConfidence: confidence)
    }

    /// Subsamples the ARKit face-mesh vertices into a small constellation
    /// expressed in world coordinates. Used by the Kabsch refiner to validate
    /// the head-pose delta. Stride matches KabschRefiner.defaultSubsampleStride.
    private func _subsampledFaceVerticesInWorld(faceAnchor: ARFaceAnchor) -> [SIMD3<Float>] {
        let vertices = faceAnchor.geometry.vertices
        let stride = KabschRefiner.defaultSubsampleStride
        let faceWorld = faceAnchor.transform
        var out: [SIMD3<Float>] = []
        out.reserveCapacity(vertices.count / stride + 1)
        var i = 0
        while i < vertices.count {
            let v = vertices[i]
            let worldH = faceWorld * SIMD4<Float>(v.x, v.y, v.z, 1)
            out.append(SIMD3<Float>(worldH.x, worldH.y, worldH.z))
            i += stride
        }
        return out
    }

    public func session(_ session: ARSession, didFailWithError error: Error) {
        delegate?.arFaceCameraManagerDidFailToStart?(self, reason: error.localizedDescription)
    }

    // MARK: - YUV -> BGRA conversion

    private func _convertedBGRABuffer(from yuvBuffer: CVPixelBuffer) -> CVPixelBuffer? {
        let width = CVPixelBufferGetWidth(yuvBuffer)
        let height = CVPixelBufferGetHeight(yuvBuffer)
        let targetSize = CGSize(width: width, height: height)

        if _bgraConversionBuffer == nil || _bgraConversionSize != targetSize {
            let attrs: [String: Any] = [
                kCVPixelBufferIOSurfacePropertiesKey as String: [:],
                kCVPixelBufferMetalCompatibilityKey as String: true,
            ]
            var pb: CVPixelBuffer?
            let status = CVPixelBufferCreate(kCFAllocatorDefault,
                                             width,
                                             height,
                                             kCVPixelFormatType_32BGRA,
                                             attrs as CFDictionary,
                                             &pb)
            guard status == kCVReturnSuccess, let buffer = pb else { return nil }
            _bgraConversionBuffer = buffer
            _bgraConversionSize = targetSize
        }

        guard let dest = _bgraConversionBuffer else { return nil }
        let image = CIImage(cvPixelBuffer: yuvBuffer)
        _ciContext.render(image, to: dest)
        return dest
    }
}
