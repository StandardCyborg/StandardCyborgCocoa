//
//  CameraManager.swift
//  Capture
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import ARKit
import AVFoundation
import Foundation
import UIKit

@objc public protocol CameraManagerDelegate: AnyObject {
    func cameraDidOutput(colorBuffer: CVPixelBuffer, depthBuffer: CVPixelBuffer, depthCalibrationData: AVCameraCalibrationData)
    @objc optional func cameraManagerDidStartSession(_ manager: CameraManager)
    @objc optional func cameraManagerDidNotStartSession(_ manager: CameraManager, result: CameraManager.SessionSetupResult)
}

public extension Notification.Name {
    /** Analogous to CameraManagerDelegate.cameraManagerDidStartSession */
    static let CameraManagerDidStartSession = NSNotification.Name("CameraManagerDidStartSession")
}

/** Interfaces with AVCaptureSession APIs to initiate and stream synchronized RGB + depth data.
    Also requests camera access and manages camera state in response to appplication state
    transitions and interruptions.
 */
@objc public class CameraManager: NSObject, AVCaptureDataOutputSynchronizerDelegate {
    
    @objc public enum SessionSetupResult: Int {
        case success
        case notAuthorized
        case configurationFailed
    }
    
    /** Returns true if a TrueDepth camera is available on the current device.
        Note that the rear-facing disparity camera in dual-lens iPhones is not
        considered TrueDepth, and it is not accurate enough to perform high
        quality 3D reconstruction.
     */
    @objc public class var isDepthCameraAvailable: Bool {
        return ARFaceTrackingConfiguration.isSupported
    }
    
    @objc public class var isCameraPermissionDenied: Bool {
        AVCaptureDevice.authorizationStatus(for: .video) == .denied
    }
    
    deinit {
        if _hasAddedObservers {
            _captureSession.removeObserver(self, forKeyPath: "running", context: &_sessionRunningContext)
        }
    }
    
    /** The mechanism for clients of CameraManager to be provided with streaming camera and depth frames */
    @objc public weak var delegate: CameraManagerDelegate!
    
    /** Configures the minimum color camera shutter speed. Defaults to 1/60s */
    @objc public var minColorExposureDuration: TimeInterval = 1.0/60.0
    
    /** Configures the default color camera resolution. Defaults to 1280x720. Values larger than 1920x1280 are not recommended. */
    @objc public var colorCaptureSessionPreset: AVCaptureSession.Preset = .hd1280x720
    
    /** Call this before calling startSession to request camera access and
        configure the AVCaptureSession for the desired resolution and framerate.
        Only necessary to call this once per instance of CameraManager.
        This does not start streaming, but prepares for it.
     */
    @objc public func configureCaptureSession(maxResolution: Int = 320, maxFramerate: Int = 30) {
        guard CameraManager.isDepthCameraAvailable else { return }
        
        // Check video authorization status, video access is required
        switch AVCaptureDevice.authorizationStatus(for: .video) {
        case .authorized:
            break
        case .notDetermined:
            _sessionQueue.suspend()
            AVCaptureDevice.requestAccess(for: .video, completionHandler: { granted in
                if !granted { self._sessionQueue_setupResult = .notAuthorized }
                self._sessionQueue.resume()
            })
        default:
            _sessionQueue_setupResult = .notAuthorized
        }
        
        _sessionQueue.async {
            if self._sessionQueue_setupResult == .success {
                self._sessionQueue_setupResult = self._sessionQueue_configureSession(maxResolution: maxResolution,
                                                                                     maxFramerate: maxFramerate)
            }
        }
    }
    
    /** After having called configureCaptureSession once, call this to start streaming
        color and depth data to the delegate
     */
    @objc public func startSession(_ completion: ((SessionSetupResult) -> Void)? = nil) {
        guard CameraManager.isDepthCameraAvailable else {
            DispatchQueue.main.async {
                completion?(.configurationFailed)
            }
            return
        }
        
        _sessionQueue.async {
            let result = self._sessionQueue_setupResult
            switch result {
            // Only set up observers and start the session running if setup succeeded
            case .success:
                if self._sessionQueue_isSessionRunning == false {
                    self._addObservers()
                    self._dataOutputQueue.async {
                        self._dataOutputQueue_renderingEnabled = true
                    }
                    
                    self._captureSession.startRunning()
                    self._sessionQueue_isSessionRunning = self._captureSession.isRunning
                    self.delegate?.cameraManagerDidStartSession?(self)
                    NotificationCenter.default.post(name: .CameraManagerDidStartSession, object: nil)
                }
            case .notAuthorized, .configurationFailed:
                self.delegate?.cameraManagerDidNotStartSession?(self, result: result)
                break
            }
            
            DispatchQueue.main.async {
                completion?(result)
            }
        }
    }
    
    /** Stops streaming to the delegate */
    @objc public func stopSession( _ completion: (() -> Void)? = nil) {
        _dataOutputQueue.async {
            self._dataOutputQueue_renderingEnabled = false
        }
        
        _sessionQueue.async {
            if self._sessionQueue_isSessionRunning {
                self._captureSession.stopRunning()
                self._sessionQueue_isSessionRunning = self._captureSession.isRunning
            }
            
            DispatchQueue.main.async { completion?() }
        }
    }
    
    /** Returns true if currently streaming depth data */
    @objc public var isSessionRunning: Bool {
        get {
            var running = false
            _sessionQueue.sync { running = self._sessionQueue_isSessionRunning }
            return running
        }
    }
    
    @objc public var isPaused: Bool = false {
        didSet {
            _sessionQueue.sync {
                if let connection = _depthDataOutput.connection(with: .depthData) {
                    connection.isEnabled = !isPaused
                }
            }
        }
    }
    
    /** Fixes the RGB camera's focus at the current focal distance when true */
    @objc public var isFocusLocked: Bool = false {
        didSet {            
            _focus(mode: isFocusLocked ? .locked : .continuousAutoFocus,
                   exposureMode: isFocusLocked ? .locked : .continuousAutoExposure,
                   at: CGPoint(x: 0.5, y: 0.5))
        }
    }
    
    /** Focuses the RGB camera at the specified point in screen space */
    @objc public func focusOnTap(at location: CGPoint) {
        let locationRect = CGRect(origin: location, size: .zero)
        let deviceRect = _videoDataOutput.metadataOutputRectConverted(fromOutputRect: locationRect)
        
        _focus(mode: .autoFocus, exposureMode: .autoExpose, at: deviceRect.origin)
    }
    
    // MARK: - Private properties
    
    private var _hasAddedObservers = false
    private var _sessionQueue_setupResult: SessionSetupResult = .success
    let _captureSession = AVCaptureSession()
    private var _sessionQueue_isSessionRunning = false
    private let _sessionQueue = DispatchQueue(label: "Session", qos: DispatchQoS.userInitiated, attributes: [], autoreleaseFrequency: .workItem)
    private let _videoDeviceDiscoverySession = AVCaptureDevice.DiscoverySession(deviceTypes: [.builtInTrueDepthCamera],
                                                                                mediaType: .video,
                                                                                position: .front)
    private var _videoDeviceInput: AVCaptureDeviceInput?
    private let _dataOutputQueue = DispatchQueue(label: "Video data output", qos: .userInitiated, attributes: [], autoreleaseFrequency: .workItem)
    private let _videoDataOutput = AVCaptureVideoDataOutput()
    private let _depthDataOutput = AVCaptureDepthDataOutput()
    private var _outputSynchronizer: AVCaptureDataOutputSynchronizer?
    private var _dataOutputQueue_renderingEnabled = true
    
    // MARK: - KVO and Notifications
    
    private var _sessionRunningContext = 0
    
    private func _addObservers() {
        guard let videoDevice = _videoDeviceInput?.device else {
            print("Video device input was nil")
            return
        }
        
        NotificationCenter.default.addObserver(self, selector: #selector(sessionRuntimeError),
                                               name: Notification.Name.AVCaptureSessionRuntimeError, object: _captureSession)
        
        _captureSession.addObserver(self, forKeyPath: "running", options: NSKeyValueObservingOptions.new, context: &_sessionRunningContext)
        
        /*
         A session can only run when the app is full screen. It will be interrupted
         in a multi-app layout, introduced in iOS 9, see also the documentation of
         AVCaptureSessionInterruptionReason. Add observers to handle these session
         interruptions and show a preview is paused message. See the documentation
         of AVCaptureSessionWasInterruptedNotification for other interruption reasons.
         */
        NotificationCenter.default.addObserver(self, selector: #selector(didEnterBackground),
                                               name: UIApplication.didEnterBackgroundNotification, object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(willEnterForground),
                                               name: UIApplication.willEnterForegroundNotification, object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(sessionWasInterrupted),
                                               name: .AVCaptureSessionWasInterrupted,
                                               object: _captureSession)
        NotificationCenter.default.addObserver(self, selector: #selector(sessionInterruptionEnded),
                                               name: .AVCaptureSessionInterruptionEnded,
                                               object: _captureSession)
        NotificationCenter.default.addObserver(self, selector: #selector(subjectAreaDidChange),
                                               name: .AVCaptureDeviceSubjectAreaDidChange,
                                               object: videoDevice)
        
        _hasAddedObservers = true
    }
    
    override public func observeValue(forKeyPath keyPath: String?, of object: Any?, change: [NSKeyValueChangeKey: Any]?, context: UnsafeMutableRawPointer?) {
        if context != &_sessionRunningContext {
            super.observeValue(forKeyPath: keyPath, of: object, change: change, context: context)
        }
    }
    
    @objc private func sessionWasInterrupted(notification: Notification) {
        if let userInfoValue = notification.userInfo?[AVCaptureSessionInterruptionReasonKey] as AnyObject?,
            let reasonIntegerValue = userInfoValue.integerValue,
            let reason = AVCaptureSession.InterruptionReason(rawValue: reasonIntegerValue) {
            print("Capture session was interrupted with reason \(reason)")
        }
    }
    
    @objc private func sessionInterruptionEnded(notification: Notification) {
    }
    
    @objc private func sessionRuntimeError(notification: Notification) {
        guard let errorValue = notification.userInfo?[AVCaptureSessionErrorKey] as? NSError else {
            return
        }
        
        let error = AVError(_nsError: errorValue)
        print("Capture session runtime error: \(error)")
        
        // Automatically try to restart the session running if media services were reset and the last start running succeeded.
        // Otherwise, enable the user to try to resume the session running.
        if error.code == .mediaServicesWereReset {
            _sessionQueue.async {
                if self._sessionQueue_isSessionRunning {
                    self._captureSession.startRunning()
                    self._sessionQueue_isSessionRunning = self._captureSession.isRunning
                }
            }
        }
    }
    
    @objc private func subjectAreaDidChange(notification: Notification) {
        guard !isFocusLocked else { return }
        
        let devicePoint = CGPoint(x: 0.5, y: 0.5)
        _focus(mode: .continuousAutoFocus, exposureMode: .continuousAutoExposure, at: devicePoint)
    }
    
    @objc private func didEnterBackground(notification: Notification) {
        // Free up resources
        _dataOutputQueue.async {
            self._dataOutputQueue_renderingEnabled = false
        }
    }
    
    @objc private func willEnterForground(notification: Notification) {
        _dataOutputQueue.async {
            self._dataOutputQueue_renderingEnabled = true
        }
    }
    
    // MARK: - AVCaptureDataOutputSynchronizerDelegate
    
    public func dataOutputSynchronizer(_ synchronizer: AVCaptureDataOutputSynchronizer,
                                       didOutput synchronizedDataCollection: AVCaptureSynchronizedDataCollection)
    {
        guard _dataOutputQueue_renderingEnabled else { return }
        
        // Read all outputs
        guard
            let syncedDepthData: AVCaptureSynchronizedDepthData =
                    synchronizedDataCollection.synchronizedData(for: _depthDataOutput) as? AVCaptureSynchronizedDepthData,
            let syncedVideoData: AVCaptureSynchronizedSampleBufferData =
                    synchronizedDataCollection.synchronizedData(for: _videoDataOutput) as? AVCaptureSynchronizedSampleBufferData
        else { return /* Only work on synced pairs */ }
        
        guard !syncedDepthData.depthDataWasDropped &&
              !syncedVideoData.sampleBufferWasDropped
        else { return }
        
        let depthData = syncedDepthData.depthData
        let depthBuffer = depthData.depthDataMap
        let sampleBuffer = syncedVideoData.sampleBuffer
        guard let colorBuffer = CMSampleBufferGetImageBuffer(sampleBuffer),
              let depthCalibrationData = depthData.cameraCalibrationData
        else { return }
        
        delegate.cameraDidOutput(colorBuffer: colorBuffer, depthBuffer: depthBuffer, depthCalibrationData: depthCalibrationData)
    }
    
    // MARK: - Private
    
    private func _sessionQueue_configureSession(maxResolution: Int, maxFramerate: Int) -> SessionSetupResult {
        guard let videoDevice = _videoDeviceDiscoverySession.devices.first else {
            print("Could not find any video device")
            return .configurationFailed
        }
        
        if let input = _videoDeviceInput {
            _captureSession.removeInput(input)
        }
        do {
            _videoDeviceInput = try AVCaptureDeviceInput(device: videoDevice)
        } catch {
            print("Could not create video device input: \(error)")
            return .configurationFailed
        }
        guard _captureSession.canAddInput(_videoDeviceInput!) else {
            print("Could not add video device input to the session")
            return .configurationFailed
        }
        guard _captureSession.outputs.contains(_videoDataOutput)
           || _captureSession.canAddOutput(_videoDataOutput)
        else {
            print("Could not add video data output to the session")
            return .configurationFailed
        }
        guard _captureSession.outputs.contains(_depthDataOutput)
           || _captureSession.canAddOutput(_depthDataOutput)
        else {
            print("Could not add depth data output to the session")
            return .configurationFailed
        }
        
        _captureSession.beginConfiguration()
        _captureSession.sessionPreset = colorCaptureSessionPreset
        _captureSession.addInput(_videoDeviceInput!)
        if !_captureSession.outputs.contains(_videoDataOutput) {
            _captureSession.addOutput(_videoDataOutput)
        }
        if !_captureSession.outputs.contains(_depthDataOutput) {
            _captureSession.addOutput(_depthDataOutput)
        }
        _videoDataOutput.videoSettings = [kCVPixelBufferPixelFormatTypeKey as String: Int(kCVPixelFormatType_32BGRA)]
        _videoDataOutput.alwaysDiscardsLateVideoFrames = true
        _depthDataOutput.isFilteringEnabled = false
        _depthDataOutput.alwaysDiscardsLateDepthData = true
        
        if let connection = _depthDataOutput.connection(with: .depthData) {
            connection.isEnabled = true
        } else {
            print("No AVCaptureConnection")
        }
        
        // Search for the highest resolution with 32-bit depth values
        let depthFormats = videoDevice.activeFormat.supportedDepthDataFormats
        let selectedFormat = depthFormats.filter {
            CMFormatDescriptionGetMediaSubType($0.formatDescription) == kCVPixelFormatType_DepthFloat32
        }.filter {
             CMVideoFormatDescriptionGetDimensions($0.formatDescription).width <= maxResolution
        }.max { first, second in
                CMVideoFormatDescriptionGetDimensions(first.formatDescription).width
              < CMVideoFormatDescriptionGetDimensions(second.formatDescription).width
        }
        
        do {
            try videoDevice.lockForConfiguration()
            videoDevice.activeDepthDataFormat = selectedFormat
            videoDevice.activeDepthDataMinFrameDuration = CMTimeMake(value: 1, timescale: Int32(maxFramerate))
            videoDevice.activeMaxExposureDuration = CMTimeMake(value: 1, timescale: Int32(minColorExposureDuration))
            videoDevice.isSubjectAreaChangeMonitoringEnabled = false
            videoDevice.unlockForConfiguration()
        } catch {
            print("Could not lock device for configuration: \(error)")
            _captureSession.commitConfiguration()
            return .configurationFailed
        }
        
        // Use an AVCaptureDataOutputSynchronizer to synchronize the video data and depth data outputs.
        // The first output in the dataOutputs array, in this case the AVCaptureVideoDataOutput, is the "master" output.
        _outputSynchronizer = AVCaptureDataOutputSynchronizer(dataOutputs: [_videoDataOutput, _depthDataOutput])
        _outputSynchronizer!.setDelegate(self, queue: _dataOutputQueue)
        _captureSession.commitConfiguration()
        
        return .success
    }
    
    private func _focus(mode focusMode: AVCaptureDevice.FocusMode,
                        exposureMode: AVCaptureDevice.ExposureMode,
                        at devicePoint: CGPoint)
    {
        _sessionQueue.async {
            guard let videoDevice = self._videoDeviceInput?.device else { return }
            
            do {
                try videoDevice.lockForConfiguration()
                if videoDevice.isFocusPointOfInterestSupported && videoDevice.isFocusModeSupported(focusMode) {
                    videoDevice.focusPointOfInterest = devicePoint
                    videoDevice.focusMode = focusMode
                }
                
                if videoDevice.isExposurePointOfInterestSupported && videoDevice.isExposureModeSupported(exposureMode) {
                    videoDevice.exposurePointOfInterest = devicePoint
                    videoDevice.exposureMode = exposureMode
                }
                
                if videoDevice.exposureDuration.seconds > self.minColorExposureDuration {
                    // Decrease the exposure duration and increase the ISO to get us closer
                    let scale = (videoDevice.exposureDuration.seconds / self.minColorExposureDuration).rounded()
                    let newDuration = CMTimeMake(value: videoDevice.exposureDuration.value, timescale: Int32(scale) * videoDevice.exposureDuration.timescale)
                    let newISO = videoDevice.iso * Float(scale)
                    if newISO < videoDevice.activeFormat.maxISO {
                        videoDevice.setExposureModeCustom(duration: newDuration,
                                                          iso: newISO,
                                                          completionHandler: nil)
                    }
                }
                videoDevice.unlockForConfiguration()
            } catch {
                print("Could not lock device for configuration: \(error)")
            }
        }
    }
    
}
